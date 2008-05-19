/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <err.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cidr.h"

unsigned int allones = ~0;

/*
 * Parse a CIDR string
 */
int
cidr_parse(const char *cidr, unsigned int *rfirst, unsigned int *rlast, int *rslash)
{
    char cidr_copy[24];
    char *t;
    int slash;
    unsigned int first;
    unsigned int last;
    strncpy(cidr_copy, cidr, 24);
    t = strchr(cidr_copy, '/');
    if (NULL == t) {
	warnx("missing / on CIDR '%s'\n", cidr_copy);
	return 0;;
    }
    *t++ = '\0';
    slash = atoi(t);
    if (1 != inet_pton(AF_INET, cidr_copy, &first)) {
	warnx("inet_pton failed on '%s'\n", cidr_copy);
	return 0;
    }
    first = ntohl(first);
    if (slash < 32)
	last = first | (allones >> slash);
    else
	last = first;
    *rfirst = first;
    *rlast = last;
    *rslash = slash;
    return 1;
}
