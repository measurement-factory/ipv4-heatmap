INCS=-I/usr/local/include
LIBS=-L/usr/local/lib -lgd
CFLAGS=${INCS}

all: ipv4-heatmap

ipv4-heatmap: ipv4-heatmap.o
	${CC} -o $@ ipv4-heatmap.o ${LIBS}
