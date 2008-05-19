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
	text.o \
	cidr.o

all: ipv4-heatmap

ipv4-heatmap: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS}
	rm -f ipv4-heatmap

install: ipv4-heatmap
	install -C -m 755 ipv4-heatmap /usr/local/bin
	install -C -m 755 ipv4-heatmap.1 /usr/local/man/man1
