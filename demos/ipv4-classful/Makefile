NAME=classful

all: ${NAME}.png ${NAME}-z.png

${NAME}.png: ${NAME}.annotations ${NAME}.shade
	../../ipv4-heatmap \
	  -s ${NAME}.shade \
	  -a ${NAME}.annotations \
	  -f /usr/X11R6/lib/X11/fonts/TTF/luximr.ttf \
	  -c 0 \
	  < /dev/null
	mv map.png $@

${NAME}-z.png: ${NAME}.annotations ${NAME}.shade
	../..//ipv4-heatmap \
	  -m \
	  -s ${NAME}.shade \
	  -a ${NAME}.annotations \
	  -f /usr/X11R6/lib/X11/fonts/TTF/luximr.ttf \
	  -c 0 \
	  < /dev/null
	mv map.png $@

clean:
	rm *.png
