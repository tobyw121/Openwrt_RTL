// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * rd05-raw - bounded AF_PACKET diagnostics for Xiaomi RD05.
 *
 * The utility intentionally uses only libc/Linux UAPI interfaces so it stays
 * small enough for the OpenWrt image.  It sends/receives an experimental raw
 * Ethernet frame (EtherType 0x8899) whose payload contains RD05MAGIC-V31.
 * The RTL8197F driver scans that token before DSA/bridge processing, while a
 * PC peer can use the same token to correlate the end-to-end path.
 */
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef ETH_P_REALTEK
#define ETH_P_REALTEK 0x8899
#endif

#ifndef PACKET_IGNORE_OUTGOING
#define PACKET_IGNORE_OUTGOING 23
#endif

#define RD05_TOKEN "RD05MAGIC-V31"
#define RD05_FRAME_MAX 2048U
#define RD05_DEFAULT_SNAPLEN 256U
#define RD05_DEFAULT_MAX_PACKETS 8192U

struct __attribute__((packed)) pcap_file_header {
    uint32_t magic;
    uint16_t major;
    uint16_t minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct __attribute__((packed)) pcap_record_header {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

struct pcap_writer {
    FILE *fp;
    uint32_t snaplen;
    uint32_t max_packets;
    uint32_t packets;
    uint32_t omitted;
    char path[256];
};

struct counters {
    uint64_t rx_total;
    uint64_t rx_bytes;
    uint64_t rx_broadcast;
    uint64_t rx_multicast;
    uint64_t rx_arp;
    uint64_t rx_ipv4;
    uint64_t rx_ipv6;
    uint64_t rx_realtek;
    uint64_t rx_other;
    uint64_t rx_magic;
    uint64_t rx_pc_probe;
    uint64_t rx_pc_ack;
    uint64_t rx_router_probe;
    uint64_t rx_router_ack;
    uint64_t tx_probe;
    uint64_t tx_ack;
    uint64_t tx_error;
};

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s exchange IFACE SECONDS SESSION [PEER_MAC|-] [INTERVAL_MS] [PCAP]\n"
            "  %s capture  IFACE SECONDS PCAP\n"
            "  %s send     IFACE DST_MAC ROLE TYPE SESSION SEQ [COUNT] [INTERVAL_MS]\n"
            "\n"
            "exchange sends router probes, captures traffic and ACKs PC probes.\n"
            "capture records bounded Ethernet PCAP data without transmitting.\n"
            "send emits explicit RD05MAGIC-V31 frames for a focused test.\n"
            "Environment: RD05_RAW_SNAPLEN (default %u), RD05_RAW_MAX_PACKETS "
            "(default %u).\n",
            prog, prog, prog, RD05_DEFAULT_SNAPLEN,
            RD05_DEFAULT_MAX_PACKETS);
}

static uint64_t monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static unsigned long parse_ulong(const char *text, unsigned long min,
                                 unsigned long max, const char *what)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 0);
    if (errno || !text[0] || !end || *end || value < min || value > max) {
        fprintf(stderr, "invalid %s: %s (expected %lu..%lu)\n",
                what, text, min, max);
        exit(2);
    }
    return value;
}

static bool parse_mac(const char *text, uint8_t mac[ETH_ALEN])
{
    unsigned int b[ETH_ALEN];
    char trailing;
    int n;

    if (!text || !strcmp(text, "-"))
        return false;
    n = sscanf(text, "%x:%x:%x:%x:%x:%x%c",
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &trailing);
    if (n != ETH_ALEN)
        return false;
    for (n = 0; n < ETH_ALEN; n++) {
        if (b[n] > 0xff)
            return false;
        mac[n] = (uint8_t)b[n];
    }
    return true;
}

static void format_mac(const uint8_t mac[ETH_ALEN], char out[18])
{
    snprintf(out, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static bool is_zero_mac(const uint8_t mac[ETH_ALEN])
{
    static const uint8_t zero[ETH_ALEN];

    return !memcmp(mac, zero, ETH_ALEN);
}

static bool is_broadcast_mac(const uint8_t mac[ETH_ALEN])
{
    static const uint8_t broadcast[ETH_ALEN] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    return !memcmp(mac, broadcast, ETH_ALEN);
}

static bool is_multicast_mac(const uint8_t mac[ETH_ALEN])
{
    return !!(mac[0] & 1U);
}

static void sanitize_token(const char *src, char *dst, size_t dst_len)
{
    size_t i, out = 0;

    if (!dst_len)
        return;
    for (i = 0; src && src[i] && out + 1 < dst_len; i++) {
        unsigned char c = (unsigned char)src[i];

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.')
            dst[out++] = (char)c;
        else
            dst[out++] = '_';
    }
    dst[out] = '\0';
}

static int set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int open_packet_socket(const char *iface, bool ignore_outgoing,
                              int *ifindex, uint8_t own_mac[ETH_ALEN])
{
    struct sockaddr_ll addr;
    struct ifreq ifr;
    int fd, one = 1;

    if (!iface || !iface[0] || strlen(iface) >= IFNAMSIZ) {
        errno = ENAMETOOLONG;
        fprintf(stderr, "invalid interface name\n");
        return -1;
    }

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) {
        perror("socket(AF_PACKET)");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        fprintf(stderr, "SIOCGIFINDEX %s: %s\n", iface, strerror(errno));
        close(fd);
        return -1;
    }
    *ifindex = ifr.ifr_ifindex;

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        fprintf(stderr, "SIOCGIFHWADDR %s: %s\n", iface, strerror(errno));
        close(fd);
        return -1;
    }
    memcpy(own_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = *ifindex;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "bind(AF_PACKET) %s: %s\n", iface, strerror(errno));
        close(fd);
        return -1;
    }

    (void)setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &(int){1024 * 1024},
                     sizeof(int));
    if (ignore_outgoing)
        (void)setsockopt(fd, SOL_PACKET, PACKET_IGNORE_OUTGOING, &one,
                         sizeof(one));
    if (set_nonblock(fd) < 0) {
        fprintf(stderr, "nonblock %s: %s\n", iface, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

static int pcap_open(struct pcap_writer *writer, const char *path)
{
    struct pcap_file_header header;
    const char *env;

    memset(writer, 0, sizeof(*writer));
    if (!path || !path[0] || !strcmp(path, "-"))
        return 0;

    writer->snaplen = RD05_DEFAULT_SNAPLEN;
    writer->max_packets = RD05_DEFAULT_MAX_PACKETS;
    env = getenv("RD05_RAW_SNAPLEN");
    if (env && env[0])
        writer->snaplen = (uint32_t)parse_ulong(env, 64, RD05_FRAME_MAX,
                                                "RD05_RAW_SNAPLEN");
    env = getenv("RD05_RAW_MAX_PACKETS");
    if (env && env[0])
        writer->max_packets = (uint32_t)parse_ulong(env, 1, 1000000,
                                                    "RD05_RAW_MAX_PACKETS");

    snprintf(writer->path, sizeof(writer->path), "%s", path);
    writer->fp = fopen(path, "wb");
    if (!writer->fp) {
        fprintf(stderr, "open PCAP %s: %s\n", path, strerror(errno));
        return -1;
    }

    memset(&header, 0, sizeof(header));
    header.magic = 0xa1b2c3d4U;
    header.major = 2;
    header.minor = 4;
    header.snaplen = writer->snaplen;
    header.network = 1; /* DLT_EN10MB */
    if (fwrite(&header, sizeof(header), 1, writer->fp) != 1) {
        fprintf(stderr, "write PCAP header %s: %s\n", path, strerror(errno));
        fclose(writer->fp);
        writer->fp = NULL;
        return -1;
    }
    return 0;
}

static void pcap_write(struct pcap_writer *writer, const void *data,
                       size_t length)
{
    struct pcap_record_header record;
    struct timespec ts;
    uint32_t included;

    if (!writer || !writer->fp)
        return;
    if (writer->packets >= writer->max_packets) {
        writer->omitted++;
        return;
    }
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        memset(&ts, 0, sizeof(ts));

    included = (uint32_t)length;
    if (included > writer->snaplen)
        included = writer->snaplen;
    memset(&record, 0, sizeof(record));
    record.ts_sec = (uint32_t)ts.tv_sec;
    record.ts_usec = (uint32_t)(ts.tv_nsec / 1000L);
    record.incl_len = included;
    record.orig_len = (uint32_t)length;
    if (fwrite(&record, sizeof(record), 1, writer->fp) != 1 ||
        fwrite(data, included, 1, writer->fp) != 1) {
        fprintf(stderr, "write PCAP %s failed: %s\n", writer->path,
                strerror(errno));
        fclose(writer->fp);
        writer->fp = NULL;
        return;
    }
    writer->packets++;
}

static void pcap_close(struct pcap_writer *writer)
{
    if (writer->fp) {
        fflush(writer->fp);
        fclose(writer->fp);
        writer->fp = NULL;
    }
}

static const uint8_t *find_bytes(const uint8_t *data, size_t length,
                                 const char *needle)
{
    size_t i, needle_len = strlen(needle);

    if (!needle_len || length < needle_len)
        return NULL;
    for (i = 0; i <= length - needle_len; i++) {
        if (!memcmp(data + i, needle, needle_len))
            return data + i;
    }
    return NULL;
}

static bool extract_field(const char *text, const char *name,
                          char *out, size_t out_len)
{
    char pattern[64];
    const char *start;
    size_t n = 0;

    if (!out_len)
        return false;
    snprintf(pattern, sizeof(pattern), "%s=", name);
    start = strstr(text, pattern);
    if (!start)
        return false;
    start += strlen(pattern);
    while (start[n] && start[n] != ' ' && start[n] != '\r' &&
           start[n] != '\n' && n + 1 < out_len) {
        out[n] = start[n];
        n++;
    }
    out[n] = '\0';
    return n > 0;
}

static size_t make_magic_frame(uint8_t *frame, size_t frame_size,
                               const uint8_t src[ETH_ALEN],
                               const uint8_t dst[ETH_ALEN],
                               const char *iface, const char *role,
                               const char *type, const char *session,
                               uint32_t sequence)
{
    char clean_session[96];
    char payload[512];
    int payload_len;
    size_t frame_len;

    if (frame_size < ETH_HLEN + 46)
        return 0;
    sanitize_token(session, clean_session, sizeof(clean_session));
    payload_len = snprintf(payload, sizeof(payload),
                           RD05_TOKEN
                           " version=1 role=%s type=%s session=%s seq=%u "
                           "mono_ns=%llu if=%s",
                           role, type, clean_session, sequence,
                           (unsigned long long)monotonic_ns(), iface);
    if (payload_len < 0)
        return 0;
    if ((size_t)payload_len >= sizeof(payload))
        payload_len = (int)sizeof(payload) - 1;

    memcpy(frame, dst, ETH_ALEN);
    memcpy(frame + ETH_ALEN, src, ETH_ALEN);
    frame[12] = (uint8_t)(ETH_P_REALTEK >> 8);
    frame[13] = (uint8_t)(ETH_P_REALTEK & 0xff);
    memcpy(frame + ETH_HLEN, payload, (size_t)payload_len);
    frame_len = ETH_HLEN + (size_t)payload_len;
    if (frame_len < ETH_ZLEN)
        memset(frame + frame_len, 0, ETH_ZLEN - frame_len), frame_len = ETH_ZLEN;
    return frame_len;
}

static int send_frame(int fd, int ifindex, const uint8_t *frame,
                      size_t frame_len, const uint8_t dst[ETH_ALEN],
                      struct pcap_writer *pcap)
{
    struct sockaddr_ll addr;
    ssize_t sent;

    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_REALTEK);
    addr.sll_ifindex = ifindex;
    addr.sll_halen = ETH_ALEN;
    memcpy(addr.sll_addr, dst, ETH_ALEN);

    sent = sendto(fd, frame, frame_len, 0, (struct sockaddr *)&addr,
                  sizeof(addr));
    if (sent < 0 || (size_t)sent != frame_len)
        return -1;
    pcap_write(pcap, frame, frame_len);
    return 0;
}

static int send_magic(int fd, int ifindex, const char *iface,
                      const uint8_t own_mac[ETH_ALEN],
                      const uint8_t dst[ETH_ALEN], const char *role,
                      const char *type, const char *session, uint32_t sequence,
                      struct pcap_writer *pcap)
{
    uint8_t frame[RD05_FRAME_MAX];
    size_t frame_len;

    frame_len = make_magic_frame(frame, sizeof(frame), own_mac, dst, iface,
                                 role, type, session, sequence);
    if (!frame_len) {
        errno = EINVAL;
        return -1;
    }
    return send_frame(fd, ifindex, frame, frame_len, dst, pcap);
}

static uint16_t ethertype_at(const uint8_t *frame, size_t length)
{
    if (length < ETH_HLEN)
        return 0;
    return ((uint16_t)frame[12] << 8) | frame[13];
}

static void count_frame(struct counters *counters, const uint8_t *frame,
                        size_t length)
{
    uint16_t proto;

    counters->rx_total++;
    counters->rx_bytes += length;
    if (length >= ETH_ALEN) {
        if (is_broadcast_mac(frame))
            counters->rx_broadcast++;
        else if (is_multicast_mac(frame))
            counters->rx_multicast++;
    }
    proto = ethertype_at(frame, length);
    switch (proto) {
    case ETH_P_ARP:
        counters->rx_arp++;
        break;
    case ETH_P_IP:
        counters->rx_ipv4++;
        break;
    case ETH_P_IPV6:
        counters->rx_ipv6++;
        break;
    case ETH_P_REALTEK:
        counters->rx_realtek++;
        break;
    default:
        counters->rx_other++;
        break;
    }
}

static void parse_magic(const uint8_t *frame, size_t length,
                        char *role, size_t role_len,
                        char *type, size_t type_len,
                        char *session, size_t session_len,
                        char *sequence, size_t sequence_len)
{
    const uint8_t *token;
    char text[768];
    size_t copy_len;

    role[0] = type[0] = session[0] = sequence[0] = '\0';
    token = find_bytes(frame, length, RD05_TOKEN);
    if (!token)
        return;
    copy_len = length - (size_t)(token - frame);
    if (copy_len >= sizeof(text))
        copy_len = sizeof(text) - 1;
    memcpy(text, token, copy_len);
    text[copy_len] = '\0';
    (void)extract_field(text, "role", role, role_len);
    (void)extract_field(text, "type", type, type_len);
    (void)extract_field(text, "session", session, session_len);
    (void)extract_field(text, "seq", sequence, sequence_len);
}

static int run_capture(const char *iface, unsigned int seconds,
                       const char *pcap_path)
{
    uint8_t own_mac[ETH_ALEN], frame[RD05_FRAME_MAX];
    struct pcap_writer pcap;
    struct counters counters;
    struct pollfd pollfd;
    struct sockaddr_ll from;
    uint64_t deadline;
    socklen_t from_len;
    char own_text[18];
    int ifindex, fd;

    memset(&counters, 0, sizeof(counters));
    fd = open_packet_socket(iface, false, &ifindex, own_mac);
    if (fd < 0)
        return 1;
    if (pcap_open(&pcap, pcap_path) < 0) {
        close(fd);
        return 1;
    }
    format_mac(own_mac, own_text);
    printf("RD05_RAW_CAPTURE_START version=1 if=%s own=%s seconds=%u pcap=%s\n",
           iface, own_text, seconds, pcap_path);
    fflush(stdout);

    pollfd.fd = fd;
    pollfd.events = POLLIN;
    deadline = monotonic_ns() + (uint64_t)seconds * 1000000000ULL;
    while (monotonic_ns() < deadline) {
        uint64_t now = monotonic_ns();
        int timeout_ms;
        int ready;

        if (now >= deadline)
            break;
        timeout_ms = (int)((deadline - now) / 1000000ULL);
        if (timeout_ms < 1)
            timeout_ms = 1;
        if (timeout_ms > 250)
            timeout_ms = 250;
        ready = poll(&pollfd, 1, timeout_ms);
        if (ready < 0) {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }
        if (ready <= 0 || !(pollfd.revents & POLLIN))
            continue;

        for (;;) {
            ssize_t received;

            memset(&from, 0, sizeof(from));
            from_len = sizeof(from);
            received = recvfrom(fd, frame, sizeof(frame), 0,
                                (struct sockaddr *)&from, &from_len);
            if (received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                if (errno == EINTR)
                    continue;
                fprintf(stderr, "recvfrom %s: %s\n", iface,
                        strerror(errno));
                break;
            }
            char role[32], type[32], session[96], seq_text[32];

            pcap_write(&pcap, frame, (size_t)received);
            count_frame(&counters, frame, (size_t)received);
            parse_magic(frame, (size_t)received, role, sizeof(role), type,
                        sizeof(type), session, sizeof(session), seq_text,
                        sizeof(seq_text));
            if (role[0] && type[0]) {
                counters.rx_magic++;
                if (!strcmp(role, "pc")) {
                    if (!strcmp(type, "probe"))
                        counters.rx_pc_probe++;
                    else if (!strcmp(type, "ack"))
                        counters.rx_pc_ack++;
                } else if (!strcmp(role, "router")) {
                    if (!strcmp(type, "probe"))
                        counters.rx_router_probe++;
                    else if (!strcmp(type, "ack"))
                        counters.rx_router_ack++;
                }
            }
        }
    }

    pcap_close(&pcap);
    printf("RD05_RAW_CAPTURE_SUMMARY version=1 if=%s own=%s rx_total=%llu "
           "rx_bytes=%llu rx_magic=%llu rx_pc_probe=%llu rx_pc_ack=%llu "
           "rx_router_probe=%llu rx_router_ack=%llu rx_arp=%llu "
           "rx_ipv4=%llu rx_ipv6=%llu rx_realtek=%llu rx_other=%llu "
           "pcap_packets=%u pcap_omitted=%u pcap=%s\n",
           iface, own_text,
           (unsigned long long)counters.rx_total,
           (unsigned long long)counters.rx_bytes,
           (unsigned long long)counters.rx_magic,
           (unsigned long long)counters.rx_pc_probe,
           (unsigned long long)counters.rx_pc_ack,
           (unsigned long long)counters.rx_router_probe,
           (unsigned long long)counters.rx_router_ack,
           (unsigned long long)counters.rx_arp,
           (unsigned long long)counters.rx_ipv4,
           (unsigned long long)counters.rx_ipv6,
           (unsigned long long)counters.rx_realtek,
           (unsigned long long)counters.rx_other,
           pcap.packets, pcap.omitted, pcap_path);
    close(fd);
    return 0;
}

static int run_send(const char *iface, const uint8_t dst[ETH_ALEN],
                    const char *role, const char *type, const char *session,
                    uint32_t first_sequence, unsigned int count,
                    unsigned int interval_ms)
{
    uint8_t own_mac[ETH_ALEN];
    char own_text[18], dst_text[18];
    int ifindex, fd;
    unsigned int i, errors = 0;

    fd = open_packet_socket(iface, true, &ifindex, own_mac);
    if (fd < 0)
        return 1;
    format_mac(own_mac, own_text);
    format_mac(dst, dst_text);
    for (i = 0; i < count; i++) {
        if (send_magic(fd, ifindex, iface, own_mac, dst, role, type,
                       session, first_sequence + i, NULL) < 0) {
            fprintf(stderr, "send %s: %s\n", iface, strerror(errno));
            errors++;
        }
        if (i + 1 < count)
            usleep(interval_ms * 1000U);
    }
    printf("RD05_RAW_SEND_SUMMARY version=1 if=%s own=%s dst=%s role=%s "
           "type=%s session=%s first_seq=%u count=%u errors=%u\n",
           iface, own_text, dst_text, role, type, session, first_sequence,
           count, errors);
    close(fd);
    return errors ? 1 : 0;
}

static int run_exchange(const char *iface, unsigned int seconds,
                        const char *router_session,
                        const uint8_t configured_peer[ETH_ALEN],
                        bool have_configured_peer, unsigned int interval_ms,
                        const char *pcap_path)
{
    static const uint8_t broadcast[ETH_ALEN] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t own_mac[ETH_ALEN], peer_mac[ETH_ALEN], frame[RD05_FRAME_MAX];
    struct pcap_writer pcap;
    struct counters counters;
    struct pollfd pollfd;
    struct sockaddr_ll from;
    uint64_t deadline, next_send;
    uint32_t sequence = 1;
    socklen_t from_len;
    char own_text[18], peer_text[18];
    int ifindex, fd;

    memset(&counters, 0, sizeof(counters));
    memset(peer_mac, 0, sizeof(peer_mac));
    if (have_configured_peer)
        memcpy(peer_mac, configured_peer, ETH_ALEN);

    fd = open_packet_socket(iface, true, &ifindex, own_mac);
    if (fd < 0)
        return 1;
    if (pcap_open(&pcap, pcap_path) < 0) {
        close(fd);
        return 1;
    }
    format_mac(own_mac, own_text);
    if (have_configured_peer)
        format_mac(peer_mac, peer_text);
    else
        snprintf(peer_text, sizeof(peer_text), "unknown");
    printf("RD05_RAW_EXCHANGE_START version=1 if=%s own=%s peer=%s "
           "session=%s seconds=%u interval_ms=%u pcap=%s\n",
           iface, own_text, peer_text, router_session, seconds, interval_ms,
           pcap_path ? pcap_path : "-");
    fflush(stdout);

    pollfd.fd = fd;
    pollfd.events = POLLIN;
    deadline = monotonic_ns() + (uint64_t)seconds * 1000000000ULL;
    next_send = monotonic_ns();

    while (monotonic_ns() < deadline) {
        uint64_t now = monotonic_ns();
        int timeout_ms;
        int ready;

        if (now >= next_send) {
            const uint8_t *dst = is_zero_mac(peer_mac) ? broadcast : peer_mac;

            if (send_magic(fd, ifindex, iface, own_mac, dst, "router",
                           "probe", router_session, sequence++, &pcap) < 0) {
                counters.tx_error++;
                fprintf(stderr, "router probe send %s: %s\n", iface,
                        strerror(errno));
            } else {
                counters.tx_probe++;
            }
            /* Keep one broadcast every tenth probe after peer learning. */
            if (!is_zero_mac(peer_mac) && (sequence % 10U) == 0U) {
                if (send_magic(fd, ifindex, iface, own_mac, broadcast,
                               "router", "probe", router_session, sequence++,
                               &pcap) < 0)
                    counters.tx_error++;
                else
                    counters.tx_probe++;
            }
            next_send = now + (uint64_t)interval_ms * 1000000ULL;
        }

        now = monotonic_ns();
        if (next_send <= now)
            timeout_ms = 1;
        else
            timeout_ms = (int)((next_send - now) / 1000000ULL);
        if (timeout_ms > 250)
            timeout_ms = 250;
        if (timeout_ms < 1)
            timeout_ms = 1;

        ready = poll(&pollfd, 1, timeout_ms);
        if (ready < 0) {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }
        if (ready <= 0 || !(pollfd.revents & POLLIN))
            continue;

        for (;;) {
            char role[32], type[32], session[96], seq_text[32];
            char src_text[18];
            ssize_t received;
            uint32_t rx_sequence = 0;

            memset(&from, 0, sizeof(from));
            from_len = sizeof(from);
            received = recvfrom(fd, frame, sizeof(frame), 0,
                                (struct sockaddr *)&from, &from_len);
            if (received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                if (errno == EINTR)
                    continue;
                fprintf(stderr, "recvfrom %s: %s\n", iface,
                        strerror(errno));
                break;
            }
            if (from.sll_pkttype == PACKET_OUTGOING)
                continue;

            pcap_write(&pcap, frame, (size_t)received);
            count_frame(&counters, frame, (size_t)received);
            parse_magic(frame, (size_t)received, role, sizeof(role), type,
                        sizeof(type), session, sizeof(session), seq_text,
                        sizeof(seq_text));
            if (!role[0] || !type[0])
                continue;

            counters.rx_magic++;
            if (seq_text[0])
                rx_sequence = (uint32_t)strtoul(seq_text, NULL, 10);
            format_mac(frame + ETH_ALEN, src_text);
            printf("RD05_RAW_RX version=1 if=%s src=%s role=%s type=%s "
                   "session=%s seq=%u len=%zd\n",
                   iface, src_text, role, type,
                   session[0] ? session : "unknown", rx_sequence, received);
            fflush(stdout);

            if (!strcmp(role, "pc")) {
                /* A received PC probe is stronger than a configured/default MAC. */
                memcpy(peer_mac, frame + ETH_ALEN, ETH_ALEN);
                if (!strcmp(type, "probe")) {
                    counters.rx_pc_probe++;
                    if (send_magic(fd, ifindex, iface, own_mac,
                                   frame + ETH_ALEN, "router", "ack",
                                   session[0] ? session : router_session,
                                   rx_sequence, &pcap) < 0) {
                        counters.tx_error++;
                    } else {
                        counters.tx_ack++;
                    }
                } else if (!strcmp(type, "ack")) {
                    counters.rx_pc_ack++;
                }
            } else if (!strcmp(role, "router")) {
                if (!strcmp(type, "probe"))
                    counters.rx_router_probe++;
                else if (!strcmp(type, "ack"))
                    counters.rx_router_ack++;
            }
        }
    }

    pcap_close(&pcap);
    if (!is_zero_mac(peer_mac))
        format_mac(peer_mac, peer_text);
    else
        snprintf(peer_text, sizeof(peer_text), "unknown");
    printf("RD05_RAW_SUMMARY version=1 if=%s own=%s peer=%s session=%s "
           "duration_s=%u interval_ms=%u tx_probe=%llu tx_ack=%llu "
           "tx_error=%llu rx_total=%llu rx_bytes=%llu rx_magic=%llu "
           "rx_pc_probe=%llu rx_pc_ack=%llu rx_router_probe=%llu "
           "rx_router_ack=%llu rx_arp=%llu rx_ipv4=%llu rx_ipv6=%llu "
           "rx_realtek=%llu rx_other=%llu pcap_packets=%u "
           "pcap_omitted=%u pcap=%s\n",
           iface, own_text, peer_text, router_session, seconds, interval_ms,
           (unsigned long long)counters.tx_probe,
           (unsigned long long)counters.tx_ack,
           (unsigned long long)counters.tx_error,
           (unsigned long long)counters.rx_total,
           (unsigned long long)counters.rx_bytes,
           (unsigned long long)counters.rx_magic,
           (unsigned long long)counters.rx_pc_probe,
           (unsigned long long)counters.rx_pc_ack,
           (unsigned long long)counters.rx_router_probe,
           (unsigned long long)counters.rx_router_ack,
           (unsigned long long)counters.rx_arp,
           (unsigned long long)counters.rx_ipv4,
           (unsigned long long)counters.rx_ipv6,
           (unsigned long long)counters.rx_realtek,
           (unsigned long long)counters.rx_other,
           pcap.packets, pcap.omitted,
           pcap_path ? pcap_path : "-");
    close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    const char *command;

    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }
    command = argv[1];

    if (!strcmp(command, "capture")) {
        unsigned int seconds;

        if (argc != 5) {
            usage(argv[0]);
            return 2;
        }
        seconds = (unsigned int)parse_ulong(argv[3], 1, 3600, "seconds");
        return run_capture(argv[2], seconds, argv[4]);
    }

    if (!strcmp(command, "exchange")) {
        uint8_t peer[ETH_ALEN];
        bool have_peer = false;
        unsigned int seconds, interval_ms = 200;
        const char *pcap_path = "-";

        if (argc < 5 || argc > 8) {
            usage(argv[0]);
            return 2;
        }
        seconds = (unsigned int)parse_ulong(argv[3], 1, 3600, "seconds");
        if (argc >= 6 && strcmp(argv[5], "-")) {
            have_peer = parse_mac(argv[5], peer);
            if (!have_peer) {
                fprintf(stderr, "invalid peer MAC: %s\n", argv[5]);
                return 2;
            }
        }
        if (argc >= 7)
            interval_ms = (unsigned int)parse_ulong(argv[6], 20, 60000,
                                                    "interval_ms");
        if (argc >= 8)
            pcap_path = argv[7];
        return run_exchange(argv[2], seconds, argv[4], peer, have_peer,
                            interval_ms, pcap_path);
    }

    if (!strcmp(command, "send")) {
        uint8_t dst[ETH_ALEN];
        uint32_t sequence;
        unsigned int count = 1, interval_ms = 200;

        if (argc < 8 || argc > 10) {
            usage(argv[0]);
            return 2;
        }
        if (!parse_mac(argv[3], dst)) {
            fprintf(stderr, "invalid destination MAC: %s\n", argv[3]);
            return 2;
        }
        sequence = (uint32_t)parse_ulong(argv[7], 0, 0xffffffffUL,
                                         "sequence");
        if (argc >= 9)
            count = (unsigned int)parse_ulong(argv[8], 1, 100000, "count");
        if (argc >= 10)
            interval_ms = (unsigned int)parse_ulong(argv[9], 20, 60000,
                                                    "interval_ms");
        return run_send(argv[2], dst, argv[4], argv[5], argv[6], sequence,
                        count, interval_ms);
    }

    usage(argv[0]);
    return 2;
}
