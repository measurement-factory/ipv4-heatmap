#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <err.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*
 * Counts IPv4 addresses
 */

struct _item {
	struct in_addr a;
	unsigned int c;
	struct _item *next;
};
typedef struct _item item;

item *array[65536];

item *
find(struct in_addr a, int new)
{
	unsigned int b = ntohl(a.s_addr) >> 16;
	item **I;
	item *i;
	for (I = &array[b]; *I; I=&(*I)->next)
		if ((*I)->a.s_addr == a.s_addr) {
			i = *I;
			if (i != array[b]) {
				/* move to top */
				*I = i->next;
				i->next = array[b];
				array[b] = i;
			}
			return i;
		}
	if (!new)
		return NULL;
	i = calloc(1, sizeof(*i));
	if (NULL == i)
		err(1, "malloc");
	i->a = a;
	i->next = array[b];
	array[b] = i;
	return i;
}

int
main(int argc, char *argv[])
{
	char buf[128];
	unsigned int b;
	while (NULL != fgets(buf, sizeof(buf), stdin)) {
		struct in_addr a;
		item *i;
		strtok(buf, "\r\n");
		if (1 != inet_pton(AF_INET, buf, &a))
			continue;
		i = find(a, 1);
		i->c++;
	}
	for (b = 0; b<65536; b++) {
		item *i;
		for (i = array[b]; i; i=i->next) {
			if (NULL == inet_ntop(AF_INET, &i->a, buf, 128))
				continue;
			printf("%s\t%u\n", buf, i->c);
		}
	}
	return 0;
}
