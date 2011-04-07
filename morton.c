/*
 * Z_order curve calculation 
 * Roy Arends
 * Nominet
 */

void
mor_xy_from_s(unsigned s, int order, unsigned *xp, unsigned *yp)
{
    int i;
    unsigned x, y;
    x = y = 0;
    for (i = 2 * order - 2; i >= 0; i -= 2) {
	x = (x << 1) | ((s >> i) & 1);
	y = (y << 1) | ((s >> (i + 1)) & 1);
    }
    *xp = x;
    *yp = y;
}
