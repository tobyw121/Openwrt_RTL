// SPDX-License-Identifier: GPL-2.0-or-later
#include <errno.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE 0x89F0
#endif

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s [eth0] reseed|rxstart|vendor-side\n", prog);
}

int main(int argc, char **argv)
{
    const char *dev = "eth0";
    const char *cmd;
    struct ifreq ifr;
    int fd, req;

    if (argc == 2) {
        cmd = argv[1];
    } else if (argc == 3) {
        dev = argv[1];
        cmd = argv[2];
    } else {
        usage(argv[0]);
        return 2;
    }

    if (!strcmp(cmd, "reseed"))
        req = SIOCDEVPRIVATE + 0;
    else if (!strcmp(cmd, "rxstart"))
        req = SIOCDEVPRIVATE + 1;
    else if (!strcmp(cmd, "vendor-side"))
        req = SIOCDEVPRIVATE + 2;
    else {
        usage(argv[0]);
        return 2;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", dev);

    if (ioctl(fd, req, &ifr) < 0) {
        fprintf(stderr, "%s: ioctl 0x%x on %s failed: %s\n", argv[0], req, dev, strerror(errno));
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
