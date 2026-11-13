/* memrd: libc-free 32-bit physical memory reader via /dev/mem. usage: memrd <phys-hex> [count] */
static long sc6(long n, long a, long b, long c, long d, long e, long f)
{
	register long r7 asm("r7") = n, r0 asm("r0") = a, r1 asm("r1") = b, r2 asm("r2") = c,
		r3 asm("r3") = d, r4 asm("r4") = e, r5 asm("r5") = f;
	asm volatile("svc 0" : "+r"(r0) : "r"(r7), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) : "memory");
	return r0;
}
static unsigned long hex(const char *s)
{
	unsigned long v = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
	for (; *s; s++) v = v * 16 + (*s <= '9' ? *s - '0' : (*s | 32) - 'a' + 10);
	return v;
}
static void out(const char *s) { int n = 0; while (s[n]) n++; sc6(4, 1, (long)s, n, 0, 0, 0); }
static void hx(unsigned long v) { char b[11]; const char *h = "0123456789abcdef"; int i; for (i = 0; i < 8; i++) b[i] = h[(v >> (28 - 4 * i)) & 15]; b[8] = 0; out(b); }
int main(int argc, char **argv)
{
	unsigned long pa, base, n = 1, i;
	volatile unsigned long *p;
	long fd;
	if (argc < 2) { out("usage: memrd phys [count]\n"); return 1; }
	pa = hex(argv[1]); if (argc > 2) n = hex(argv[2]);
	fd = sc6(5, (long)"/dev/mem", 2 | 0x101000 /* O_RDWR|O_SYNC */, 0, 0, 0, 0);
	if (fd < 0) { out("open failed\n"); return 1; }
	base = pa & ~0xfffUL;
	p = (volatile unsigned long *)sc6(192 /* mmap2 */, 0, 0x2000, 3, 1, fd, base >> 12);
	if ((long)p < 0 && (long)p > -4096) { out("mmap failed\n"); return 1; }
	for (i = 0; i < n; i++) { hx(pa + 4 * i); out(": "); hx(p[((pa - base) >> 2) + i]); out("\n"); }
	return 0;
}
void _start(void) { asm volatile("ldr r0, [sp]\n add r1, sp, #4\n bl main\n mov r7, #1\n svc 0"); }
