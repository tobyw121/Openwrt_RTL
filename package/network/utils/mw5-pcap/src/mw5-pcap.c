/* SPDX-License-Identifier: ISC */
/* Tiny AF_PACKET -> classic PCAP writer for MW5 diagnostics. */
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct pcap_hdr {
    uint32_t magic;
    uint16_t major;
    uint16_t minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct pcap_rec {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

struct stats {
    uint64_t packets, bytes;
    uint64_t arp, ipv4, ipv6, eapol, other;
    uint64_t icmp, tcp, udp, dhcp, dns;
};

static int write_all(int fd, const void *buf, size_t len)
{
    const unsigned char *p = buf;
    while (len) {
        ssize_t n = write(fd, p, len);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0)
            return -1;
        p += n;
        len -= (size_t)n;
    }
    return 0;
}

static uint16_t be16(const unsigned char *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static void classify(const unsigned char *b, size_t n, struct stats *s)
{
    uint16_t et;
    const unsigned char *p;
    size_t l;
    unsigned int ihl;

    if (n < 14) {
        s->other++;
        return;
    }

    et = be16(b + 12);
    p = b + 14;
    l = n - 14;

    if (et == ETH_P_ARP) {
        s->arp++;
        return;
    }
    if (et == ETH_P_PAE) {
        s->eapol++;
        return;
    }
    if (et == ETH_P_IPV6) {
        s->ipv6++;
        return;
    }
    if (et != ETH_P_IP) {
        s->other++;
        return;
    }

    s->ipv4++;
    if (l < 20)
        return;
    ihl = (unsigned int)(p[0] & 0x0f) * 4U;
    if (ihl < 20 || l < ihl)
        return;

    if (p[9] == 1) {
        s->icmp++;
    } else if (p[9] == 6) {
        s->tcp++;
        if (l >= ihl + 4) {
            uint16_t sp = be16(p + ihl), dp = be16(p + ihl + 2);
            if (sp == 53 || dp == 53)
                s->dns++;
        }
    } else if (p[9] == 17) {
        s->udp++;
        if (l >= ihl + 8) {
            uint16_t sp = be16(p + ihl), dp = be16(p + ihl + 2);
            if (sp == 53 || dp == 53)
                s->dns++;
            if (sp == 67 || sp == 68 || dp == 67 || dp == 68)
                s->dhcp++;
        }
    }
}

static uint64_t mono_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

int main(int argc, char **argv)
{
    const char *ifname, *out;
    long seconds;
    unsigned int ifindex;
    int sfd = -1, ofd = -1, rc = 1;
    struct sockaddr_ll addr;
    struct pcap_hdr gh = { 0xa1b2c3d4U, 2, 4, 0, 0, 65535U, 1U };
    unsigned char buf[65535];
    struct stats st = {0};
    uint64_t end_ms;

    if (argc != 4) {
        fprintf(stderr, "usage: mw5-pcap IFACE OUTPUT.pcap SECONDS\n");
        return 2;
    }
    ifname = argv[1];
    out = argv[2];
    seconds = strtol(argv[3], NULL, 10);
    if (seconds < 1 || seconds > 300) {
        fprintf(stderr, "invalid duration: %s\n", argv[3]);
        return 2;
    }

    ifindex = if_nametoindex(ifname);
    if (!ifindex) {
        fprintf(stderr, "%s: no such interface\n", ifname);
        return 3;
    }

    sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sfd < 0) {
        perror("socket(AF_PACKET)");
        goto out;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = (int)ifindex;
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind(AF_PACKET)");
        goto out;
    }

    ofd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (ofd < 0) {
        perror(out);
        goto out;
    }
    if (write_all(ofd, &gh, sizeof(gh)) < 0) {
        perror("write pcap header");
        goto out;
    }

    end_ms = mono_ms() + (uint64_t)seconds * 1000ULL;
    while (mono_ms() < end_ms) {
        struct pollfd pfd = { .fd = sfd, .events = POLLIN };
        uint64_t now = mono_ms();
        int timeout = (end_ms > now) ? (int)(end_ms - now) : 0;
        int pr;
        ssize_t n;
        struct timeval tv;
        struct pcap_rec rec;

        if (timeout > 1000)
            timeout = 1000;
        pr = poll(&pfd, 1, timeout);
        if (pr < 0) {
            if (errno == EINTR)
                continue;
            perror("poll");
            goto out;
        }
        if (pr == 0 || !(pfd.revents & POLLIN))
            continue;

        n = recv(sfd, buf, sizeof(buf), 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("recv");
            goto out;
        }
        if (gettimeofday(&tv, NULL) != 0)
            memset(&tv, 0, sizeof(tv));
        rec.ts_sec = (uint32_t)tv.tv_sec;
        rec.ts_usec = (uint32_t)tv.tv_usec;
        rec.incl_len = (uint32_t)n;
        rec.orig_len = (uint32_t)n;
        if (write_all(ofd, &rec, sizeof(rec)) < 0 ||
            write_all(ofd, buf, (size_t)n) < 0) {
            perror("write pcap record");
            goto out;
        }
        st.packets++;
        st.bytes += (uint64_t)n;
        classify(buf, (size_t)n, &st);
    }

    fprintf(stderr,
            "iface=%s packets=%llu bytes=%llu arp=%llu ipv4=%llu ipv6=%llu eapol=%llu icmp=%llu tcp=%llu udp=%llu dhcp=%llu dns=%llu other=%llu\n",
            ifname,
            (unsigned long long)st.packets,
            (unsigned long long)st.bytes,
            (unsigned long long)st.arp,
            (unsigned long long)st.ipv4,
            (unsigned long long)st.ipv6,
            (unsigned long long)st.eapol,
            (unsigned long long)st.icmp,
            (unsigned long long)st.tcp,
            (unsigned long long)st.udp,
            (unsigned long long)st.dhcp,
            (unsigned long long)st.dns,
            (unsigned long long)st.other);
    rc = 0;

out:
    if (ofd >= 0)
        close(ofd);
    if (sfd >= 0)
        close(sfd);
    return rc;
}
