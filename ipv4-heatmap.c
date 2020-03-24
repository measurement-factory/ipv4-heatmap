/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
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
 * output is a squre PNG file
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
#include "cidr.h"
#include "ipv4-heatmap.h"
#include "annotate.h"
#include "shade.h"
#include "legend.h"
#include "xy_from_ip.h"

#define NUM_DATA_COLORS 256
#undef RELEASE_VER

gdImagePtr image = NULL;
int colors[NUM_DATA_COLORS];
int num_colors = NUM_DATA_COLORS;
int debug = 0;
const char *whitespace = " \t\r\n";
const char *font_file_or_name = "Luxi Mono:style=Regular";
const char *legend_orient = "vert";
const char *annotations = NULL;
const char *shadings = NULL;
const char *title = NULL;
const char *legend_scale_name = NULL;
int legend_prefixes_flag = 0;
int reverse_flag = 0;		/* reverse background/font colors */
int morton_flag = 0;
int accumulate_counts = 0;	/* for when the input data contains a value */
struct {
	unsigned int secs;
	double input_time;
	time_t next_output;
} anim_gif = {0, 0.0, 0};
const char *legend_keyfile = NULL;
const char *savename = "map.png";

static void savegif(int done);
static void annotate(gdImagePtr);

/*
 * if log_A and log_B are set, then the input data will be scaled
 * logarithmically such that log_A -> 0 and log_B -> 255. log_C is calculated
 * such that log_B -> 255.
 */
double log_A = 0.0;
double log_B = 0.0;
double log_C = 0.0;

static void
initialize(void)
{
    int i;
    int w;
    int h;
    int order = set_order();
    w = 1<<order;
    h = 1<<order;
    if (title && 4096 != w) {
	warnx("Image width/height must be 4096 to render a legend.");
	fprintf(stderr,
		"\nIf you are using the -y or -z options, then your image size "
		"may be smaller\n(or larger) than 4096.  The legend-rendering "
		"code has a number of hard-coded\nparameters designed to work "
		"with a 4096x4096 output image.\n");
	exit(1);
    }
    if (NULL == title)
	(void)0;		/* no legend */
    else if (0 == strcmp(legend_orient, "horiz"))
	h += (h>>2);
    else
	w += (w>>2);
    if (debug) {
	fprintf(stderr, "image width = %d\n", w);
	fprintf(stderr, "image height = %d\n", h);
    }
    image = gdImageCreateTrueColor(w, h);
    if (image == NULL)
	err(1, "gdImageCreateTrueColor(w=%d, h=%d)", w, h);
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

static int
get_pixel_value(unsigned int x, unsigned int y)
{
    int color;
    int k;
    color = gdImageGetPixel(image, x, y);
    if (debug)
	fprintf(stderr, "pixel (%d,%d) has color index %d\n", x, y, color);
    for (k = 0; k < NUM_DATA_COLORS; k++) {
	if (colors[k] == color) {
	    if (debug)
		fprintf(stderr, "color %d has index %d\n", color, k);
	    break;
	}
    }
    if (k == NUM_DATA_COLORS)	/* not found */
	k = 0;
    return k;
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
	int k;
	char *strtok_arg = buf;
	char *t;

	/*
	 * In animated gif mode the first field is a timestamp
	 */
	if (anim_gif.secs) {
	    char *e;
	    t = strtok(strtok_arg, whitespace);
	    strtok_arg = NULL;
	    if (NULL == t)
		continue;
	    anim_gif.input_time = strtod(t, &e);
	    if (e == t)
		errx(1, "bad input parsing time on line %d: %s", line, t);
	}

	/*
	 * next field is an IP address.  We also accept its integer notation
	 * equivalent.
	 */
	t = strtok(strtok_arg, whitespace);
	strtok_arg = NULL;
	if (NULL == t)
	    continue;
	if (strspn(t, "0123456789") == strlen(t))
	    i = strtoul(t, NULL, 10);
	else if (1 == inet_pton(AF_INET, t, &i))
	    i = ntohl(i);
	else
	    errx(1, "bad input parsing IP on line %d: %s", line, t);

	if (0 == xy_from_ip(i, &x, &y))
	    continue;
	if (debug)
	    fprintf(stderr, "%s => %u => (%d,%d)\n", t, i, x, y);

	/*
	 * next field is an optional value, which might also be
	 * logarithmically scaled by us.  If no value is given, then find the
	 * existing value at that point and increment by one.
	 */
	t = strtok(NULL, whitespace);
	if (NULL != t) {
	    k = atoi(t);
	    if (accumulate_counts)
		k += get_pixel_value(x, y);
	    if (0.0 != log_A) {
		/*
		 * apply logarithmic stretching
		 */
		k = (int) ((log_C * log((double) k / log_A)) + 0.5);
	    }
	} else {
	    k = get_pixel_value(x, y);
	    k++;
	}
	if (k < 0)
	    k = 0;
	if (k >= NUM_DATA_COLORS)
	    k = NUM_DATA_COLORS - 1;
	color = colors[k];

	/*
	 * Now that we're doing parsing the entire input line, we can check if
	 * an animated gif file needs to be written out.  This is done here because
         * saving the gif image can call annotation routines that also use strtok().
	 */
	if (anim_gif.secs) {
	    if ((time_t) anim_gif.input_time > anim_gif.next_output) {
		savegif(0);
		anim_gif.next_output = (time_t) anim_gif.input_time + anim_gif.secs;
	    }
	}

	gdImageSetPixel(image, x, y, color);
	line++;
    }
}

void
watermark(gdImagePtr i)
{
    int color = gdImageColorAllocateAlpha(i, 127, 127, 127, 63);
    gdImageStringUp(i,
	gdFontGetSmall(),
	gdImageSX(i) - 20, 220,
	(u_char *) "IPv4 Heatmap / Measurement Factory", color);
}

void
save(void)
{
    FILE *pngout = fopen(savename, "wb");
    if (NULL == pngout)
	err(1, "%s", savename);
    annotate(image);
    gdImagePng(image, pngout);
    fclose(pngout);
    gdImageDestroy(image);
    image = NULL;
}

void
savegif(int done)
{
	static int ngif = 0;
	static char *tdir = NULL;
	static char tmpl[] = "heatmap-tmp-XXXXXX";
	char fname[512];
	FILE *gifout = NULL;
	gdImagePtr clone;
	if (NULL == tdir) {
		tdir = mkdtemp(tmpl);
		if (NULL == tdir)
			err(1, "%s", tmpl);
	}
	snprintf(fname, 512, "%s/%07d.gif", tdir, ngif++);
	gifout = fopen(fname, "wb");
	if (NULL == gifout)	
		err(1, "%s", fname);
	clone = gdImageClone(image);
	if (NULL == clone)
		errx(1, "gdImageClone() failed");
	annotate(clone);
	gdImageGif(clone, gifout);
	fclose(gifout);
	gdImageDestroy(clone);
	/* don't destroy image! */
	if (done) {
		char cmd[512];
		snprintf(cmd, 512, "gifsicle --colors 256 %s/*.gif > %s", tdir, savename);
		fprintf(stderr, "Executing: %s\n", cmd);
		if (0 != system(cmd))
			errx(1, "gifsicle failed");
		snprintf(cmd, 512, "rm -rf %s", tdir);
		fprintf(stderr, "Executing: %s\n", cmd);
		system(cmd);
		tdir = NULL;
		gdImageDestroy(image);
		image = NULL;
	}
}

static void
annotate(gdImagePtr i)
{
    if (shadings)
	shade_file(i, shadings);
    if (annotations)
	annotate_file(i, annotations);
    if (title)
	legend(i, title, legend_orient);
    watermark(i);
}

void
usage(const char *argv0)
{
    const char *t = strrchr(argv0, '/');
    printf("IPv4 Heatmap"
#ifdef RELEASE_VER
	" (release " RELEASE_VER ")"
#endif
	"\n");
    printf("(C) 2007 The Measurement Factory, Inc\n");
    printf("Licensed under the GPL, version 2\n");
    printf("http://maps.measurement-factory.com/\n");
    printf("\n");
    printf("usage: %s [options] < iplist\n", t ? t + 1 : argv0);
    printf("\t-A float   logarithmic scaling, min value\n");
    printf("\t-B float   logarithmic scaling, max value\n");
    printf("\t-C         values accumulate in Exact input mode\n");
    printf("\t-a file    annotations file\n");
    printf("\t-c color   color of annotations (0xRRGGBB)\n");
    printf("\t-d         increase debugging\n");
    printf("\t-f font    fontconfig name or .ttf file\n");
    printf("\t-g secs    make animated gif from each secs of data\n");
    printf("\t-h         draw horizontal legend instead\n");
    printf("\t-k file    key file for legend\n");
    printf("\t-m         use morton order instead of hilbert\n");
    printf("\t-o file    output filename\n");
    printf("\t-p         show size of prefixes in legend\n");
    printf("\t-r         reverse; white background, black text\n");
    printf("\t-s file    shading file\n");
    printf("\t-t str     map title\n");
    printf("\t-u str     scale title in legend\n");
    printf("\t-y cidr    address space to render\n");
    printf("\t-z bits    address space bits per pixel\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, "A:B:a:Cc:df:g:hk:mo:prs:t:u:y:z:")) != -1) {
	switch (ch) {
	case 'A':
	    log_A = atof(optarg);
	    break;
	case 'B':
	    log_B = atof(optarg);
	    break;
	case 'C':
	    accumulate_counts = 1;
	    break;
	case 'd':
	    debug++;
	    break;
	case 'a':
	    annotations = strdup(optarg);
	    break;
	case 'c':
	    annotateColor = strtol(optarg, NULL, 16);
	    break;
	case 's':
	    shadings = strdup(optarg);
	    break;
	case 'f':
	    font_file_or_name = strdup(optarg);
	    break;
	case 'g':
	    anim_gif.secs = strtol(optarg, NULL, 10);
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
	case 'm':
		morton_flag = 1;
		set_morton_mode();
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
	case 'y':
	    set_crop(optarg);
	    break;
	case 'z':
	    set_bits_per_pixel(strtol(optarg, NULL, 10));
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
    if (anim_gif.secs) {
	savegif(1);
    } else {
	annotate(image);
    	save();
    }
    return 0;
}
