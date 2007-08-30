/*
 * input is a list of IPv4 addrs (or thier integer representation) and an
 * intensity value 0--255.
 * 
 * data is drawn using a hilbert curve, which preserves grouping see
 * http://xkcd.com/195/ see http://en.wikipedia.org/wiki/Hilbert_curve see
 * Hacker's Delight (Henry S. Warren, Jr. 2002), sec 14-2, fig 14-5
 * 
 * output is a 4096x4096 PNG file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gd.h>

extern gdImagePtr image;
extern void hil_xy_from_s(unsigned s, int n, unsigned *xp, unsigned *yp);
extern int debug;
extern const char *font_file_or_name;
int fontColor = -1;
unsigned int allones = ~0;

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)
#endif

/*
 * A bounding box
 */
struct bb {
    int xmin, ymin, xmax, ymax;
};

/*
 * Find the "bounding box" for the CIDR prefix starting at 'first'
 * and having "slash" value of 'slash'
 * 
 * For square areas this is pretty easy.  We know how to find the
 * point diagonally opposite the first value (add 1010..1010). Its
 * a little harder for rectangular areas, so I cheat a little and
 * divide it into the two smaller squares.
 */
struct bb
bounding_box(unsigned int first, int slash)
{
    struct bb box;
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

void
annotate_text(const char *text, const char *text2, struct bb box)
{
    double sz;
    int brect[8];
    char *errmsg;
    for (sz = 64.0; sz > 0.0; sz *= 0.9) {
	errmsg = gdImageStringFT(NULL, &brect[0], 0,
		(char *) font_file_or_name,
		sz, 0.0, 0, 0, (char*)text);
	int tw, th;
	if (NULL != errmsg)
		errx(1, errmsg);
	tw = brect[2] - brect[0];
	th = brect[5] - brect[3];
	if (tw > (box.xmax - box.xmin))
		continue;
	if (th > (box.ymax - box.ymin))
		continue;
	gdImageStringFT(image, &brect[0], fontColor,
		(char*) font_file_or_name, sz, 0.0,
		((box.xmin + box.xmax) / 2) - (tw/2),
		((box.ymin + box.ymax) / 2) - (th/2),
		(char*)text);
	break;
    }
}

void
annotate_cidr(const char *cidr, const char *label)
{
    char cidr_copy[24];
    char *t;
    int slash;
    unsigned int first;
    unsigned int last;
    struct bb bbox;
    gdPoint points[4];
    strncpy(cidr_copy, cidr, 24);
    t = strchr(cidr_copy, '/');
    if (NULL == t) {
	fprintf(stderr, "WARNING: missing / on CIDR '%s'\n", cidr_copy);
	return;
    }
    *t++ = '\0';
    slash = atoi(t);
    if (1 != inet_pton(AF_INET, cidr_copy, &first)) {
	fprintf(stderr, "WARNING: inet_pton failed on '%s'\n", cidr_copy);
	return;
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
    points[0].x = bbox.xmin;
    points[1].x = bbox.xmax;
    points[2].x = bbox.xmax;
    points[3].x = bbox.xmin;
    points[0].y = bbox.ymin;
    points[1].y = bbox.ymin;
    points[2].y = bbox.ymax;
    points[3].y = bbox.ymax;
    gdImagePolygon(image, points, 4, fontColor);
    annotate_text(label, cidr, bbox);
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
    if (fontColor < 0)
	fontColor = gdImageColorAllocateAlpha(image, 255, 255, 255, 45);
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
