/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Reconstruction from kernel SHA256
 * 7d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e.
 * Names/field meanings cross-checked with Realtek GPL rtl819x_swNic.[ch].
 * No Linux ABI, DMA allocator, MMIO access or device-registration implementation.
 */
#include "r76_desc_reference.h"
#include <stddef.h>
#include <string.h>

_Static_assert(sizeof(struct r76_dma_desc) == R76_DESCRIPTOR_BYTES, "descriptor stride");
const uint32_t r76_tx_cdp_registers[4] = {
    0xb8010020u, 0xb8010024u, 0xb8010060u, 0xb8010064u
};
const uint32_t r76_oem_tx_ring_counts[4] = { 768, 2, 2, 2 };
const uint32_t r76_oem_rx_ring_counts[6] = { 900, 2, 2, 2, 2, 2 };

uint32_t r76_oem_wire_length(uint32_t length)
{
    /* 0x8052a7c4..0x8052a7ec: uint32 add, then 17-bit field truncation. */
    return ((length < 60u) ? 64u : length + 4u) & 0x1ffffu;
}

void r76_oem_build_tx(struct r76_dma_desc *d, const struct r76_tx_input *p)
{
    /* Data/metadata construction of _New_swNic_send, 0x8052a708.
     * The two VLAN callbacks are represented by their resolved return values.
     * OWN is left zero here. Real OEM publication: cache writeback 32 bytes,
     * sync, uncached word0 |= OWN, producer update, CPUICR |= TXFD.
     * A host memory fence is not a replacement for this MIPS DMA sequence.
     */
    uint32_t wire_len = r76_oem_wire_length(p->payload_length);
    uint32_t tag = p->vlan_tag_mask & 0x3fu;
    uint32_t pri = p->vlan_priority & 7u;
    uint32_t dp = p->port_mask & 0x7fu;
    int tagged = p->ethernet_type == 0x8100u;
    memset(d, 0, sizeof(*d));
    d->dw[0] = R76_FIRST | R76_LAST | (wire_len << 6);
    d->dw[1] = p->dma_address;
    d->dw[2] = wire_len << 15;
    d->dw[3] = p->vlan_id & 0xfffu;
    d->dw[4] = dp << 24;
    if (p->nic_flags & 0x20u) {
        d->dw[0] |= 1u << 4;
        if (p->nic_flags & 0x40u)
            d->dw[0] |= 1u << 5;
        d->dw[5] = 3u << 30; /* Actual compiled constant; preserve it. */
    }
    if (tagged) {
        d->dw[0] |= 1u << 28;
        if (!(p->passthrough_enabled && p->skb_passthrough_flag))
            tag = 0;
    }
    d->dw[2] |= tag;
    if (p->priority_override_enabled && dp == 4u && (tag & 4u))
        pri = p->skb_vlan_priority & 7u;
    d->dw[3] |= pri << 22;
}

void r76_desc_store_le(uint8_t out[32], const struct r76_dma_desc *d)
{
    for (size_t w = 0; w < 8; ++w)
        for (size_t byte = 0; byte < 4; ++byte)
            out[w * 4 + byte] = (uint8_t)(d->dw[w] >> (byte * 8));
}

void r76_desc_load_le(struct r76_dma_desc *d, const uint8_t in[32])
{
    for (size_t w = 0; w < 8; ++w) {
        d->dw[w] = 0;
        for (size_t byte = 0; byte < 4; ++byte)
            d->dw[w] |= (uint32_t)in[w * 4 + byte] << (byte * 8);
    }
}

uint32_t r76_oem_rx_init_word(uint32_t cluster_size, int last)
{
    /* 0x802626ec,0x80262708,0x8026272c,0x802627a8..0x80262818. */
    return (cluster_size << 16) | R76_OWN | (last ? R76_EOR : 0u);
}

uint32_t r76_oem_rearm_word(uint32_t previous_word)
{
    /* 0x8052a5f4..0x8052a604. OEM preserves other bits, including FS/LS. */
    return previous_word | R76_OWN;
}

void r76_oem_decode_rx(const struct r76_dma_desc *d, struct r76_rx_metadata *o)
{
    /* Loads/extracts 0x8052a504..0x8052a55c, checksum check 0x8052a420. */
    o->dma_address = d->dw[1];
    o->wire_length = d->dw[2] & 0x3fffu;
    o->host_length = (uint16_t)(o->wire_length - 4u); /* OEM truncation. */
    o->source_port = (d->dw[4] >> 13) & 7u;
    o->destination_vlan = (d->dw[4] >> 16) & 0xfffu;
    o->priority = (d->dw[5] >> 19) & 7u;
    o->source_extension = (d->dw[2] >> 24) & 3u;
    o->destination_extension_mask = (d->dw[2] >> 20) & 15u;
    o->l2_action = (d->dw[4] >> 28) & 1u;
    o->l3_checksum_ok = (d->dw[5] >> 31) & 1u;
    o->l4_checksum_ok = (d->dw[5] >> 30) & 1u;
}

int r76_oem_rx_preprocess(const struct r76_dma_desc *d, int enabled, uint16_t *vid)
{
    /* Complete leaf-function translation of 0x8052a234..0x8052a2e8. */
    static const uint8_t ext_to_port[8] = {5,6,7,5,8,5,5,5}; /* 0x8054d940 */
    uint32_t src = (d->dw[4] >> 13) & 7u;
    uint32_t ext = (d->dw[2] >> 20) & 15u;
    *vid = (uint16_t)((d->dw[4] >> 16) & 0xfffu);
    if (!enabled)
        return 0;
    if (src == 7u) {
        if (!(ext & 8u))
            return -1;
        *vid = 0xa533u;
        return 0;
    }
    if (ext & 8u)
        return 0;
    if (!ext || ext_to_port[ext] == 5u)
        return -1;
    if (d->dw[4] & (1u << 28))
        *vid = 0xa530u;
    return 0;
}

uint32_t r76_oem_next_index(uint32_t index, uint32_t count)
{
    ++index;
    return index == count ? 0u : index;
}

int r76_oem_tx_ring_full(uint32_t producer, uint32_t consumer, uint32_t count)
{
    return r76_oem_next_index(producer, count) == consumer;
}

uint32_t r76_oem_cdp_index(uint32_t cdp, uint32_t base)
{
    /* Literal calculation 0x8052aac8..0x8052aad0; both OEM operands KSEG1. */
    return (cdp - base) >> 5;
}

int r76_oem_rx_not_empty(uint32_t software, uint32_t hardware, uint32_t word0)
{
    /* Complete decision in 0x8052a2ec..0x8052a364. */
    return software != hardware || !(word0 & R76_OWN);
}

int r76_checked_cdp_index(uint32_t cdp, uint32_t base, uint32_t count, uint32_t *index)
{
    uint32_t physical_cdp = cdp & 0x1fffffffu;
    uint32_t physical_base = base & 0x1fffffffu;
    uint32_t delta;
    if (!count || count > 0x1000000u || physical_cdp < physical_base)
        return -1;
    delta = physical_cdp - physical_base;
    if ((delta & 31u) || delta / R76_DESCRIPTOR_BYTES >= count)
        return -1;
    *index = delta / R76_DESCRIPTOR_BYTES;
    return 0;
}
