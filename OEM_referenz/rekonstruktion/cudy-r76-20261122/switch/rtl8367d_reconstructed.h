/* Selective register-level reconstruction, not a complete switch driver.
 * See rtl8367d_reconstructed.c for provenance and deliberately adapted API. */
#ifndef R76_RTL8367D_RECONSTRUCTED_H
#define R76_RTL8367D_RECONSTRUCTED_H
#include <stdint.h>

struct r76_d_bus {
    void *context;
    int (*read)(void *context, uint16_t reg, uint16_t *value);
    int (*write)(void *context, uint16_t reg, uint16_t value);
};
enum r76_d_result { R76_D_OK=0, R76_D_ARGUMENT=-1, R76_D_BUS_ERROR=-2 };
struct r76_d_vlan {
    unsigned vid, member, untag, fid, svlan_ivl, ivl;
};

/* Callers supply a usable, initialized bus and serialization covering each
 * complete helper, including read/modify/write and table writes. No GPIO,
 * IRQ, delay, reset, PHY initialization or Linux network integration is here.
 * Ports below are physical. The OEM logical EXT1 -> physical7 mapping is
 * already resolved by these API boundaries. Errors use the enum above,
 * not OEM RT_ERR values. No helper is a complete board-init sequence. */
int r76_d_reg_update(const struct r76_d_bus *, uint16_t reg,
                     uint16_t mask, unsigned unshifted_value);
int r76_d_ext1_delay(const struct r76_d_bus *, unsigned tx, unsigned rx);
int r76_d_ext1_ssc(const struct r76_d_bus *, unsigned enabled);
int r76_d_pvid_phys(const struct r76_d_bus *, unsigned port, unsigned vid);
int r76_d_vlan_write(const struct r76_d_bus *, const struct r76_d_vlan *);
int r76_d_cpu_port(const struct r76_d_bus *, unsigned port, unsigned insert_mode);
int r76_d_cpu_taglen4(const struct r76_d_bus *);
int r76_d_cpu_accept_length(const struct r76_d_bus *, unsigned enabled);
int r76_d_cpu_enable(const struct r76_d_bus *, unsigned enabled);
int r76_d_cpu_force_1g(const struct r76_d_bus *);
#endif
