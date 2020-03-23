/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

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

extern const char *font_file_or_name;
double _text_last_sz = 0.0;


/*
 * Calculate the width and height of some text draw at some size
 */
static int *
text_width_height(const char *text, double sz, int *w, int *h)
{
    static int brect[8];
    char *errmsg = gdImageStringFT(NULL, &brect[0], 0,
	(char *)font_file_or_name,
	sz, 0.0, 0, 0, (char *)text);
    if (NULL != errmsg)
	errx(1, "%s", errmsg);
    *w = brect[2] - brect[0];
    *h = brect[3] - brect[5];
    return &brect[0];
}


/*
 * Draws 'text' inside the bbox bounding box with color color. The text is
 * sized to be as large as possible and still fit within the box.
 *
 * If the optional 'text2' is given, then it is drawn below the first text, always
 * in a 12-point size.  This is mainly to print the CIDR prefix values
 * underneath their owner names in each /8 box.
 */
void
text_in_bbox(gdImagePtr image, const char *text, bbox box, int color, double maxsize)
{
    double sz;
    int tw, th;
    int oneline_h;
    int *brectPtr;
    char *text_copy = calloc(1, strlen(text) + 1);
    const char *s;
    char *d;
    /*
     * convert newlines
     */
    for (s = text, d = text_copy; *s; s++, d++) {
	if (*s == '\\' && *(s + 1) == 'n')
	    s++, *d = '\n';
	else
	    *d = *s;
    }
    if (maxsize < 1.0)
	maxsize = 128.0;
    for (sz = maxsize; sz > 6.0; sz *= 0.9) {
	(void)text_width_height("ABCD", sz, &tw, &oneline_h);
	brectPtr = text_width_height(text_copy, sz, &tw, &th);
	if (tw > ((box.xmax - box.xmin) * 95 / 100))
	    continue;
	if (th > ((box.ymax - box.ymin) * 95 / 100))
	    continue;
	gdImageStringFT(image, brectPtr, color,
	    (char *)font_file_or_name, sz, 0.0,
	    ((box.xmin + box.xmax) / 2) - (tw / 2),
	    ((box.ymin + box.ymax) / 2) - (th / 2) + oneline_h,
	    text_copy);
	_text_last_sz = sz;
	break;
    }
}
