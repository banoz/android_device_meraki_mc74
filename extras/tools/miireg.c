/* miireg: libc-free MII register read via SIOCGMIIREG. usage: miireg <if> <phy-hex> <reg-hex> */
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
	for (; *s; s++) v = v * 16 + (*s <= '9' ? *s - '0' : (*s | 32) - 'a' + 10);
	return v;
}
static void out(const char *s) { int n = 0; while (s[n]) n++; sc3(4, 1, (long)s, n); }
struct ifr { char name[16]; unsigned short phy_id, reg_num, val_in, val_out; char pad[8]; };
int main(int argc, char **argv)
{
	struct ifr r;
	char b[8] = "0x0000\n";
	const char *h = "0123456789abcdef";
	int i;
	long fd;
	if (argc < 4) { out("usage: miireg if phy reg\n"); return 1; }
	for (i = 0; i < 16; i++) r.name[i] = 0;
	for (i = 0; argv[1][i] && i < 15; i++) r.name[i] = argv[1][i];
	r.phy_id = hex(argv[2]); r.reg_num = hex(argv[3]); r.val_out = 0;
	fd = sc3(281 /* socket */, 2, 2, 0);
	if (fd < 0) { out("socket failed\n"); return 1; }
	if (sc3(54, fd, 0x8948 /* SIOCGMIIREG */, (long)&r) < 0) { out("ioctl failed\n"); return 1; }
	for (i = 0; i < 4; i++) b[2 + i] = h[(r.val_out >> (12 - 4 * i)) & 15];
	out(b);
	return 0;
}
void _start(void) { asm volatile("ldr r0, [sp]\n add r1, sp, #4\n bl main\n mov r7, #1\n svc 0"); }
