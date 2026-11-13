/*
 * Meraki MC74 sensors HAL: the CM3232 ambient light sensor at 0x10 on I2C
 * bus 2 (no kernel driver; read through /dev/i2c-2 from system_server).
 * Values are raw CM3232 counts (about 120 in an office, 0 covered, several
 * hundred under a flashlight), reported as lux; the auto-brightness curve in
 * the framework overlay is tuned to these counts.
 */
#define LOG_TAG "sensors.capri"
#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <hardware/sensors.h>

#define I2C_DEV     "/dev/i2c-2"
#define CM3232_ADDR 0x10
#define CM3232_ALS  0x50
#define HANDLE_LIGHT 1
#define POLL_NS     200000000LL

static const struct sensor_t sensor_list[] = {
    {
        .name = "CM3232 Light sensor",
        .vendor = "Capella",
        .version = 1,
        .handle = HANDLE_LIGHT,
        .type = SENSOR_TYPE_LIGHT,
        .maxRange = 65535.0f,
        .resolution = 1.0f,
        .power = 0.1f,
        .minDelay = 0,
        .stringType = SENSOR_STRING_TYPE_LIGHT,
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ON_CHANGE_MODE,
    },
};

struct ctx {
    struct sensors_poll_device_1 dev;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int enabled;
    int flush_pending;
    int last;
};

static int64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static int read_als(void)
{
    unsigned char cmd = CM3232_ALS, buf[2];
    struct i2c_msg msgs[2] = {
        { .addr = CM3232_ADDR, .flags = 0, .len = 1, .buf = &cmd },
        { .addr = CM3232_ADDR, .flags = I2C_M_RD, .len = 2, .buf = buf },
    };
    struct i2c_rdwr_ioctl_data data = { .msgs = msgs, .nmsgs = 2 };
    int fd = open(I2C_DEV, O_RDWR), ret;

    if (fd < 0)
        return -errno;
    ret = ioctl(fd, I2C_RDWR, &data) < 0 ? -errno : (buf[0] | (buf[1] << 8));
    close(fd);
    return ret;
}

static int activate(struct sensors_poll_device_t *d, int handle, int enabled)
{
    struct ctx *c = (struct ctx *)d;
    if (handle != HANDLE_LIGHT)
        return -EINVAL;
    pthread_mutex_lock(&c->lock);
    c->enabled = enabled;
    c->last = -1;   /* report the first reading right away */
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->lock);
    return 0;
}

static int set_delay(struct sensors_poll_device_t *d, int handle, int64_t ns)
{
    (void)d; (void)ns;
    return handle == HANDLE_LIGHT ? 0 : -EINVAL;
}

static int batch(struct sensors_poll_device_1 *d, int handle, int flags,
                 int64_t period_ns, int64_t timeout)
{
    (void)d; (void)flags; (void)period_ns; (void)timeout;
    return handle == HANDLE_LIGHT ? 0 : -EINVAL;
}

static int flush(struct sensors_poll_device_1 *d, int handle)
{
    struct ctx *c = (struct ctx *)d;
    if (handle != HANDLE_LIGHT)
        return -EINVAL;
    pthread_mutex_lock(&c->lock);
    c->flush_pending++;
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->lock);
    return 0;
}

static int poll_events(struct sensors_poll_device_t *d, sensors_event_t *data, int count)
{
    struct ctx *c = (struct ctx *)d;
    int n = 0;

    if (count <= 0)
        return 0;
    pthread_mutex_lock(&c->lock);
    for (;;) {
        if (c->flush_pending) {
            c->flush_pending--;
            memset(&data[n], 0, sizeof(data[n]));
            data[n].version = META_DATA_VERSION;
            data[n].type = SENSOR_TYPE_META_DATA;
            data[n].meta_data.what = META_DATA_FLUSH_COMPLETE;
            data[n].meta_data.sensor = HANDLE_LIGHT;
            n++;
            break;
        }
        if (c->enabled) {
            int v;
            pthread_mutex_unlock(&c->lock);
            v = read_als();
            pthread_mutex_lock(&c->lock);
            if (v >= 0 && c->enabled && v != c->last) {
                c->last = v;
                memset(&data[n], 0, sizeof(data[n]));
                data[n].version = sizeof(sensors_event_t);
                data[n].sensor = HANDLE_LIGHT;
                data[n].type = SENSOR_TYPE_LIGHT;
                data[n].timestamp = now_ns();
                data[n].light = (float)v;
                n++;
                break;
            }
            if (v < 0)
                ALOGE("CM3232 read failed: %d", v);
        }
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += POLL_NS;
            if (ts.tv_nsec >= 1000000000L) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000L;
            }
            if (c->enabled)
                pthread_cond_timedwait(&c->cond, &c->lock, &ts);
            else
                pthread_cond_wait(&c->cond, &c->lock);
        }
    }
    pthread_mutex_unlock(&c->lock);
    return n;
}

static int close_dev(struct hw_device_t *d)
{
    free(d);
    return 0;
}

static int open_sensors(const struct hw_module_t *module, const char *id,
                        struct hw_device_t **device)
{
    struct ctx *c;
    (void)id;
    c = calloc(1, sizeof(*c));
    if (!c)
        return -ENOMEM;
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->cond, NULL);
    c->last = -1;
    c->dev.common.tag = HARDWARE_DEVICE_TAG;
    c->dev.common.version = SENSORS_DEVICE_API_VERSION_1_3;
    c->dev.common.module = (struct hw_module_t *)module;
    c->dev.common.close = close_dev;
    c->dev.activate = activate;
    c->dev.setDelay = set_delay;
    c->dev.poll = poll_events;
    c->dev.batch = batch;
    c->dev.flush = flush;
    *device = &c->dev.common;
    return 0;
}

static int get_sensors_list(struct sensors_module_t *m, const struct sensor_t **list)
{
    (void)m;
    *list = sensor_list;
    return sizeof(sensor_list) / sizeof(sensor_list[0]);
}

static struct hw_module_methods_t sensors_methods = {
    .open = open_sensors,
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = SENSORS_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "MC74 sensors module",
        .author = "MC74 port",
        .methods = &sensors_methods,
    },
    .get_sensors_list = get_sensors_list,
};
