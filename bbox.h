
#ifndef BBOX_H
#define BBOX_H

struct bb {
    int xmin, ymin, xmax, ymax;
};

typedef struct bb bbox;

#define BBOX_WIDTH(b) (b.xmax-b.xmin)
#define BBOX_HEIGHT(b) (b.ymax-b.ymin)
#define BBOX_CTR_X(b) ((b.xmax+b.xmin)/2)
#define BBOX_CTR_Y(b) ((b.ymax+b.ymin)/2)
#define BBOX_SET(B,W,X,Y,Z) B.xmin=W; B.ymin=X; B.xmax=Y; B.ymax=Z;
#define BBOX_PRINT(B) fprintf(stderr, "%s=%d,%d,%d,%d\n", #B, B.xmin,B.ymin,B.xmax,B.ymax)

void bbox_draw_outline(bbox box, gdImagePtr image, int color);
void bbox_draw_filled(bbox box, gdImagePtr image, int color);
bbox bbox_from_cidr(const char *prefix);

#endif
