/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2.0
 * http://maps.measurement-factory.com/
 */

/*
 * Place annotations (text) on the image.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include <gd.h>
#include "bbox.h"

/*
 * FONT_ALPHA sets the transparency for the annotations. in libgd, 0 means 100%
 * transparent, and 127 means 100% opaque.
 */
#define FONT_ALPHA 45

extern gdImagePtr image;
extern const char *font_file_or_name;
int annotateColor = -1;

extern void text_in_bbox(const char *text, bbox box, int color, double maxsize);
extern int _text_last_height;
extern double _text_last_sz;

/*
 * Calculate the bounding box, draw an outline of the box, then render the
 * text.
 */
void
annotate_cidr(const char *cidr, const char *label)
{
    bbox box = bbox_from_cidr(cidr);
    bbox box2;
    bbox_draw_outline(box, image, annotateColor);
    text_in_bbox(label, box, annotateColor, 128.0);
    box2 = box;
    box2.ymin = (box.ymin + box.ymax) / 2 + (int)(_text_last_sz / 2) + 6;
    box2.ymax = box2.ymin + 24;
    text_in_bbox(cidr, box2, annotateColor, 12.0);
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
