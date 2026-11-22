/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Cudy R76 OEM descriptor reference, reconstructed from the supplied firmware.
 * This is a host-testable format/algorithm reference, NOT a Linux netdev driver.
 * Instruction addresses, provenance and limitations: ETHERNET_ANALYSE.md.
 */
#ifndef R76_DESC_REFERENCE_H
#define R76_DESC_REFERENCE_H
#include <stdint.h>

#define R76_DESCRIPTOR_BYTES 32u
#define R76_OWN (1u << 0)
#define R76_EOR (1u << 1)
#define R76_LAST (1u << 2)
#define R76_FIRST (1u << 3)
#define R76_CPUICR 0xb8010000u
#define R76_CPURPDCR0 0xb8010004u
#define R76_CPUIIMR 0xb8010028u
#define R76_CPUIISR 0xb801002cu
#define R76_DMA_CR1 0xb8010040u
#define R76_DMA_CR4 0xb80100a0u
#define R76_CPUICR1 0xb80100a4u
#define R76_TXFD (1u << 23)

extern const uint32_t r76_tx_cdp_registers[4];
extern const uint32_t r76_oem_tx_ring_counts[4];
extern const uint32_t r76_oem_rx_ring_counts[6];

struct r76_dma_desc { uint32_t dw[8]; };
/* Fields are host integers. The serializer explicitly writes little endian. */
struct r76_tx_input {
    uint32_t dma_address;
    uint32_t payload_length;
    uint32_t vlan_id;
    uint32_t port_mask;
    uint32_t nic_flags;                 /* 0x20 HWLOOKUP, 0x40 BRIDGING */
    uint32_t ethernet_type;             /* host notation, e.g. 0x8100 */
    uint32_t vlan_tag_mask;             /* resolved OEM VLAN helper result */
    uint32_t vlan_priority;             /* resolved OEM priority helper result */
    uint32_t passthrough_enabled;
    uint32_t skb_passthrough_flag;      /* OEM skb byte +0xb8 bit1 */
    uint32_t priority_override_enabled; /* global at OEM address 0x806c6814 */
    uint32_t skb_vlan_priority;         /* OEM skb word +0xac bits28..30 */
};

struct r76_rx_metadata {
    uint32_t dma_address, wire_length, host_length;
    uint32_t source_port, destination_vlan, priority, source_extension;
    uint32_t destination_extension_mask, l2_action, l3_checksum_ok, l4_checksum_ok;
};

uint32_t r76_oem_wire_length(uint32_t payload_length);
void r76_oem_build_tx(struct r76_dma_desc *desc, const struct r76_tx_input *input);
void r76_desc_store_le(uint8_t out[R76_DESCRIPTOR_BYTES], const struct r76_dma_desc *desc);
void r76_desc_load_le(struct r76_dma_desc *desc, const uint8_t in[R76_DESCRIPTOR_BYTES]);
uint32_t r76_oem_rx_init_word(uint32_t cluster_size, int last_descriptor);
uint32_t r76_oem_rearm_word(uint32_t previous_word);
void r76_oem_decode_rx(const struct r76_dma_desc *desc, struct r76_rx_metadata *out);
int r76_oem_rx_preprocess(const struct r76_dma_desc *desc, int extension_ports_enabled,
                          uint16_t *vlan_id);
uint32_t r76_oem_next_index(uint32_t index, uint32_t ring_count);
int r76_oem_tx_ring_full(uint32_t producer, uint32_t consumer, uint32_t ring_count);
uint32_t r76_oem_cdp_index(uint32_t cdp, uint32_t same_alias_ring_base);
int r76_oem_rx_not_empty(uint32_t software_index, uint32_t hardware_index,
                          uint32_t current_descriptor_word0);

/* New validation helper, explicitly NOT part of the OEM binary translation.
 * Accepts physical, KSEG0 or KSEG1 aliases; rejects unaligned/out-of-ring CDPs.
 * Does not establish DMA completion or memory ordering on its own. */
int r76_checked_cdp_index(uint32_t cdp, uint32_t ring_base, uint32_t ring_count,
                           uint32_t *index);
#endif
