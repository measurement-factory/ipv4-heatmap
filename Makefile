INCS=-I/usr/local/include
LIBS=-L/usr/local/lib -lgd
CFLAGS=-g -Wall ${INCS}
LDFLAGS=-g
OBJS=\
	ipv4-heatmap.o \
	hilbert.o \
	annotate.o \
	shade.o \
	legend.o \
	bbox.o \
	text.o

all: ipv4-heatmap

ipv4-heatmap: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS}
	rm -f ipv4-heatmap
