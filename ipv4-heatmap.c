/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2.0
 * http://maps.measurement-factory.com/
 */

/*
 * ipv4-heatmap produces a "map" of IPv4 address space.
 * 
 * input is a list of IPv4 addrs and optional value for each.
 * 
 * Data is drawn using a hilbert curve, which preserves grouping see
 * http://xkcd.com/195/ and http://en.wikipedia.org/wiki/Hilbert_curve see
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
#include <gdfonts.h>

#include "hsv2rgb.h"

#define NUM_DATA_COLORS 256

extern void annotate_file(const char *fn);
extern void shade_file(const char *fn);
extern void legend(const char *, const char *orient);
extern void hil_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp);

/*
 * For now, the Hilbert curve order is hard-coded at 12.  To produce a
 * 4096x4096 image we need a 12th-order Hilbert curve.  That is, 2^12 = 4096.
 */
int hilbert_curve_order = 12;

gdImagePtr image = NULL;
int colors[NUM_DATA_COLORS];
int num_colors = NUM_DATA_COLORS;
int debug = 0;
const char *font_file_or_name = "Luxi Mono:style=Regular";
const char *legend_orient = "vert";
const char *annotations = NULL;
const char *shadings = NULL;
const char *title = NULL;
const char *legend_scale_name = NULL;
int legend_prefixes_flag = 0;
int reverse_flag = 0;		/* reverse background/font colors */
const char *legend_keyfile = NULL;
const char *savename = "map.png";

/*
 * if log_A and log_B are set, then the input data will be scaled
 * logarithmically such that log_A -> 0 and log_B -> 255. log_C is calculated
 * such that log_B -> 255.
 */
double log_A = 0.0;
double log_B = 0.0;
double log_C = 0.0;

void
initialize(void)
{
    int i;
    int w = 4096;
    int h = 4096;
    if (NULL == title)
	(void)0;		/* no legend */
    else if (0 == strcmp(legend_orient, "horiz"))
	h += 1024;
    else
	w += 1024;
    image = gdImageCreateTrueColor(w, h);
    /* first allocated color becomes background by default */
    if (reverse_flag)
	gdImageFill(image, 0, 0, gdImageColorAllocate(image, 255, 255, 255));

    /*
     * The default color map ranges from red to blue
     */
    for (i = 0; i < NUM_DATA_COLORS; i++) {
	double hue;
	double r, g, b;
	hue = 240.0 * (255 - i) / 255;
	PIX_HSV_TO_RGB_COMMON(hue, 1.0, 1.0, r, g, b);
	colors[i] = gdImageColorAllocate(image, r, g, b);
	if (debug > 1)
	    fprintf(stderr, "colors[%d]=%d\n", i, colors[i]);
    }

    /*
     * If the input data should be logarithmically scaled, then calculate the
     * value of log_C.
     */
    if (0.0 != log_A && 0.0 == log_B)
	log_B = 10.0 * log_A;
    log_C = 255.0 / log(log_B / log_A);
}

void
paint(void)
{
    char buf[512];
    unsigned int line = 1;
    while (fgets(buf, 512, stdin)) {
	unsigned int i;
	unsigned int x;
	unsigned int y;
	int color = -1;

	/*
	 * First field is an IP address.  We also accept its integer notation
	 * equivalent.
	 */
	char *t = strtok(buf, " \t\r\n");
	if (NULL == t)
	    continue;
	if (strspn(t, "0123456789") == strlen(t))
	    i = strtoul(t, NULL, 10) >> 8;
	else if (1 == inet_pton(AF_INET, t, &i))
	    i = ntohl(i) >> 8;
	else
	    errx(1, "bad input on line %d: %s", line, t);
	hil_xy_from_s(i, hilbert_curve_order, &x, &y);
	if (debug)
	    fprintf(stderr, "%s => (%d,%d)\n", t, x, y);

	/*
	 * Second field is an optional value, which might also be
	 * logarithmically scaled by us.  If no value is given, then find the
	 * existing value at that point and increment by one.
	 */
	t = strtok(NULL, " \t\r\n");
	if (NULL != t) {
	    unsigned int k = atoi(t);
	    if (0.0 != log_A) {
		/*
		 * apply logarithmic stretching
		 */
		k = (int)((log_C * log((double)k / log_A)) + 0.5);
	    }
	    if (k < 0)
		k = 0;
	    if (k >= NUM_DATA_COLORS)
		k = NUM_DATA_COLORS - 1;
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
	    color = colors[k + 1];
#endif
	}

	gdImageSetPixel(image, x, y, color);
	line++;
    }
}

void
watermark(void)
{
    int color = gdImageColorAllocateAlpha(image, 127, 127, 127, 63);
    gdImageStringUp(image,
	gdFontGetSmall(),
	4096 - 20, 220,
	"IPv4 Heatmap / Measurement Factory", color);
}

void
save(void)
{
    FILE *pngout = fopen(savename, "wb");
    gdImagePng(image, pngout);
    fclose(pngout);
    gdImageDestroy(image);
    image = NULL;
}

void
usage(const char *argv0)
{
    const char *t = strrchr(argv0, '/');
    printf("IPv4 Heatmap\n");
    printf("(C) 2007 The Measurement Factory, Inc\n");
    printf("Licensed under the GPL, version 2.0\n");
    printf("http://maps.measurement-factory.com/\n");
    printf("\n");
    printf("usage: %s [options] < iplist\n", t ? t + 1 : argv0);
    printf("\t-A float   logarithmic scaling, min value\n");
    printf("\t-B float   logarithmic scaling, max value\n");
    printf("\t-a file    annotations file\n");
    printf("\t-d         increase debugging\n");
    printf("\t-f font    fontconfig name or .ttf file\n");
    printf("\t-h         draw horizontal legend instead\n");
    printf("\t-k file    key file for legend\n");
    printf("\t-o file    output filename\n");
    printf("\t-p         show size of prefixes in legend\n");
    printf("\t-r         reverse; white background, black text\n");
    printf("\t-s file    shading file\n");
    printf("\t-t str     map title\n");
    printf("\t-u str     scale title in legend\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, "A:B:a:df:hk:o:prs:t:u:")) != -1) {
	switch (ch) {
	case 'A':
	    log_A = atof(optarg);
	    break;
	case 'B':
	    log_B = atof(optarg);
	    break;
	case 'd':
	    debug++;
	    break;
	case 'a':
	    annotations = strdup(optarg);
	    break;
	case 's':
	    shadings = strdup(optarg);
	    break;
	case 'f':
	    font_file_or_name = strdup(optarg);
	    break;
	case 'h':
	    legend_orient = "horiz";
	    break;
	case 'k':
	    legend_keyfile = strdup(optarg);
	    break;
	case 'o':
	    savename = strdup(optarg);
	    break;
	case 't':
	    title = strdup(optarg);
	    break;
	case 'p':
	    legend_prefixes_flag = 1;
	    break;
	case 'u':
	    legend_scale_name = strdup(optarg);
	    break;
	case 'r':
	    reverse_flag = 1;
	    break;
	default:
	    usage(argv[0]);
	    break;
	}
    }
    argc -= optind;
    argv += optind;

    initialize();
    paint();
    if (shadings)
	shade_file(shadings);
    if (annotations)
	annotate_file(annotations);
    if (title)
	legend(title, legend_orient);
    watermark();
    save();
    return 0;
}
