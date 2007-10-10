/*
 * Draws text on the image
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include <gd.h>
#include "bbox.h"

/*
 * FONT_ALPHA sets the transparency for the annotations. in libgd,
 * 0 means 100% transparent, and 127 means 100% opaque.
 */
#define FONT_ALPHA 45

extern gdImagePtr image;
extern const char *font_file_or_name;
int annotateColor = -1;


/*
 * Draws 'text' inside the bbox bounding box with color color.
 * The text is sized to be as large as possible and still
 * fit within the box.
 *
 * If the optional 'text2' is given, then it is drawn below
 * the first text, always in a 12-point size.  This is mainly
 * to print the CIDR prefix values underneath their owner names
 * in each /8 box.
 */
void
annotate_text(const char *text, const char *text2, bbox box, int color)
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
	gdImageStringFT(image, &brect[0], color,
	    (char *)font_file_or_name, sz, 0.0,
	    ((box.xmin + box.xmax) / 2) - (tw / 2),
	    ((box.ymin + box.ymax) / 2) + (th / 2),
	    (char *)text);
	break;
    }
    if (NULL == text2)
	return;

    sz = 12.0;
    errmsg = gdImageStringFT(NULL, &brect[0], 0,
	(char *)font_file_or_name,
	sz, 0.0, 0, 0, (char *)text2);
    tw = brect[2] - brect[0];
    /* don't update th, we need the previous value */
    gdImageStringFT(image, &brect[0], color,
	(char *)font_file_or_name, sz, 0.0,
	((box.xmin + box.xmax) / 2) - (tw / 2),
	((box.ymin + box.ymax) / 2) + (th / 2) + (sz * 2),
	(char *)text2);
}

/*
 * Calculate the bounding box, draw an outline of the box,
 * then render the text.
 */
void
annotate_cidr(const char *cidr, const char *label)
{
    bbox box = bbox_from_cidr(cidr);
    bbox_draw_outline(box, image, annotateColor);
    annotate_text(label, cidr, box, annotateColor);
}

/*
 * Input is a text file with TAB-separated fields First field is a CIDR address
 * Second field is a text label
 */
void
annotate_file(const char *fn)
{
    char buf[512];
    FILE *fp = fopen(fn, "r");
    if (NULL == fp)
	err(1, fn);
    if (annotateColor < 0)
	annotateColor = gdImageColorAllocateAlpha(image, 255, 255, 255, FONT_ALPHA);
    if (!gdFTUseFontConfig(1))
	warnx("fontconfig not available");
    while (NULL != fgets(buf, 512, fp)) {
	char *cidr;
	char *label;
	cidr = strtok(buf, "\t");
	if (NULL == cidr)
	    continue;
	label = strtok(NULL, "\t\r\n");
	if (NULL == label)
	    continue;
	annotate_cidr(cidr, label);
    }
    fclose(fp);
}
