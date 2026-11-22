#ifndef CUDY_WLAN_RECOVERED_H
#define CUDY_WLAN_RECOVERED_H
#include <stddef.h>
#include <stdint.h>
/* Provenance and scope: see WLAN_ANALYSE.md. These are recovered pure logic
 * and callback-driven interpreters, not loadable Linux WLAN drivers. */
struct cudy_8832br_tx_meta {
    uint32_t packet_type;             /* OEM metadata +0x00 */
    uint16_t payload_length;          /* OEM metadata +0x08 */
    uint8_t metadata_byte_0x21;
    uint32_t adapter_hw_word_0x08;     /* OEM *(adapter+0x48)+0x08 */
};
size_t cudy_8832br_txdesc_length(const struct cudy_8832br_tx_meta *meta);
/* A safe wrapper around the exact XOR core. descriptor_capacity must cover
 * both the computed descriptor and the checksum field at offsets 16..19. */
int cudy_8832br_wd_checksum(uint8_t *descriptor, size_t descriptor_capacity,
                          const struct cudy_8832br_tx_meta *meta);
uint32_t cudy_wlan_mask_merge(uint32_t old_value, uint32_t mask, uint32_t field);
/* Updates ONLY bytes written by QueryRxDesc88XX_V1 in the OEM. The remaining
 * bytes of the unknown OEM output structure are deliberately retained. */
void cudy_8197g_decode_rx_fields(uint32_t rxbd_word0, const uint8_t descriptor[16],
                               int option_0xa90, int option_0xa91,
                               uint8_t output[26]);
struct cudy_power_callbacks {
    void *context;
    uint8_t (*read8)(void *context, uint16_t offset);
    void (*write8)(void *context, uint16_t offset, uint8_t value);
    void (*delay_us)(void *context, uint32_t microseconds);
};
/* Reconstructed HalPwrSeqCmdParsing88XX core. Callback layer must enforce
 * OEM's platform clock-access gate when appropriate. The added table bound
 * prevents out-of-bounds access if an END entry is absent.
 * Returns: 1 END reached, 0 OEM polling timeout, -1 invalid/missing END. */
int cudy_8197g_run_power_sequence(const uint8_t *table, size_t table_size,
                                uint8_t cut_mask, uint8_t fab_mask,
                                uint8_t interface_mask,
                                const struct cudy_power_callbacks *io);
#endif
