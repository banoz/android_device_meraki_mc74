/* i2cw: libc-free SMBus-style 16-bit register access (repeated start) for old kernels.
 * usage: i2cw <bus> <addr-hex> <cmd-hex> [word-hex]   (no word = read LSB-first word) */
#define SYS_write 4
#define SYS_open 5
#define SYS_ioctl 54
static long sc3(long n, long a, long b, long c)
{
	register long r7 asm("r7") = n, r0 asm("r0") = a, r1 asm("r1") = b, r2 asm("r2") = c;
	asm volatile("svc 0" : "+r"(r0) : "r"(r7), "r"(r1), "r"(r2) : "memory");
	return r0;
}
static unsigned long hex(const char *s)
{
	unsigned long v = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
	for (; *s; s++)
		v = v * 16 + (*s <= '9' ? *s - '0' : (*s | 32) - 'a' + 10);
	return v;
}
static void out(const char *s) { int n = 0; while (s[n]) n++; sc3(SYS_write, 1, (long)s, n); }
static void outhex4(unsigned v) { char b[8] = "0x0000\n"; const char *h = "0123456789abcdef"; int i; for (i = 0; i < 4; i++) b[5 - i] = h[(v >> (4 * i)) & 15]; out(b); }
struct i2c_msg { unsigned short addr, flags, len; unsigned char *buf; };
struct i2c_rdwr { struct i2c_msg *msgs; unsigned nmsgs; };
int main(int argc, char **argv)
{
	char dev[16] = "/dev/i2c-0";
	unsigned char w[3], r[2];
	struct i2c_msg m[2];
	struct i2c_rdwr d;
	long fd;
	if (argc < 4) { out("usage: i2cw bus addr cmd [word]\n"); return 1; }
	dev[9] = argv[1][0];
	fd = sc3(SYS_open, (long)dev, 2, 0);
	if (fd < 0) { out("open failed\n"); return 1; }
	m[0].addr = m[1].addr = hex(argv[2]);
	w[0] = hex(argv[3]);
	m[0].flags = 0; m[0].buf = w;
	if (argc > 4) {
		unsigned v = hex(argv[4]);
		w[1] = v & 0xff; w[2] = v >> 8; m[0].len = 3;
		d.msgs = m; d.nmsgs = 1;
	} else {
		m[0].len = 1;
		m[1].flags = 1 /* I2C_M_RD */; m[1].len = 2; m[1].buf = r;
		d.msgs = m; d.nmsgs = 2;
	}
	if (sc3(SYS_ioctl, fd, 0x0707 /* I2C_RDWR */, (long)&d) < 0) { out("transfer failed\n"); return 1; }
	if (argc <= 4) outhex4(r[0] | (r[1] << 8));
	return 0;
}
void _start(void)
{
	asm volatile("ldr r0, [sp]\n add r1, sp, #4\n bl main\n mov r7, #1\n svc 0");
}
