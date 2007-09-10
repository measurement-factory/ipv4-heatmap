#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gd.h>
#include "bbox.h"

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)
#endif

extern void hil_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp);
unsigned int allones = ~0;
extern int debug;

void
bbox_draw_outline(bbox box, gdImagePtr image, int color)
{
    gdPoint points[4];
    points[0].x = box.xmin;
    points[1].x = box.xmax;
    points[2].x = box.xmax;
    points[3].x = box.xmin;
    points[0].y = box.ymin;
    points[1].y = box.ymin;
    points[2].y = box.ymax;
    points[3].y = box.ymax;
    gdImagePolygon(image, points, 4, color);
}

void
bbox_draw_filled(bbox box, gdImagePtr image, int color)
{
    gdPoint points[4];
    points[0].x = box.xmin;
    points[1].x = box.xmax;
    points[2].x = box.xmax;
    points[3].x = box.xmin;
    points[0].y = box.ymin;
    points[1].y = box.ymin;
    points[2].y = box.ymax;
    points[3].y = box.ymax;
    gdImageFilledPolygon(image, points, 4, color);
}

/*
 * Find the "bounding box" for the IPv4 netblock starting at 'first' and having
 * 'slash' netmask bits.
 * 
 * For square areas this is pretty easy.  We know how to find the point diagonally
 * opposite the first value (add 1010..1010). Its a little harder for
 * rectangular areas, so I cheat a little and divide it into the two smaller
 * squares.
 */
static bbox
bounding_box(unsigned int first, int slash)
{
    bbox box;
    unsigned int diag = 0xAAAAAAAA;
    int x1, y1, x2, y2;

    /*
     * find the point diagonally opposite the first point
     */
    if (0 == (slash & 1)) {
	/* square */
	diag >>= slash;
	hil_xy_from_s(first >> 8, 12, &x1, &y1);
	hil_xy_from_s((first + diag) >> 8, 12, &x2, &y2);
	box.xmin = MIN(x1, x2);
	box.ymin = MIN(y1, y2);
	box.xmax = MAX(x1, x2);
	box.ymax = MAX(y1, y2);
    } else {
	/* rectangle */
	int x3, y3, x4, y4;
	slash += 1;
	diag >>= slash;
	hil_xy_from_s(first >> 8, 12, &x1, &y1);
	hil_xy_from_s((first + diag) >> 8, 12, &x2, &y2);
	first += (1 << (32 - slash));
	hil_xy_from_s((first) >> 8, 12, &x3, &y3);
	hil_xy_from_s((first + diag) >> 8, 12, &x4, &y4);
	box.xmin = MIN(MIN(x1, x2), MIN(x3, x4));
	box.ymin = MIN(MIN(y1, y2), MIN(y3, y4));
	box.xmax = MAX(MAX(x1, x2), MAX(x3, x4));
	box.ymax = MAX(MAX(y1, y2), MAX(y3, y4));
    }
    return box;
}

bbox
bbox_from_cidr(const char *cidr)
{
    char cidr_copy[24];
    char *t;
    int slash;
    unsigned int first;
    unsigned int last;
    bbox bbox;
    memset(&bbox, '\0', sizeof(bbox));
    strncpy(cidr_copy, cidr, 24);
    t = strchr(cidr_copy, '/');
    if (NULL == t) {
	warnx("missing / on CIDR '%s'\n", cidr_copy);
	return bbox;
    }
    *t++ = '\0';
    slash = atoi(t);
    if (1 != inet_pton(AF_INET, cidr_copy, &first)) {
	warnx("inet_pton failed on '%s'\n", cidr_copy);
	return bbox;
    }
    first = ntohl(first);
    last = first | (allones >> slash);
    bbox = bounding_box(first, slash);
    if (debug) {
	char fstr[24];
	char lstr[24];
	unsigned int tmp;
	tmp = htonl(first);
	inet_ntop(AF_INET, &tmp, fstr, 24);
	tmp = htonl(last);
	inet_ntop(AF_INET, &tmp, lstr, 24);
	fprintf(stderr, "cidr=%s"
	    ", first=%s, last=%s, last-first=%u"
	    ", bbox=%d,%d to %d,%d"
	    "\n",
	    cidr, fstr, lstr, last - first,
	    bbox.xmin, bbox.ymin, bbox.xmax, bbox.ymax);
    }
    return bbox;
}
