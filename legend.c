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

struct bb {
    int xmin, ymin, xmax, ymax;
};

struct bb legend_bb = { 4096, 0, 5120, 4096};
struct bb legend_title_bb = { 4224, 128, 4992, 512 };
struct bb legend_legend_bb = { 4224, 768, 4352, 1024 };

char *sample_cidr[] = {
	"0.0.0.0/8",
	"0.0.0.0/12",
	"0.0.0.0/16",
	"0.0.0.0/20",
	"0.0.0.0/24",
	NULL,
};

extern gdImagePtr image;
extern int debug;
extern int fontColor;
extern void annotate_text(const char *text, const char *text2, struct bb box);
extern struct bb cidr_to_bbox(const char *);
extern int colors[];
extern int num_colors;

static void
legend_samples(void)
{
	unsigned int i = 0;
	while (sample_cidr[i]) {
		struct bb box;
		box = cidr_to_bbox(sample_cidr[i]);
		box.xmin += legend_title_bb.xmin;
		box.xmax += legend_title_bb.xmin;
		box.ymin += 2048 + (i*275);
		box.ymax += 2048 + (i*275);
		gdImageFilledRectangle(image,
			box.xmin, box.ymin, box.xmax, box.ymax,
			colors[0]);
		i++;
	}
}


static void
legend_legend(void)
{
	unsigned int i;
	struct bb desc_bb = legend_title_bb;
	desc_bb.ymax = legend_legend_bb.ymin;
	desc_bb.ymin = legend_legend_bb.ymin - 256;
	annotate_text("Utilization", NULL, desc_bb);
	for (i=0; i< num_colors; i++) {
		gdImageFilledRectangle(image,
			legend_legend_bb.xmin, legend_legend_bb.ymin+(i*4),
			legend_legend_bb.xmax, legend_legend_bb.ymin+(i*4)+3,
			colors[i]);
	}

	for (i=0; i<=100 ; i += 10) {
		char tmp[10];
		struct bb box;
		snprintf(tmp, 10, "%d%%", i);
		box.xmin = legend_legend_bb.xmax;
		box.xmax = legend_legend_bb.xmax + 256;
		box.ymin = legend_legend_bb.ymin+(i*4*2.55)-30;
		box.ymax = legend_legend_bb.ymin+(i*4*2.55)+30;
		annotate_text(tmp, NULL, box);
	}
}

static void
legend_title(const char *text)
{
	annotate_text(text, NULL, legend_title_bb);
}

/*
 */
void
legend(const char *title)
{
    gdImageLine(image, legend_bb.xmin, legend_bb.ymin, legend_bb.xmin, legend_bb.ymax, fontColor);
    legend_title(title);
    legend_legend();
    legend_samples();
}
