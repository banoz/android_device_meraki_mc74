/* i2creg: libc-free I2C register peek/poke for old kernels.
 * usage: i2creg <bus> <addr-hex> <reg-hex> [value-hex]   (no value = read 1 byte) */
#define SYS_exit 1
#define SYS_read 3
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
static void outhex(unsigned v) { char b[6] = "0x00\n"; const char *h = "0123456789abcdef"; b[2] = h[(v >> 4) & 15]; b[3] = h[v & 15]; out(b); }
int main(int argc, char **argv)
{
	char dev[16] = "/dev/i2c-0";
	unsigned char buf[2];
	long fd;
	if (argc < 4) { out("usage: i2creg bus addr reg [val]\n"); return 1; }
	dev[9] = argv[1][0];
	fd = sc3(SYS_open, (long)dev, 2, 0);
	if (fd < 0) { out("open failed\n"); return 1; }
	if (sc3(SYS_ioctl, fd, 0x0706 /* I2C_SLAVE_FORCE */, hex(argv[2])) < 0) { out("addr failed\n"); return 1; }
	buf[0] = hex(argv[3]);
	if (argc > 4) {
		buf[1] = hex(argv[4]);
		if (sc3(SYS_write, fd, (long)buf, 2) != 2) { out("write failed\n"); return 1; }
		return 0;
	}
	if (sc3(SYS_write, fd, (long)buf, 1) != 1) { out("write reg failed\n"); return 1; }
	if (sc3(SYS_read, fd, (long)buf, 1) != 1) { out("read failed\n"); return 1; }
	outhex(buf[0]);
	return 0;
}
void _start(void)
{
	asm volatile("ldr r0, [sp]\n add r1, sp, #4\n bl main\n mov r7, #1\n svc 0");
}
