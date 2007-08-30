/*
 * input is a list of IPv4 addrs (or thier integer representation) and an
 * intensity value 0--255.
 * 
 * data is drawn using a hilbert curve, which preserves grouping see
 * http://xkcd.com/195/ see http://en.wikipedia.org/wiki/Hilbert_curve see
 * Hacker's Delight (Henry S. Warren, Jr. 2002), sec 14-2, fig 14-5
 * 
 * output is a 4096x4096 PNG file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <gd.h>

#include "hsv2rgb.h"


gdImagePtr image = NULL;
#define NUM_DATA_COLORS 256
int colors[NUM_DATA_COLORS];
int debug = 0;
const char *font_file_or_name = "Luxi Mono:style=Regular";

extern void annotate_file(const char *fn);
const char *annotations = NULL;

void
initialize(void)
{
    int i;
    image = gdImageCreateTrueColor(4096, 4096);
    /* first allocated color becomes background by default */
    gdImageColorAllocate(image, 0, 0, 0);
    for (i = 0; i < NUM_DATA_COLORS; i++) {
	double hue;
	double r, g, b;
	hue = 240.0 * i / 255;
	PIX_HSV_TO_RGB_COMMON(hue, 1.0, 1.0, r, g, b);
	colors[i] = gdImageColorAllocate(image, r, g, b);
	if (debug)
	    fprintf(stderr, "colors[%d]=%d\n", i, colors[i]);
    }
}

/* from Hacker's Delight, fig 14-5 */
void
hil_xy_from_s(unsigned s, int n, unsigned *xp,
    unsigned *yp)
{

    int i;
    unsigned state, x, y, row;

    state = 0;
    //Initialize.
	x = y = 0;

    for (i = 2 * n - 2; i >= 0; i -= 2) {
	//Do n times.
	    row = 4 * state | ((s >> i) & 3);
	//Row in table.
	    x = (x << 1) | ((0x936C >> row) & 1);
	y = (y << 1) | ((0x39C6 >> row) & 1);
	state = (0x3E6B94C1 >> 2 * row) & 3;
	//New state.
    }
    *xp = x;
    //Pass back
	* yp = y;
    //results.
}


void
paint(void)
{
    char buf[512];
    unsigned int line = 0;
    while (fgets(buf, 512, stdin)) {
	unsigned int i;
	unsigned int x;
	unsigned int y;
	int color = -1;

	//get the "address"
	    char *t = strtok(buf, " \t\r\n");
	if (NULL == t)
	    continue;
	if (strspn(t, "0123456789") == strlen(t))
	    i = atoi(t) >> 8;
	else if (1 == inet_pton(AF_INET, t, &i))
	    i = ntohl(i) >> 8;
	else
	    errx(1, "bad input on line %d: %s", line, t);
	hil_xy_from_s(i, 12, &x, &y);
	if (debug)
	    fprintf(stderr, "%s => (%d,%d)\n", t, x, y);

	t = strtok(NULL, " \t\r\n");
	if (NULL != t) {
	    unsigned int k = atoi(t);
	    color = colors[k];
	} else {
	    unsigned int k;
	    color = gdImageGetPixel(image, x, y);
	    if (debug)
		fprintf(stderr, "pixel (%d,%d) has color index %d\n", x, y, color);
#if PNG_256_COLORS
	    assert(color >= 0);
	    assert(color < NUM_DATA_COLORS);
	    color++;
#else
	    for (k = 0; k < NUM_DATA_COLORS; k++) {
		if (colors[k] == color) {
	            if (debug)
		        fprintf(stderr, "color %d has index %d\n", color, k);
		    break;
		}
	    }
	    if (k == NUM_DATA_COLORS)
		k = 0;
	    color = colors[k+1];
#endif
	}

	gdImageSetPixel(image, x, y, color);
	line++;
    }
}

void
save(void)
{
    FILE *pngout = fopen("map.png", "wb");
    gdImagePng(image, pngout);
    fclose(pngout);
    gdImageDestroy(image);
    image = NULL;
}

int
main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, "da:f:")) != -1) {
	switch (ch) {
	case 'd':
	    debug++;
	    break;
	case 'a':
	    annotations = strdup(optarg);
	    break;
	case 'f':
	    font_file_or_name = strdup(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: %s [-d]\n", argv[0]);
	    exit(1);
	    break;
	}
    }
    argc -= optind;
    argv += optind;

    initialize();
    paint();
    if (annotations)
	annotate_file(annotations);
    save();
    return 0;
}
