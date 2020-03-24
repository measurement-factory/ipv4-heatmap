#if 0
annotate_file(gdImagePtr image, const char *fn);
bbox_draw_filled(bbox box, gdImagePtr image, int color);
bbox_draw_outline(bbox box, gdImagePtr image, int color);
bbox_from_cidr(const char *cidr);
cidr_parse(const char *cidr, unsigned int *rfirst, unsigned int *rlast, int *rslash);
hil_xy_from_s(unsigned s, int order, unsigned *xp, unsigned *yp);
legend(gdImagePtr image, const char *title, const char *orient);
mor_xy_from_s(unsigned s, int order, unsigned *xp, unsigned *yp);
set_bits_per_pixel(int bpp);
set_crop(const char *cidr);
set_morton_mode();
set_order();
shade_file(gdImagePtr image, const char *fn);
text_in_bbox(gdImagePtr image, const char *text, bbox box, int color, double maxsize);
xy_from_ip(unsigned ip, unsigned *xp, unsigned *yp);
#endif

extern const char *font_file_or_name;
extern const char *legend_keyfile;
extern const char *legend_scale_name;
extern double log_A;
extern double log_C;
extern int colors[];
extern int debug;
extern int legend_prefixes_flag;
extern int morton_flag;
extern int num_colors;
extern int reverse_flag;


