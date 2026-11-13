/* hcitest: libc-free HCI_Reset over a UART at 115200 8N1 + RTS/CTS.
 * usage: hcitest /dev/ttySx   -> prints the reply bytes (expect 04 0e 04 01 03 0c 00) */
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
static void out(const char *s) { int n = 0; while (s[n]) n++; sc3(SYS_write, 1, (long)s, n); }
static void outhex(unsigned v) { char b[4] = "00 "; const char *h = "0123456789abcdef"; b[0] = h[(v >> 4) & 15]; b[1] = h[v & 15]; out(b); }
struct termios_arm { unsigned iflag, oflag, cflag, lflag; unsigned char line, cc[19]; };
int main(int argc, char **argv)
{
	struct termios_arm t;
	unsigned char cmd[4] = { 1, 3, 0x0c, 0 }, buf[64];
	long fd, n, i, total = 0;
	if (argc < 2) { out("usage: hcitest /dev/ttySx\n"); return 1; }
	fd = sc3(SYS_open, (long)argv[1], 2 | 0400 /* O_RDWR|O_NOCTTY */, 0);
	if (fd < 0) { out("open failed\n"); return 1; }
	if (sc3(SYS_ioctl, fd, 0x5401, (long)&t) < 0) { out("TCGETS failed\n"); return 1; }
	t.iflag = 0; t.oflag = 0; t.lflag = 0;
	t.cflag = 0010002 | 060 | 0200 | 04000 | 020000000000u; /* B115200 CS8 CREAD CLOCAL CRTSCTS */
	t.cc[6] = 0; t.cc[5] = 10; /* VMIN=0, VTIME=1 s */
	if (sc3(SYS_ioctl, fd, 0x5402, (long)&t) < 0) { out("TCSETS failed\n"); return 1; }
	sc3(SYS_ioctl, fd, 0x540B, 2); /* TCFLSH both */
	if (sc3(SYS_write, fd, (long)cmd, 4) != 4) { out("write failed\n"); return 1; }
	out("reply: ");
	while (total < 7 && (n = sc3(SYS_read, fd, (long)buf, sizeof(buf))) > 0) {
		for (i = 0; i < n; i++) outhex(buf[i]);
		total += n;
	}
	out(total ? "\n" : "(nothing)\n");
	return 0;
}
void _start(void)
{
	asm volatile("ldr r0, [sp]\n add r1, sp, #4\n bl main\n mov r7, #1\n svc 0");
}
