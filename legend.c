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

#define MAX(a,b) (a>b?a:b)

struct bb {
    int xmin, ymin, xmax, ymax;
};
#define BOX_WIDTH(b) (b.xmax-b.xmin)
#define BOX_HEIGHT(b) (b.ymax-b.ymin)
#define BOX_CTR_X(b) ((b.xmax+b.xmin)/2)
#define BOX_CTR_Y(b) ((b.ymax+b.ymin)/2)
#define BOX_SET(B,W,X,Y,Z) B.xmin=W; B.ymin=X; B.xmax=Y; B.ymax=Z;
#define BOX_PRINT(B) fprintf(stderr, "%s=%d,%d,%d,%d\n", #B, B.xmin,B.ymin,B.xmax,B.ymax)

struct bb legend_bb;
struct bb AAA;
struct bb BBB;
struct bb CCC;

static gdImagePtr image;
static int textColor;

extern int debug;
extern int annotateColor;
extern struct bb cidr_to_bbox(const char *);
extern int colors[];
extern int num_colors;
extern const char *font_file_or_name;

/*
 * XXX too much like annotate_text().  need to merge them.
 */
void
legend_text(const char *text, struct bb box)
{
    double sz;
    int brect[8];
    char *errmsg;
    int tw, th;
    for (sz = 128.0; sz > 6.0; sz *= 0.9) {
        errmsg = gdImageStringFT(NULL, &brect[0], 0,
            (char *)font_file_or_name,
            sz, 0.0, 0, 0, (char *)text);
        if (NULL != errmsg)
            errx(1, errmsg);
        tw = brect[2] - brect[0];
        th = brect[3] - brect[5];
        if (tw > ((box.xmax - box.xmin) * 95 / 100))
            continue;
        if (th > ((box.ymax - box.ymin) * 95 / 100))
            continue;
        gdImageStringFT(image, &brect[0], textColor,
            (char *)font_file_or_name, sz, 0.0,
            ((box.xmin + box.xmax) / 2) - (tw / 2),
            ((box.ymin + box.ymax) / 2) + (th / 2),
            (char *)text);
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
    struct bb tbox;
    BOX_SET(tbox,
	CCC.xmin,
	CCC.ymin,
	CCC.xmax,
	CCC.ymin + 128);
    legend_text("Prefix Sizes", tbox);
    samplebox_ctr_x = CCC.xmin + 256;
    samplebox_ctr_y = tbox.ymax + 192;
    while (sample_cidr[i]) {
	struct bb box;
	struct bb tbox;
	char tstr[10];
	int hw;
	int hh;
	box = cidr_to_bbox(sample_cidr[i]);
	hw = BOX_WIDTH(box)/2;
	hh = BOX_HEIGHT(box)/2;
	box.xmin += samplebox_ctr_x - hw;
	box.xmax += samplebox_ctr_x - hw;
	box.ymin += samplebox_ctr_y - hh;
	box.ymax += samplebox_ctr_y - hh;
	gdImageFilledRectangle(image,
	    box.xmin, box.ymin, box.xmax, box.ymax,
	    colors[127]);
	tbox.xmin = samplebox_ctr_x + 128;
	tbox.xmax = tbox.xmin + 256;
	tbox.ymin = ((box.ymin+box.ymax)/2) - 30;
	tbox.ymax = ((box.ymin+box.ymax)/2) + 30;
	snprintf(tstr, 10, "= %s", strchr(sample_cidr[i], '/'));
	legend_text(tstr, tbox);
	samplebox_ctr_y += MAX(hh+64, 64);
	i++;
    }
}


static void
legend_legend(void)
{
    unsigned int i;
    struct bb tbox;
    BOX_SET(tbox,
	BBB.xmin,
	BBB.ymin,
	BBB.xmax,
	BBB.ymin + 128);
    legend_text("Utilization", tbox);
    for (i = 0; i < num_colors; i++) {
	BOX_SET(tbox,
		BBB.xmin + 128,
		BBB.ymin + 256 + ((num_colors-i-1) * 4),
		BBB.xmin + 256,
		BBB.ymin + 256 + ((num_colors-i-1) * 4) + 3);
	gdImageFilledRectangle(image,
	    tbox.xmin, tbox.ymin, tbox.xmax, tbox.ymax,
	    colors[i]);
    }

    for (i = 0; i <= 100; i += 10) {
	char tmp[10];
	snprintf(tmp, 10, "%d%%", i);
	BOX_SET(tbox,
		 BBB.xmin + 256,
		 BBB.ymin + 256 + ((num_colors-(i*2.55)-1) * 4) - 30,
		 BBB.xmin + 512,
		 BBB.ymin + 256 + ((num_colors-(i*2.55)-1) * 4) + 30);
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

void
draw_box(struct bb bbox)
{
    gdPoint points[4];
    points[0].x = bbox.xmin;
    points[1].x = bbox.xmax;
    points[2].x = bbox.xmax;
    points[3].x = bbox.xmin;
    points[0].y = bbox.ymin;
    points[1].y = bbox.ymin;
    points[2].y = bbox.ymax;
    points[3].y = bbox.ymax;
    gdImagePolygon(image, points, 4, annotateColor);
}

/*
 * */
void
legend(const char *title, const char *orient)
{
    if (0 == strcmp(orient, "vert")) {
	BOX_SET(legend_bb, 0, 0, 1024, 4096);
	BOX_SET(AAA,
		legend_bb.xmin + 128,
		legend_bb.ymin + 128,
		legend_bb.xmin + 1024 - 128,
		legend_bb.ymin + 1024 - 128);
	BOX_SET(BBB,
		AAA.xmin,
		AAA.ymax + 128,
		AAA.xmax,
		AAA.ymax + 128 + 1400);
	BOX_SET(CCC,
		BBB.xmin,
		BBB.ymax + 128,
		BBB.xmax,
		BBB.ymax + 128 + 1024);
    } else if (0 == strcmp(orient, "horiz")) {
	BOX_SET(legend_bb, 0, 0, 4096, 1024);
	BOX_SET(AAA,
		legend_bb.xmin + 128,
		legend_bb.ymin + 128,
		legend_bb.xmin + 1024 - 128,
		legend_bb.ymin + 1024 - 128);
	BOX_SET(BBB,
		AAA.xmax + 128,
		AAA.ymin,
		AAA.xmax + 128 + 1400,
		AAA.ymax);
	BOX_SET(CCC,
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

    BOX_PRINT(AAA);
    draw_box(AAA);
    BOX_PRINT(BBB);
    draw_box(BBB);
    draw_box(CCC);

    legend_title(title);
    legend_legend();
    legend_samples();
    legend_save();
}
