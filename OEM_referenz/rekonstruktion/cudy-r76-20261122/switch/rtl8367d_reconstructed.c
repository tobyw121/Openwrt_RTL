/*
 * Selective C reconstruction of register operations in Cudy R76 kernel
 * SHA256 7d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e.
 * Evidence: register_disassembly.txt, tag_init_disassembly.txt,
 * mac_vlan_disassembly.txt and evidence/verified_functions.json.
 *
 * This readable API is newly written; it is not recovered original C.
 * Successful register operations and ordering follow the inspected binary.
 * The callback boundary, physical-port parameters and local error constants
 * are adaptations. Original source/API remains in the OEM reference tree.
 * The original VLAN routine permits enum sentinel2 in two tests; this API
 * deliberately accepts only boolean0/1 for its two one-bit VLAN fields.
 * Callers implement locking, initialization, logical-port mapping and bus.
 */
#include "rtl8367d_reconstructed.h"
#include "rtl8367d_register_contract.h"

static int bus_valid(const struct r76_d_bus *bus)
{ return bus && bus->read && bus->write; }
static int physical_valid(unsigned port)
{ return port <= 4 || port == 6 || port == 7; }
static int write_reg(const struct r76_d_bus *bus, uint16_t reg, uint16_t value)
{ return bus->write(bus->context, reg, value) ? R76_D_BUS_ERROR : R76_D_OK; }

/* rtl8367d_setAsicRegBits, 0x802acd38: align value to first set mask bit. */
int r76_d_reg_update(const struct r76_d_bus *bus, uint16_t reg,
                     uint16_t mask, unsigned value)
{
    unsigned shift = 0;
    uint16_t old;
    if (!bus_valid(bus) || !mask) return R76_D_ARGUMENT;
    while (!(mask & (1u << shift))) ++shift;
    if (value > (0xffffu >> shift)) return R76_D_ARGUMENT;
    if (bus->read(bus->context, reg, &old)) return R76_D_BUS_ERROR;
    return write_reg(bus, reg, (uint16_t)((old & ~mask) | ((value << shift) & mask)));
}

/* dal_rtl8367d_port_rgmiiDelayExt_set, 0x802b23a0. */
int r76_d_ext1_delay(const struct r76_d_bus *bus, unsigned tx, unsigned rx)
{
    if (tx > 1 || rx > 7) return R76_D_ARGUMENT;
    return r76_d_reg_update(bus, 0x1307, 0x000f, (tx << 3) | rx);
}

/* dal_rtl8367d_port_macForceLinkExtSSC_set, 0x802b599c. */
int r76_d_ext1_ssc(const struct r76_d_bus *bus, unsigned enabled)
{
    const struct r76_rtl8367d_pair *seq;
    unsigned i;
    int result;
    if (!bus_valid(bus) || enabled > 1) return R76_D_ARGUMENT;
    seq = enabled ? r76_rtl8367d_ssc_on : r76_rtl8367d_ssc_off;
    for (i = 0; i < 4; ++i) {
        result = write_reg(bus, seq[i].reg, seq[i].value);
        if (result) return result;
    }
    return R76_D_OK;
}

/* dal_rtl8367d_vlan_portPvid_set, 0x802c289c, priority fixed0. */
int r76_d_pvid_phys(const struct r76_d_bus *bus, unsigned port, unsigned vid)
{
    if (!physical_valid(port) || vid > 4095) return R76_D_ARGUMENT;
    return r76_d_reg_update(bus, (uint16_t)(0x0700 + port), 0x0fff, vid);
}

/* _dal_rtl8367d_setAsicVlan4kEntry, 0x802c23d4. */
int r76_d_vlan_write(const struct r76_d_bus *bus, const struct r76_d_vlan *vlan)
{
    int result;
    if (!bus_valid(bus) || !vlan || vlan->vid > 4095 ||
        vlan->member > 255 || vlan->untag > 255 || vlan->fid > 3 ||
        vlan->svlan_ivl > 1 || vlan->ivl > 1) return R76_D_ARGUMENT;
    result = write_reg(bus, 0x0510, r76_rtl8367d_vlan_word0(
                       (uint8_t)vlan->member, (uint8_t)vlan->untag));
    if (result) return result;
    result = write_reg(bus, 0x0511, r76_rtl8367d_vlan_word1(
                       vlan->fid, vlan->svlan_ivl, vlan->ivl));
    if (result) return result;
    result = write_reg(bus, 0x0501, (uint16_t)vlan->vid);
    if (result) return result;
    return r76_d_reg_update(bus, 0x0500, 0x000f, 0x000b);
}

/* dal_rtl8367d_cpu_tagPort_set, 0x802acfd4; modes ALL0, TRAP1, NONE2. */
int r76_d_cpu_port(const struct r76_d_bus *bus, unsigned port, unsigned mode)
{
    int result;
    if (!bus_valid(bus) || !physical_valid(port) || mode > 2)
        return R76_D_ARGUMENT;
    result = write_reg(bus, 0x1219, (uint16_t)(1u << port));
    if (result) return result;
    result = r76_d_reg_update(bus, 0x121a, 0x0038, port);
    if (result) return result;
    return r76_d_reg_update(bus, 0x121a, 0x0006, mode);
}

/* dal_rtl8367d_cpu_tagLength_set, 0x802ad2c0, enum1 branch. */
int r76_d_cpu_taglen4(const struct r76_d_bus *bus)
{
    int result = r76_d_reg_update(bus, 0x121a, 0x0200, 1);
    if (result) return result;
    return r76_d_reg_update(bus, 0x121a, 0x0400, 0);
}

/* dal_rtl8367d_cpu_acceptLength_set, 0x802ad530. */
int r76_d_cpu_accept_length(const struct r76_d_bus *bus, unsigned enabled)
{
    if (enabled > 1) return R76_D_ARGUMENT;
    return r76_d_reg_update(bus, 0x121a, 0x0080, enabled);
}

/* dal_rtl8367d_cpu_enable_set, 0x802acf10: bit0; disabling clears CPU mask. */
int r76_d_cpu_enable(const struct r76_d_bus *bus, unsigned enabled)
{
    int result;
    if (enabled > 1) return R76_D_ARGUMENT;
    result = r76_d_reg_update(bus, 0x121a, 0x0001, enabled);
    if (result || enabled) return result;
    return write_reg(bus, 0x1219, 0);
}

/* dal_rtl8367d_port_macForceLink_set, 0x802b1818, fixed board CPU ability. */
int r76_d_cpu_force_1g(const struct r76_d_bus *bus)
{
    int result;
    if (!bus_valid(bus)) return R76_D_ARGUMENT;
    result = write_reg(bus, 0x12c7, 0x0076);
    if (result) return result;
    return write_reg(bus, 0x12cf, 0xffff);
}
