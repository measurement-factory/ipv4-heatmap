
/*
 * Figure 14-5 from Hacker's Delight (by Henry S. Warren, Jr. published by
 * Addison Wesley, 2002)
 *
 * See also http://www.hackersdelight.org/permissions.htm
 */

void
hil_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp)
{

    int i;
    unsigned state, x, y, row;

    state = 0;			/* Initialize. */
    x = y = 0;

    for (i = 2 * n - 2; i >= 0; i -= 2) {	/* Do n times. */
	row = 4 * state | ((s >> i) & 3);	/* Row in table. */
	x = (x << 1) | ((0x936C >> row) & 1);
	y = (y << 1) | ((0x39C6 >> row) & 1);
	state = (0x3E6B94C1 >> 2 * row) & 3;	/* New state. */
    }
    *xp = x;			/* Pass back */
    *yp = y;			/* results. */
}
