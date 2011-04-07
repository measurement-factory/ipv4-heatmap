#include <stdio.h>
#include <err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cidr.h"


extern void hil_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp);
extern void mor_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp);
void (*xy_from_s) (unsigned s, int n, unsigned *xp, unsigned *yp) = hil_xy_from_s;

extern int debug;

/*
 * The default the Hilbert curve order is 12.  This gives a 4096x4096
 * output image covering the entire space where each pixel represents
 * a /24.
 */
int hilbert_curve_order = -1;
int addr_space_bits_per_image = 32;	/* /0 */
int addr_space_bits_per_pixel = 8;	/* /24 */
unsigned int addr_space_first_addr = 0;
unsigned int addr_space_last_addr = ~0;


/*
 * Translate an IPv4 address (stored as a 32bit int) into
 * output X,Y coordinates.  First check if its within our
 * crop bounds.  Return 0 if out of bounds.
 */
unsigned int
xy_from_ip(unsigned ip, unsigned *xp, unsigned *yp)
{
    unsigned int s;
    if (ip < addr_space_first_addr)
	return 0;
    if (ip > addr_space_last_addr)
	return 0;
    s = (ip - addr_space_first_addr) >> addr_space_bits_per_pixel;
    xy_from_s(s, hilbert_curve_order, xp, yp);
    return 1;
}


void
set_morton_mode()
{
    xy_from_s = mor_xy_from_s;
}

int
set_order()
{
    hilbert_curve_order = (addr_space_bits_per_image - addr_space_bits_per_pixel) / 2;
    if (debug) {
	struct in_addr a;
	char buf[20];
	fprintf(stderr, "addr_space_bits_per_image = %d\n", addr_space_bits_per_image);
	fprintf(stderr, "addr_space_bits_per_pixel = %d\n", addr_space_bits_per_pixel);
	fprintf(stderr, "hilbert_curve_order = %d\n", hilbert_curve_order);
	a.s_addr = htonl(addr_space_first_addr);
	inet_ntop(AF_INET, &a, buf, 20);
	fprintf(stderr, "first_address = %s\n", buf);
	a.s_addr = htonl(addr_space_last_addr);
	inet_ntop(AF_INET, &a, buf, 20);
	fprintf(stderr, "last = %s\n", buf);
    }
    return hilbert_curve_order;
}

void
set_crop(const char *cidr)
{
    cidr_parse(cidr, &addr_space_first_addr, &addr_space_last_addr, &addr_space_bits_per_image);
    addr_space_bits_per_image = 32 - addr_space_bits_per_image;
    if (1 == (addr_space_bits_per_image % 2))
	errx(1, "Space to render must have even number of CIDR bits");
}

void
set_bits_per_pixel(int bpp)
{
    addr_space_bits_per_pixel = bpp;
    if (1 == (addr_space_bits_per_pixel % 2))
	errx(1, "CIDR bits per pixel must be even");
}
