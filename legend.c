/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

/*
 * Legend rendering routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <math.h>

#include <gd.h>
#include "bbox.h"

#define MAX(a,b) (a>b?a:b)

bbox legend_bb;
bbox AAA;
bbox BBB;
bbox CCC;

static int textColor;

extern int debug;
extern int annotateColor;
extern int colors[];
extern int num_colors;
extern const char *font_file_or_name;
extern const char *legend_scale_name;
extern int legend_prefixes_flag;
extern const char *legend_keyfile;
extern int reverse_flag;
extern double log_A;
extern double log_C;
extern void text_in_bbox(gdImagePtr image, const char *text, bbox box, int color, double maxsize);


/*
 * Show how big various blocks are
 */
static void
legend_prefixes(gdImagePtr image)
{
    char *sample_cidr[] = {
	"0.0.0.0/8",
	"0.0.0.0/12",
	"0.0.0.0/16",
	"0.0.0.0/20",
	"0.0.0.0/24",
	NULL,
    };
    unsigned int i = 0;
    int samplebox_ctr_x;
    int samplebox_ctr_y;
    bbox tbox;
    BBOX_SET(tbox,
	CCC.xmin,
	CCC.ymin,
	CCC.xmax,
	CCC.ymin + 128);
    text_in_bbox(image, "Prefix Sizes", tbox, textColor, 0.0);
    samplebox_ctr_x = CCC.xmin + 256;
    samplebox_ctr_y = tbox.ymax + 192;
    while (sample_cidr[i]) {
	bbox box;
	bbox tbox;
	char tstr[10];
	int hw;
	int hh;
	box = bbox_from_cidr(sample_cidr[i]);
	hw = BBOX_WIDTH(box) / 2;
	hh = BBOX_HEIGHT(box) / 2;
	box.xmin += samplebox_ctr_x - hw;
	box.xmax += samplebox_ctr_x - hw;
	box.ymin += samplebox_ctr_y - hh;
	box.ymax += samplebox_ctr_y - hh;
	gdImageFilledRectangle(image,
	    box.xmin, box.ymin, box.xmax, box.ymax,
	    colors[127]);
	tbox.xmin = samplebox_ctr_x + 128;
	tbox.xmax = tbox.xmin + 256;
	tbox.ymin = ((box.ymin + box.ymax) / 2) - 30;
	tbox.ymax = ((box.ymin + box.ymax) / 2) + 30;
	snprintf(tstr, 10, "= %s", strchr(sample_cidr[i], '/'));
	text_in_bbox(image, tstr, tbox, textColor, 0.0);
	samplebox_ctr_y += MAX(hh + 64, 64);
	i++;
    }
}

static void
legend_scale(gdImagePtr image, const char *orient)
{
    unsigned int i;
    bbox tbox;
    int pct_inc = 10;
    BBOX_SET(tbox,
	BBB.xmin,
	BBB.ymin,
	BBB.xmax,
	BBB.ymin + 128);
    text_in_bbox(image, legend_scale_name, tbox, textColor, 0.0);
    for (i = 0; i < num_colors; i++) {
	if (0 == strcmp(orient, "vert")) {
	    BBOX_SET(tbox,
		BBB.xmin + 128,
		BBB.ymin + 256 + ((num_colors - i - 1) * 4),
		BBB.xmin + 256,
		BBB.ymin + 256 + ((num_colors - i - 1) * 4) + 3);
	} else {
	    pct_inc = 25;
	    BBOX_SET(tbox,
		BBB.xmin + 256 + (i * 4),
		BBB.ymin + 256,
		BBB.xmin + 256 + (i * 4) + 3,
		BBB.ymin + 384);
	}
	gdImageFilledRectangle(image,
	    tbox.xmin, tbox.ymin, tbox.xmax, tbox.ymax,
	    colors[i]);
    }

    for (i = 0; i <= 100; i += pct_inc) {
	char tmp[10];
	if (0.0 == log_A) {
	    snprintf(tmp, 10, "%d%%", i);
	} else {
	    snprintf(tmp, 10, "%d", (int)(log_A * exp(2.55 * i / log_C) + 0.5));
	}
	if (0 == strcmp(orient, "vert")) {
	    BBOX_SET(tbox,
		BBB.xmin + 256,
		BBB.ymin + 256 + ((num_colors - (i * 2.55) - 1) * 4) - 30,
		BBB.xmin + 512,
		BBB.ymin + 256 + ((num_colors - (i * 2.55) - 1) * 4) + 30);
	} else {
	    BBOX_SET(tbox,
		BBB.xmin + 256 + (i * 2.55 * 4) - 256,
		BBB.ymin + 442,
		BBB.xmin + 256 + (i * 2.55 * 4) + 256,
		BBB.ymin + 442 + 72);
	}
	text_in_bbox(image, tmp, tbox, textColor, 0.0);
    }
}

/*
 * Read a "key" file and draw a legend First line of the key file is a
 * description of the legend (e.g. "Announcement Size".  Remaining lines begin
 * with hex RGB color values (e.g., "0x00FF00").  An optional label follows the
 * color. This code currently makes assumptions about the length of the label
 * strings.  It also assumes that not every color will be labelled -- that is,
 * text is larger than the boxes
 */
static void
legend_key(gdImagePtr image, const char *orient, const char *file)
{
    unsigned int i = 0;
    bbox tbox;
    char buf[128];
    FILE *fp = fopen(file, "r");
    if (NULL == fp)
	return;
    BBOX_SET(tbox,
	BBB.xmin,
	BBB.ymin,
	BBB.xmax,
	BBB.ymin + 128);
    if (NULL != fgets(buf, 128, fp)) {
	strtok(buf, "\r\n");
	text_in_bbox(image, buf, tbox, textColor, 0.0);
    }
    while (NULL != fgets(buf, 128, fp)) {
	char *rgbhex;
	char *label;
	unsigned int rgb;
	int color;
	bbox sbox;
	if (NULL == (rgbhex = strtok(buf, " \t")))
	    continue;
	rgb = strtol(rgbhex, NULL, 16);
	if (0 == strcmp(orient, "vert")) {
	    BBOX_SET(tbox,
		BBB.xmin + 128,
		BBB.ymin + 256 + (i * 80),
		BBB.xmin + 256,
		BBB.ymin + 256 + (i * 80) + 64);
	    BBOX_SET(sbox,
		tbox.xmax + 32,
		tbox.ymin - 64,
		tbox.xmax + 32 + 256,
		tbox.ymax + 64);
	} else {
	    BBOX_SET(tbox,
		BBB.xmin + 256 + (i * 80),
		BBB.ymin + 256,
		BBB.xmin + 256 + (i * 80) + 64,
		BBB.ymin + 384);
	    BBOX_SET(sbox,
		tbox.xmin - 64,
		tbox.ymax + 16,
		tbox.xmax + 64,
		tbox.ymax + 16 + 128);
	}
	color = gdImageColorAllocate(image,
	    rgb >> 16,
	    (rgb >> 8) & 0xFF,
	    rgb & 0xFF);
	gdImageFilledRectangle(image,
	    tbox.xmin, tbox.ymin, tbox.xmax, tbox.ymax,
	    color);
	if (NULL != (label = strtok(NULL, " \t\r\n")))
	    text_in_bbox(image, label, sbox, textColor, 0.0);
	i++;
    }
}

static void
legend_title(gdImagePtr image, const char *text)
{
    text_in_bbox(image, text, AAA, textColor, 0.0);
}

#if SEPARATE_LEGEND_FILE
static void
legend_save(gdImagePtr image, const char *orient)
{
    FILE *pngout;
    char fname[128];
    snprintf(fname, 128, "legend-%s.png", orient);
    pngout = fopen(fname, "wb");
    if (NULL == pngout)
	err(1, "%s", fname);
    gdImagePng(image, pngout);
    fclose(pngout);
}
#endif

/*
 * Render a legend
 */
void
legend(gdImagePtr image, const char *title, const char *orient)
{
    if (0 == strcmp(orient, "vert")) {
	BBOX_SET(legend_bb, 4096, 0, 1024, 4096);
	BBOX_SET(AAA,
	    legend_bb.xmin,
	    legend_bb.ymin,
	    legend_bb.xmin + 1024,
	    legend_bb.ymin + 1024);
	BBOX_SET(BBB,
	    AAA.xmin + 128,
	    AAA.ymax + 128,
	    AAA.xmax - 128,
	    AAA.ymax + 128 + 1400);
	BBOX_SET(CCC,
	    BBB.xmin,
	    BBB.ymax + 128,
	    BBB.xmax,
	    BBB.ymax + 128 + 1024);
    } else if (0 == strcmp(orient, "horiz")) {
	BBOX_SET(legend_bb, 0, 4096, 4096, 1024);
	BBOX_SET(AAA,
	    legend_bb.xmin,
	    legend_bb.ymin,
	    legend_bb.xmin + 1024,
	    legend_bb.ymin + 1024);
	BBOX_SET(BBB,
	    AAA.xmax + 128,
	    AAA.ymin + 128,
	    AAA.xmax + 128 + 1400,
	    AAA.ymax - 128);
	BBOX_SET(CCC,
	    BBB.xmax + 128,
	    BBB.ymin,
	    BBB.xmax + 128 + 1024,
	    BBB.ymin);
    } else {
	errx(1, "bad orientation: %s\n", orient);
    }

#if SEPARATE_LEGEND_FILE
    image = gdImageCreateTrueColor(legend_bb.xmax, legend_bb.ymax);
    if (reverse_flag)
	gdImageFill(l, 0, 0, gdImageColorAllocate(legend_image, 255, 255, 255));
#endif
    if (!reverse_flag)
	textColor = gdImageColorAllocate(image, 255, 255, 255);
    else
	textColor = gdImageColorAllocate(image, 0, 0, 0);

    if (0 == strcmp(orient, "vert")) {
	gdImageLine(image,
	    legend_bb.xmin,
	    legend_bb.ymin,
	    legend_bb.xmin,
	    legend_bb.ymax,
	    annotateColor);
    } else {
	gdImageLine(image,
	    legend_bb.xmin,
	    legend_bb.ymin,
	    legend_bb.xmax,
	    legend_bb.ymin,
	    annotateColor);
    }

    legend_title(image, title);
    if (legend_scale_name)
	legend_scale(image, orient);
    if (legend_keyfile)
	legend_key(image, orient, legend_keyfile);
    if (legend_prefixes_flag)
	legend_prefixes(image);
#if SEPARATE_LEGEND_FILE
    legend_save(image, orient);
    gdImageDestroy(image);
    image = NULL;
#endif
}
