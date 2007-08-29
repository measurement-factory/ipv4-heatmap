INCS=-I/usr/local/include
LIBS=-L/usr/local/lib -lgd
CFLAGS=-g -Wall ${INCS}
LDFLAGS=-g
OBJS=\
	ipv4-heatmap.o \
	annotate.o

all: ipv4-heatmap

ipv4-heatmap: ${OBJS}
	${CC} -o $@ ipv4-heatmap.o annotate.o ${LIBS}

clean:
	rm -f ${OBJS}
	rm -f ipv4-heatmap
