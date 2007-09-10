/*
 * Legend rendering routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <assert.h>

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
legend_samples(void)
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
legend_legend(const char *orient)
{
    unsigned int i;
    bbox tbox;
    int pct_inc = 10;
    BBOX_SET(tbox,
	BBB.xmin,
	BBB.ymin,
	BBB.xmax,
	BBB.ymin + 128);
    legend_text("Utilization", tbox);
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
}

static void
legend_title(const char *text)
{
    legend_text(text, AAA);
}

static void
legend_save(void)
{
    FILE *pngout = fopen("legend.png", "wb");
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
	    legend_bb.xmin + 128,
	    legend_bb.ymin + 128,
	    legend_bb.xmin + 1024 - 128,
	    legend_bb.ymin + 1024 - 128);
	BBOX_SET(BBB,
	    AAA.xmin,
	    AAA.ymax + 128,
	    AAA.xmax,
	    AAA.ymax + 128 + 1400);
	BBOX_SET(CCC,
	    BBB.xmin,
	    BBB.ymax + 128,
	    BBB.xmax,
	    BBB.ymax + 128 + 1024);
    } else if (0 == strcmp(orient, "horiz")) {
	BBOX_SET(legend_bb, 0, 0, 4096, 1024);
	BBOX_SET(AAA,
	    legend_bb.xmin + 128,
	    legend_bb.ymin + 128,
	    legend_bb.xmin + 1024 - 128,
	    legend_bb.ymin + 1024 - 128);
	BBOX_SET(BBB,
	    AAA.xmax + 128,
	    AAA.ymin,
	    AAA.xmax + 128 + 1400,
	    AAA.ymax);
	BBOX_SET(CCC,
	    BBB.xmax + 128,
	    BBB.ymin,
	    BBB.xmax + 128 + 1024,
	    BBB.ymin);
    } else {
	errx(1, "bad orientation: %s\n", orient);
    }

    image = gdImageCreateTrueColor(legend_bb.xmax, legend_bb.ymax);
    gdImageColorAllocate(image, 0, 0, 0);
    textColor = gdImageColorAllocate(image, 255, 255, 255);

    gdImageLine(image,
	legend_bb.xmin,
	legend_bb.ymin,
	legend_bb.xmin,
	legend_bb.ymax,
	annotateColor);

    legend_title(title);
    legend_legend(orient);
    legend_samples();
    legend_save();
}
