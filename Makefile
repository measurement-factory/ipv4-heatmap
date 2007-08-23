INCS=-I/usr/local/include
LIBS=-L/usr/local/lib -lgd
CFLAGS=-g ${INCS}
LDFLAGS=-g

all: ipv4-heatmap

ipv4-heatmap: ipv4-heatmap.o
	${CC} -o $@ ipv4-heatmap.o ${LIBS}

clean:
	rm -f ipv4-heatmap.o
	rm -f ipv4-heatmap
