/* Reconstructed from Cudy_AX1500.bin SHA256
 * f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188.
 * This is newly written C describing verified instruction semantics.
 * It is NOT recovered original C and NOT a complete kernel driver.
 * No MMIO, DMA, firmware execution, radio enabling or host code loading here.
 */
#include "cudy_wlan_recovered.h"
static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | (uint16_t)p[1] << 8);
}
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void wr16(uint8_t *p, uint16_t v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); }
static void wr32(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8);
    p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
/* rtk_wifi6.ko .text+0x28ce4c, 104 bytes. */
size_t cudy_8832br_txdesc_length(const struct cudy_8832br_tx_meta *m) {
    if (m->packet_type == 2 || m->packet_type == 4) {
        if (m->adapter_hw_word_0x08 != 0) return 16;
        return (((uint32_t)m->payload_length + 16u) & 0x1ffu) ? 16u : 32u;
    }
    return (m->metadata_byte_0x21 & 0x20u) ? 56u : 32u;
}
/* rtk_wifi6.ko .text+0x28cff8, 260 bytes. OEM logging omitted;
 * error 5 for NULL retained. Added bounds error -1 for safe offline use. */
int cudy_8832br_wd_checksum(uint8_t *d, size_t capacity,
                          const struct cudy_8832br_tx_meta *m) {
    uint16_t checksum=0;
    if (!d) return 5;
    const size_t length=cudy_8832br_txdesc_length(m);
    if (capacity < length || capacity < 20) return -1;
    const uint32_t upper=rd32(d+16) & 0xffff0000u;
    wr32(d+16,upper);
    for (size_t i=0;i<length;i+=4)
        checksum=(uint16_t)(checksum ^ rd16(d+i) ^ rd16(d+i+2));
    wr32(d+16,upper|checksum);
    return 0;
}
/* rtk_wifi6.ko hal_write32_mask .text+0x1eefb0, 176 bytes.
 * Mask 0 yields no changed bits; no shift by 32 is performed in C. */
uint32_t cudy_wlan_mask_merge(uint32_t old, uint32_t mask, uint32_t field) {
    if (mask == 0xffffffffu) return field;
    if (mask == 0) return old;
    unsigned shift=0;
    while (((mask>>shift)&1u)==0) ++shift;
    return old ^ (((field<<shift)^old)&mask);
}
/* rtl8192cd.ko QueryRxDesc88XX_V1 .text+0x158efc..0x159144.
 * This function reconstructs its field-extraction suffix, NOT the preceding
 * DMA ring synchronisation, expected-tag polling or state update. Field
 * semantics absent from the binary are represented as OEM byte offsets. */
void cudy_8197g_decode_rx_fields(uint32_t bd, const uint8_t d[16],
                               int opt_a90, int opt_a91, uint8_t o[26]) {
    o[0]=(uint8_t)((bd>>15)&1u);
    o[1]=(uint8_t)((bd>>14)&1u);
    wr16(o+2,(uint16_t)(bd&0x3fffu));
    if (!o[0]) return;
    const uint32_t dw0=rd32(d), dw1=rd32(d+4), dw2=rd32(d+8), dw3=rd32(d+12);
    wr16(o+4,(uint16_t)(dw0&0x3fffu));
    o[6]=(uint8_t)((dw0>>14)&1u);
    o[7]=(uint8_t)((dw0>>15)&1u);
    o[8]=(uint8_t)(((dw0>>16)&0x0fu)<<3);
    o[10]=(uint8_t)((dw0>>24)&3u);
    o[11]=(uint8_t)((dw0>>26)&1u);
    o[12]=(uint8_t)((dw0>>27)&1u);
    if (opt_a90) o[15]=(uint8_t)((dw1>>15)&1u);
    o[17]=(uint8_t)((dw2>>28)&1u);
    o[25]=(uint8_t)(dw3&0x7fu);
    if (opt_a91) o[18]=(uint8_t)(dw2>>31);
}
/* rtl8192cd.ko HalPwrSeqCmdParsing88XX .text+0x158480, 652 bytes.
 * 8-byte commands; filtering, read/modify/write, shared polling budget and
 * delay scaling recovered from machine instructions. Logging-free callback
 * form permits offline verification without accessing actual radio hardware. */
int cudy_8197g_run_power_sequence(const uint8_t *table, size_t size,
                                uint8_t cut, uint8_t fab, uint8_t iface,
                                const struct cudy_power_callbacks *io) {
    unsigned polls=0;
    if (!table || !io || !io->read8 || !io->write8 || !io->delay_us) return -1;
    for (size_t pos=0; pos+8<=size; pos+=8) {
        const uint8_t *s=table+pos;
        if (!(s[2]&cut) || !((s[3]&15u)&fab) || !((s[3]>>4)&iface)) continue;
        const uint16_t offset=rd16(s);
        const uint8_t cmd=(uint8_t)(s[4]>>4), mask=s[5], value=s[6];
        switch (cmd) {
        case 1: {
            uint8_t old=io->read8(io->context,offset);
            io->write8(io->context,offset,(uint8_t)((old&~mask)|(value&mask)));
            break;
        }
        case 2:
            for (;;) {
                uint8_t actual=(uint8_t)(io->read8(io->context,offset)&mask);
                if (actual==(uint8_t)(value&mask)) {
                    if (polls++ >= 5001u) return 0;
                    break;
                }
                io->delay_us(io->context,10);
                if (polls++ >= 5001u) return 0;
            }
            break;
        case 3:
            io->delay_us(io->context,(uint32_t)offset*(value?1000u:1u));
            break;
        case 4:
            return 1;
        default:
            break;
        }
    }
    return -1;
}
