/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

/*
 * Shading routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include <gd.h>
#include "bbox.h"

extern int debug;

static void
shade_cidr(gdImagePtr image, const char *cidr, unsigned int rgb, int alpha)
{
    bbox box = bbox_from_cidr(cidr);
    int color = gdImageColorAllocateAlpha(image,
	rgb >> 16,
	(rgb >> 8) & 0xFF,
	rgb & 0xFF,
	alpha);
    if (box.xmin != box.xmax || box.ymin != box.ymax)
	bbox_draw_filled(box, image, color);
    else
	bbox_draw_outline(box, image, color);
}

/*
 * Input is a text file with TAB-separated fields First field is a CIDR address
 * Second field is an RGB value Third field is an alpha value
 */
void
shade_file(gdImagePtr image, const char *fn)
{
    char buf[512];
    FILE *fp = fopen(fn, "r");
    if (NULL == fp)
	err(1, "%s", fn);
    while (NULL != fgets(buf, 512, fp)) {
	char *cidr;
	char *rgbhex;
	char *alpha_str;
	unsigned int rgb;
	int alpha;
	cidr = strtok(buf, "\t");
	if (NULL == cidr)
	    continue;
	rgbhex = strtok(NULL, "\t\r\n");
	if (NULL == rgbhex)
	    continue;
	rgb = strtol(rgbhex, NULL, 16);
	alpha_str = strtok(NULL, "\t\r\n");
	if (NULL == alpha_str)
	    continue;
	alpha = strtol(alpha_str, NULL, 10);
	shade_cidr(image, cidr, rgb, alpha);
    }
    fclose(fp);
}
