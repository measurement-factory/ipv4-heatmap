/*
 * input is a list of IPv4 addrs (or thier integer representation)
 * and an intensity value 0--255.
 *
 * data is drawn using a hilbert curve, which preserves grouping
 * see http://xkcd.com/195/
 * see http://en.wikipedia.org/wiki/Hilbert_curve
 * see Hacker's Delight (Henry S. Warren, Jr. 2002), sec 14-2, fig 14-5
 *
 * output is a 4096x4096 PNG file 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <gd.h>

// ######################################################################
// T. Nathan Mundhenk
// mundhenk@usc.edu
// C/C++ Macro HSV to RGB
#define PIX_HSV_TO_RGB_COMMON(H,S,V,R,G,B)                          \
if( V == 0 )                                                        \
{ R = 0; G = 0; B = 0; }                                            \
else if( S == 0 )                                                   \
{                                                                   \
  R = V;                                                            \
  G = V;                                                            \
  B = V;                                                            \
}                                                                   \
else                                                                \
{                                                                   \
  const double hf = H / 60.0;                                       \
  const int    i  = (int) floor( hf );                              \
  const double f  = hf - i;                                         \
  const double pv  = V * ( 1 - S );                                 \
  const double qv  = V * ( 1 - S * f );                             \
  const double tv  = V * ( 1 - S * ( 1 - f ) );                     \
  switch( i )                                                       \
    {                                                               \
    case 0:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case 1:                                                         \
      R = qv;                                                       \
      G = V;                                                        \
      B = pv;                                                       \
      break;                                                        \
    case 2:                                                         \
      R = pv;                                                       \
      G = V;                                                        \
      B = tv;                                                       \
      break;                                                        \
    case 3:                                                         \
      R = pv;                                                       \
      G = qv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 4:                                                         \
      R = tv;                                                       \
      G = pv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 5:                                                         \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    case 6:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case -1:                                                        \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    default:                                                        \
      abort();                                                      \
      break;                                                        \
    }                                                               \
}                                                                   \
R *= 255.0F;                                                        \
G *= 255.0F;                                                        \
B *= 255.0F;


gdImagePtr image = NULL;
int colors[256];

void
initialize(void)
{
	int i;
	image = gdImageCreate(4096, 4096);
	/* first allocated color becomes background by default */
	gdImageColorAllocate(image, 0, 0, 0);
	for (i=0; i<256; i++) {
		double hue;
		double r,g,b;
		hue = 240.0 * i / 255;
		PIX_HSV_TO_RGB_COMMON(hue, 1.0, 1.0, r, g, b);
		colors[i] = gdImageColorAllocate(image, r, g, b);
	}
}

/* from Hacker's Delight, fig 14-5 */
void hil_xy_from_s(unsigned s, int n, unsigned *xp,
                                      unsigned *yp) {

   int i;
   unsigned state, x, y, row;

   state = 0;                            // Initialize.
   x = y = 0;

   for (i = 2*n - 2; i >= 0; i -= 2) {   // Do n times.
      row = 4*state | ((s >> i) & 3);      // Row in table.
      x = (x << 1) | ((0x936C >> row) & 1);
      y = (y << 1) | ((0x39C6 >> row) & 1);
      state = (0x3E6B94C1 >> 2*row) & 3; // New state.
   }
   *xp = x;                              // Pass back
   *yp = y;                              // results.
}


int
main(int argc, char *argv[])
{
	char buf[512];
	FILE *pngout;
	initialize();

	while (fgets(buf, 512, stdin)) {
		unsigned int i;
		unsigned int k;
		unsigned int x;
		unsigned int y;
		char *t = strtok(buf, " \t\r\n");
		if (NULL == t)
			continue;
		if (strspn(t, "0123456789") == strlen(t))
			i = atoi(t);
		else
			i = inet_addr(t) >> 8;
		t = strtok(NULL, " \t\r\n");
		if (NULL == t)
			continue;
		k = atoi(t);

		hil_xy_from_s(i, 12, &x, &y);
		gdImageSetPixel(image, x, y, colors[k]);
	}

	pngout = fopen("map.png", "wb");
	gdImagePng(image, pngout);
	fclose(pngout);
	gdImageDestroy(image);
	return 0;
}
