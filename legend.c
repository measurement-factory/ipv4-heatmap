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

static gdImagePtr image;
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
extern double log_B;
extern double log_C;

int *
legend_text_width_height(const char *text, double sz, int *w, int *h)
{
    static int brect[8];
    char *errmsg = gdImageStringFT(NULL, &brect[0], 0,
	    (char *)font_file_or_name,
	    sz, 0.0, 0, 0, (char *)text);
	if (NULL != errmsg)
	    errx(1, errmsg);
	*w = brect[2] - brect[0];
	*h = brect[3] - brect[5];
	return &brect[0];
}


/*
 * XXX too much like annotate_text().  need to merge them.
 */
void
legend_text(const char *text, bbox box)
{
    double sz;
    int tw, th;
    int oneline_h;
    int *brectPtr;
    char *text_copy = calloc(1, strlen(text)+1);
    const char *s;
    char *d;
    for (s=text, d=text_copy; *s; s++, d++) {
	if (*s == '\\' && *(s+1) == 'n')
		s++, *d = '\n';
	else
		*d = *s;
    }
    for (sz = 128.0; sz > 6.0; sz *= 0.9) {
	(void) legend_text_width_height("ABCD", sz, &tw, &oneline_h);
	brectPtr = legend_text_width_height(text_copy, sz, &tw, &th);
	if (tw > ((box.xmax - box.xmin) * 95 / 100))
	    continue;
	if (th > ((box.ymax - box.ymin) * 95 / 100))
	    continue;
	gdImageStringFT(image, brectPtr, textColor,
	    (char *)font_file_or_name, sz, 0.0,
	    ((box.xmin + box.xmax) / 2) - (tw / 2),
	    ((box.ymin + box.ymax) / 2) - (th / 2) + oneline_h,
	    text_copy);
	break;
    }
}


/*
 * Show how big various blocks are
 */
static void
legend_prefixes(void)
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
    legend_text("Prefix Sizes", tbox);
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
	legend_text(tstr, tbox);
	samplebox_ctr_y += MAX(hh + 64, 64);
	i++;
    }
}

static void
legend_utilization(const char *orient)
{
    unsigned int i;
    bbox tbox;
    int pct_inc = 10;
    BBOX_SET(tbox,
	BBB.xmin,
	BBB.ymin,
	BBB.xmax,
	BBB.ymin + 128);
    legend_text(legend_scale_name, tbox);
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
		snprintf(tmp, 10, "%d", (int) (log_A * exp(((double)i - log_C)/log_B) + 0.5));
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
	legend_text(tmp, tbox);
    }
}

/*
 * Read a "key" file and draw a legend
 * First line of the key file is a description of the legend (e.g.
 * "Announcement Size".  Remaining lines begin with hex RGB color
 * values (e.g., "0x00FF00").  An optional label follows the color.
 * This code currently makes assumptions about the length of the
 * label strings.  It also assumes that not every color will be
 * labelled -- that is, text is larger than the boxes
 */
static void
legend_key(const char *orient, const char *file)
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
    	legend_text(buf, tbox);
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
		legend_text(label, sbox);
	i++;
    }

#if 0
    for (i = 0; i <= 100; i += pct_inc) {
	char tmp[10];
	snprintf(tmp, 10, "%d%%", i);
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
	legend_text(tmp, tbox);
    }
#endif
}

static void
legend_title(const char *text)
{
    legend_text(text, AAA);
}

static void
legend_save(const char *orient)
{
    FILE *pngout;
    char fname[128];
    snprintf(fname, 128, "legend-%s.png", orient);
    pngout = fopen(fname, "wb");
    gdImagePng(image, pngout);
    fclose(pngout);
    gdImageDestroy(image);
    image = NULL;
}

/*
 * */
void
legend(const char *title, const char *orient)
{
    if (0 == strcmp(orient, "vert")) {
	BBOX_SET(legend_bb, 0, 0, 1024, 4096);
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
	BBOX_SET(legend_bb, 0, 0, 4096, 1024);
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

    image = gdImageCreateTrueColor(legend_bb.xmax, legend_bb.ymax);
    if (reverse_flag)
	gdImageFill(image, 0, 0, gdImageColorAllocate(image, 255, 255, 255));
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

    legend_title(title);
    if (legend_scale_name)
	legend_utilization(orient);
    if (legend_keyfile)
	legend_key(orient, legend_keyfile);
    if (legend_prefixes_flag)
	legend_prefixes();
    legend_save(orient);
}
