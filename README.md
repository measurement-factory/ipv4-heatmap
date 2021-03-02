# ipv4-heatmap
Generate Hilbert curve heatmaps of the IPv4 address space.

This software was inspired by https://xkcd.com/195/

See http://maps.measurement-factory.com/ for additional information and a gallery of examples.

# Dependencies

- GD library

# Installing Dependencies & Compiling

- apt-get install libgd-dev build-essential
- make

# Documentation

## NAME
     ipv4‐heatmap — Create a map of IPv4 address data

## SYNOPSIS
     ipv4‐heatmap [−dhprm] [−A float] [−B float] [−a file] [−f font]
                  [−g seconds] [−k file] [−o file] [−s file] [−t string]
                  [−u string] [−y prefix] [−z bits] < iplist

## DESCRIPTION
     ipv4‐heatmap is a program that generates a map of IPv4 address data using
     a space‐filling Hilbert Curve.  Inspiration for ipv4‐heatmap comes from
     the xkcd comic (http://www.xkcd.org/195/).

     The output of ipv4‐heatmap is a 4096x4096 PNG image.  Each pixel in the
     image represents a single /24 network and is assigned one of 256 colors.
     Typically, the pixel color represents the number of hosts within the /24
     belonging to a dataset or having some property, such as being pingable or
     being the source of some traffic.

     Pixel colors range from blue (1 host) to red (256 hosts), while black
     represents no data (0 hosts).  Of course, the colors and pixel values may
     also be used to represent other properties of IPv4 addresses.

     The map may be annotated by placing transparent text labels over specific
     regions of address space.

     ipv4‐heatmap also supports shading of areas specified by CIDR netblocks.
     This is useful to show reserved and other special address space, for
     example.

     ipv4‐heatmap can also add an optional legend to the map.

     ipv4‐heatmap uses the GD library (libgd) to create the map image.

## COMMAND LINE OPTIONS
     The options are as follows:

     −A logmin
             Input data will be scaled logarithmically such that input values
             less than or equal to logmin will be set to 1.

     −B logmax
             Input data will be scaled logarithmically such that input values
             greater than or equal to logmax will be set to 255.

     −a annotations
             The annotations file contains a list of annotations for the map.
             See ANNOTATIONS below for the format of this file.

     −c color
             The color of the annotations (those that appear inside the map).
             Specified as 0xRRGGBB.

     −d      increase debugging levels.

     −f font
             Specifies the font to use for the legend and annotations.  If
             libgd was compiled with fontconfig support, then this can be a
             fontconfig string such as "Times‐12:bold".  Otherwise, you can
             also specify the pathname to a True Type Font (.ttf) file.

     −g seconds
             This option enables animated GIF output mode.  A new frame is
             created for each seconds interval of the input data file.  See
             the ANIMATED GIFS below for additional details.

     −h      Attach a horizontal legend to the bottom of the map.  Note that
             the legend is drawn only if the −t option is given.

     −k keyfile
             Use keyfile to create the legend scale, rather than the built‐in
             blue‐to‐red scale.

     −m      Use Morton (aka "Z") Curve ordering instead of Hilbert.

     −o outfile
             Output file name.  If none is given, the image is saved as
             map.png by default.

     −p      Include a section in the legend that shows the size of CIDR pre‐
             fixes.  Boxes and labels will be drawn to show the size of /8,
             /12, /16, /20, and /24 prefixes.

     −r      Reverse the background and foreground colors.

     −s shades
             The shades file can be used to shade certain areas of the map
             with specific colors and transparency levels.  See SHADING below
             for the format of this file.

     −t title
             Instructs ipv4‐heatmap to draw a legend for the map and place the
             title string in the top (or right) section.  You may use "\n" to
             create multi‐line titles.  By default the legend will be drawn
             vertically and attached to the right side of the map.  Use the −h
             option to create a horizontal legend instead.

     −u string
             Instructs ipv4‐heatmap to draw a scale in the legend showing the
             range of colors and their values.  string will be placed above
             the scale.  Currently, ipv4‐heatmap always assumes the data rep‐
             resents some kind of utilization and prints percentages from 0 to
             100% next to the scale.

     −y cidr
             Specifies the CIDR netblock that should be rendered.  The default
             is to render the entire IPv4 space (0.0.0.0/0).  The "slash"
             value must be even so that the output image is square.  The −y
             and −z options together determine the size of the output image.

     −z bits
             Specifies the number of address space bits assigned to each pixel
             in the output image.  By default each pixel represents a /24 net‐
             work, which corresponds to 8 host bits (i.e., 256 hosts).  Spec‐
             ify 0 here for one pixel per host address.

## INPUT MODES
     ipv4‐heatmap accepts three input modes:

     1.   Increment mode.

          In this mode, the input consists of IPv4 addresses only, one address
          per line.  Each address in the list increments the pixel value for
          the corresponding /24.  In this mode, repeated addresses may lead to
          false results because ipv4‐heatmap does not check for uniqueness of
          the input values.  It silently limits pixels to the maximum value.
          The user should run the data through sort(1) and uniq(1) beforehand,
          if necessary.

     2.   Exact mode.

          In this mode, the input consists of two whitespace‐separated fields:
          an IPv4 address and a color index.  The color index is an integer in
          the range 0‐‐255.  Be careful with this mode because later addresses
          may overwrite earlier ones in the same /24.

     3.   Logarithmic mode.

          Very similar to Exact mode, except that the second column is loga‐
          rithmically scaled to calculate the color index.  In order to use
          this mode the input must have two fields, and the −A logmin and −B
          logmax command line options must be given.  Color index k is calcu‐
          lated from input value i according to this formula:

                            ln ( i / logmin)
                k = 255 * ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
                          ln (logmax / logmin)

## ANNOTATIONS
     The annotations file consists of two or three TAB‐separated fields.  The
     first field is a CIDR prefix, and the second is the annotation string.
     The annotation string is rendered within the boundary of that prefix,
     centered both vertically and horizontally.  The third field, if present,
     is also rendered just below the annotation text in a fixed‐size 12‐point
     font.  If the third field is equal to the string "prefix" then the CIDR
     prefix (from field 1) is rendered instead.

     The annotations file might look like this, for example:

           15.0.0.0/8      HP
           16.0.0.0/8      DEC
           17.0.0.0/8      Apple

     The ipv4‐heatmap source code distribution should include a file named
     iana‐labels.txt, which is based on the list of IPv4 assignments made by
     IANA.

     The font can be selected with the −f command line option.  At this time,
     however, the text color and transparency are hard‐coded in the
     ipv4‐heatmap program.

## SHADING
     Areas of the map can be "shaded" by using the −s shades option.  This was
     originally developed simply to highlight address space that is designated
     reserved or unallocated, but is also a useful way to render complex map
     data.  The shades file consists of three TAB‐separated fields: prefix,
     color, and alpha value.

     The CIDR prefix specifies the area to be shaded.  The color should be
     specified has a hexadecimal value beginning with "0x".  The alpha value
     controls the transparency of the shaded area and is passed directly to
     the GD library functions.  An alpha value of 0 means full transparency,
     while a value of 127 means no transparency (full opacity).  Here is an
     example that shows RFC 1918 address space in a light purple color:

           10.0.0.0/8      0x7F7FFF        64
           172.16.0.0/12   0x7F7FFF        64
           192.168.0.0/16  0x7F7FFF        64

## ANIMATED GIFS
     When the −g option is given, ipv4‐heatmap outputs an animated GIF image
     file.  This feature requires the gifsicle(1) program to be installed.

     This feature also requires timestamps in the input data.  Thus, use of
     the −g option changes the input format.  Each line of the input must
     begin with a timestamp given in Unix epoch time.  For example:

           1234567890.123  192.168.1.1
           1234567890.234  192.168.1.2
           1234567891.456  192.168.1.3

     Note that decimal time values are accepted, although the fractional sec‐
     onds are ignored.  ipv4‐heatmap Assumes that the input timestamps are
     already sorted.  A new output frame is generated every seconds seconds of
     the input file.

     Note that, currently, the data accumulates between frames.  That is, any
     pixels that are colored at the end of one frame will also be colored at
     the start of the next frame.

## HILBERT CURVE
     ipv4‐heatmap uses a 12th‐order Hilbert Curve to represnet the entire IPv4
     address space.  Locating a particular IP address along the curve can be
     confusing at first.  Here is what a 2nd‐order Hilbert curve looks like:

               0‐‐‐1   14‐‐15
                   |   |
               3‐‐‐2   13‐‐12
               |            |
               4   7‐‐‐8   11
               |   |   |    |
               5‐‐‐6   9‐‐‐10

     The best way to understand how the Hilbert Curve works is to try drawing
     your own!

## COPYRIGHT
           IPv4 Heatmap
           (C) 2011 The Measurement Factory, Inc
           Licensed under the GPL, version 2
           http://maps.measurement‐factory.com/

## SEE ALSO
     gifsicle(1)

## AUTHORS
     ipv4‐heatmap was written by Duane Wessels of The Measurement Factory,
     Inc.  With contributions from: Roy Arends of Nominet UK.

## BUGS
     Can’t draw IPv6 address maps.

     The legends don’t look all that great.  You can use an image editing pro‐
     gram like The Gimp to rearrange the legend and add better‐looking text.
