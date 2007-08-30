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

extern gdImagePtr image;
extern int debug;
extern int fontColor;
extern void annotate_text(const char *text, const char *text2, struct bb box);


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
}
