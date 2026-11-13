/*
 * Meraki MC74 lights HAL: the LCD backlight, driven through the MP3309
 * backlight driver (falls back to the VideoCore display backlight), and the
 * front RGBW LED, a PCA9632 at 0x62 on I2C bus 2 (no kernel driver; written
 * through /dev/i2c-2). Notification and attention lights share the LED,
 * attention wins.
 */
#define LOG_TAG "lights.capri"
#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hardware/lights.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define BL_PATH "/sys/class/backlight/mp3309/brightness"
#define BL_PATH_VC "/sys/class/backlight/vc-display-bl/brightness"

#define LED_I2C_DEV "/dev/i2c-2"
#define LED_I2C_ADDR 0x62
/* PCA9632 registers */
#define PCA_MODE1   0x00
#define PCA_MODE2   0x01
#define PCA_PWM0    0x02    /* PWM0..3 = white, green, red, blue (checked on the MC74) */
#define PCA_GRPPWM  0x06
#define PCA_GRPFREQ 0x07
#define PCA_LEDOUT  0x08
#define PCA_AUTOINC 0x80

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_notification, g_attention;

static int write_int(const char *path, int value)
{
    char buf[16];
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        ALOGE("open %s: %s", path, strerror(errno));
        return -errno;
    }
    int len = snprintf(buf, sizeof(buf), "%d\n", value);
    int r = write(fd, buf, len);
    close(fd);
    return r == len ? 0 : -errno;
}

static int rgb_to_brightness(const struct light_state_t *state)
{
    int color = state->color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static int set_light_backlight(struct light_device_t *dev,
                               const struct light_state_t *state)
{
    (void)dev;
    int b = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    int err = write_int(BL_PATH, b);
    if (err == -ENOENT)
        err = write_int(BL_PATH_VC, b);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int led_write(int fd, const unsigned char *buf, int len)
{
    return write(fd, buf, len) == len ? 0 : -errno;
}

/* called with g_lock held */
static int led_apply(void)
{
    const struct light_state_t *st =
        (g_attention.color & 0xffffff) ? &g_attention : &g_notification;
    unsigned int color = st->color;
    int r = (color >> 16) & 0xff, g = (color >> 8) & 0xff, b = color & 0xff;
    int on = st->flashMode != LIGHT_FLASH_NONE ? st->flashOnMS : 0;
    int off = st->flashMode != LIGHT_FLASH_NONE ? st->flashOffMS : 0;
    int fd = open(LED_I2C_DEV, O_RDWR);
    int err;

    if (fd < 0) {
        ALOGE("open %s: %s", LED_I2C_DEV, strerror(errno));
        return -errno;
    }
    if (ioctl(fd, I2C_SLAVE_FORCE, LED_I2C_ADDR) < 0) {
        err = -errno;
        close(fd);
        return err;
    }
    if (!(r | g | b)) {
        unsigned char ledoff[] = { PCA_LEDOUT, 0x00 };
        unsigned char sleep[] = { PCA_MODE1, 0x11 };
        err = led_write(fd, ledoff, 2);
        if (!err)
            err = led_write(fd, sleep, 2);
    } else {
        unsigned char wake[] = { PCA_MODE1, 0x01 };
        /* blink with the group PWM: period (GFRQ+1)/24 s, duty GRPPWM/256 */
        int period = on + off, blink = on > 0 && off > 0;
        int gfrq = blink ? period * 24 / 1000 - 1 : 0;
        int gpwm = blink ? on * 256 / period : 0xff;
        unsigned char regs[] = {
            PCA_MODE2 | PCA_AUTOINC,
            blink ? 0x21 : 0x01,                 /* MODE2: DMBLNK for blinking */
            /* PWM0..3: grey/white colours use the white LED alone */
            (r == g && g == b) ? r : 0,
            (r == g && g == b) ? 0 : g,
            (r == g && g == b) ? 0 : r,
            (r == g && g == b) ? 0 : b,
            gpwm > 255 ? 255 : gpwm,             /* GRPPWM */
            gfrq < 0 ? 0 : gfrq > 255 ? 255 : gfrq, /* GRPFREQ */
            0xff,                                /* LEDOUT: PWM + group */
        };
        err = led_write(fd, wake, 2);
        usleep(1000);   /* oscillator start-up */
        if (!err)
            err = led_write(fd, regs, sizeof(regs));
    }
    close(fd);
    if (err)
        ALOGE("pca9632 write failed: %d", err);
    return err;
}

static int set_light_notifications(struct light_device_t *dev,
                                   const struct light_state_t *state)
{
    (void)dev;
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    int err = led_apply();
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_attention(struct light_device_t *dev,
                               const struct light_state_t *state)
{
    (void)dev;
    pthread_mutex_lock(&g_lock);
    g_attention = *state;
    int err = led_apply();
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int close_lights(struct light_device_t *dev)
{
    free(dev);
    return 0;
}

static int open_lights(const struct hw_module_t *module, const char *name,
                       struct hw_device_t **device)
{
    int (*set)(struct light_device_t *, const struct light_state_t *);

    if (!strcmp(LIGHT_ID_BACKLIGHT, name))
        set = set_light_backlight;
    else if (!strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set = set_light_notifications;
    else if (!strcmp(LIGHT_ID_ATTENTION, name))
        set = set_light_attention;
    else
        return -EINVAL;

    struct light_device_t *dev = calloc(1, sizeof(*dev));
    if (!dev)
        return -ENOMEM;
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->common.close = (int (*)(struct hw_device_t *))close_lights;
    dev->set_light = set;
    *device = (struct hw_device_t *)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open = open_lights,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "MC74 lights module",
    .author = "MC74 port",
    .methods = &lights_module_methods,
};
