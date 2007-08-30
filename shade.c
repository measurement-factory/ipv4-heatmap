/*
 * Shading routines
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

extern gdImagePtr image;
extern int debug;
extern struct bb cidr_to_bbox(const char *);

void
shade_cidr(const char *cidr, unsigned int rgb, int alpha)
{
    struct bb bbox = cidr_to_bbox(cidr);
    int color = gdImageColorAllocateAlpha(image,
	rgb >> 16,
	(rgb >> 8) & 0xFF,
	rgb & 0xFF,
	alpha);
    gdPoint points[4];
    points[0].x = bbox.xmin;
    points[1].x = bbox.xmax;
    points[2].x = bbox.xmax;
    points[3].x = bbox.xmin;
    points[0].y = bbox.ymin;
    points[1].y = bbox.ymin;
    points[2].y = bbox.ymax;
    points[3].y = bbox.ymax;
    gdImageFilledPolygon(image, points, 4, color);
}

/*
 * Input is a text file with TAB-separated fields First field is a CIDR address
 * Second field is an RGB value Third field is an alpha value
 */
void
shade_file(const char *fn)
{
    char buf[512];
    FILE *fp = fopen(fn, "r");
    if (NULL == fp)
	err(1, fn);
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
	shade_cidr(cidr, rgb, alpha);
    }
    fclose(fp);
}
