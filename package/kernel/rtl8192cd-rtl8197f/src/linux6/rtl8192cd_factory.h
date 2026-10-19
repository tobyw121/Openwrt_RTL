/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RTL8192CD_FACTORY_H
#define RTL8192CD_FACTORY_H

/* Included after Dot11RFEntry and the standard integer/string definitions.
 * These helpers never write flash, OTP, eFuse, registers or unrelated MIBs.
 */
#include "rtl8192cd_factory_format.h"

struct rtl_factory_stats {
	u32 applied;
	u32 rejected;
};

struct rtl_factory_field {
	const char *key;
	u16 offset;
	u16 size;
	u16 rd05_offset;
	u8 band; /* 0: common, 2: 2.4 GHz, 5: 5 GHz */
	u8 path; /* number of RF paths required */
	u8 scalar; /* 0: byte array, 1: u32, 2: u8 */
	u8 maximum;
};

#define RF_FIELD(key, member, bin, band, path, kind, max) \
	{ key, offsetof(struct Dot11RFEntry, member), \
	  sizeof(((struct Dot11RFEntry *)0)->member), bin, band, path, kind, max }

/* RD05 offsets recovered from its own libapmib.so hwmib_wlan_table, not
 * from a different SDK's packed C struct.  Only OEM-script RF fields below.
 */
static const struct rtl_factory_field rtl_factory_fields[] = {
	RF_FIELD("11N_XCAP", xcap, 0x09d, 0, 1, 1, 63),
	RF_FIELD("11N_THER", ther, 0x0a0, 0, 1, 1, 254),
	RF_FIELD("11N_THER", thermal[0], 0x0a0, 0, 1, 1, 254),
	RF_FIELD("11N_THER_2", thermal[1], 0x0a6, 0, 2, 1, 254),
	RF_FIELD("11N_THER_3", thermal[2], 0x0a7, 0, 3, 1, 254),
	RF_FIELD("11N_THER_4", thermal[3], 0x0a8, 0, 4, 1, 254),
	RF_FIELD("11N_TSSI1", tssi1, 0x09e, 0, 1, 1, 254),
	RF_FIELD("11N_TSSI2", tssi2, 0x09f, 0, 2, 1, 254),
	RF_FIELD("TX_POWER_CCK_A", pwrlevelCCK_A, 0x030, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_CCK_B", pwrlevelCCK_B, 0x03e, 2, 2, 0, 0),
	RF_FIELD("TX_POWER_HT40_1S_A", pwrlevelHT40_1S_A, 0x04c, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_HT40_1S_B", pwrlevelHT40_1S_B, 0x05a, 2, 2, 0, 0),
	RF_FIELD("TX_POWER_DIFF_HT40_2S", pwrdiffHT40_2S, 0x068, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_DIFF_HT20", pwrdiffHT20, 0x076, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_DIFF_OFDM", pwrdiffOFDM, 0x084, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_5G_HT40_1S_A", pwrlevel5GHT40_1S_A, 0x0ad, 5, 1, 0, 0),
	RF_FIELD("TX_POWER_5G_HT40_1S_B", pwrlevel5GHT40_1S_B, 0x171, 5, 2, 0, 0),
	RF_FIELD("TX_POWER_DIFF_5G_HT40_2S", pwrdiff5GHT40_2S, 0x235, 5, 1, 0, 0),
	RF_FIELD("TX_POWER_DIFF_5G_HT20", pwrdiff5GHT20, 0x2f9, 5, 1, 0, 0),
	RF_FIELD("TX_POWER_DIFF_5G_OFDM", pwrdiff5GOFDM, 0x3bd, 5, 1, 0, 0),
	RF_FIELD("TX_POWER_TSSI_CCK_A", pwrlevel_TSSICCK_A, 0x481, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_TSSI_CCK_B", pwrlevel_TSSICCK_B, 0x48f, 2, 2, 0, 0),
	RF_FIELD("TX_POWER_TSSI_HT40_1S_A", pwrlevel_TSSIHT40_1S_A, 0x4b9, 2, 1, 0, 0),
	RF_FIELD("TX_POWER_TSSI_HT40_1S_B", pwrlevel_TSSIHT40_1S_B, 0x4c7, 2, 2, 0, 0),
	RF_FIELD("TX_POWER_TSSI_5G_HT40_1S_A", pwrlevel_TSSI5GHT40_1S_A, 0x4f1, 5, 1, 0, 0),
	RF_FIELD("TX_POWER_TSSI_5G_HT40_1S_B", pwrlevel_TSSI5GHT40_1S_B, 0x5b5, 5, 2, 0, 0),
	/* RD05 has two paths.  C/D offsets are unused for its layout. */
	RF_FIELD("TX_POWER_TSSI_5G_HT40_1S_C", pwrlevel_TSSI5GHT40_1S_C, 0x679, 5, 3, 0, 0),
	RF_FIELD("TX_POWER_TSSI_5G_HT40_1S_D", pwrlevel_TSSI5GHT40_1S_D, 0x73d, 5, 4, 0, 0),
	/* Enable is last: never enable TSSI from a partial factory table set. */
	RF_FIELD("11N_TSSI_ENABLE", tssi_enable, 0x801, 0, 1, 2, 1),
};
#undef RF_FIELD

/* A record must begin at a delimiter; substrings inside values are not keys.
 * Reject duplicates and unterminated values rather than guessing a winner.
 */
static int rtl_factory_kv(const u8 *data, size_t len, const char *key,
			  const u8 **value, size_t *value_len)
{
	size_t i, end, n = strlen(key);
	int found = 0;

	for (i = 0; i + n + 1 < len; i++) {
		if (i && data[i - 1] != '\n' && data[i - 1] != '\r' &&
		    data[i - 1] != '\0')
			continue;
		if (memcmp(data + i, key, n) || data[i + n] != '=')
			continue;
		if (found)
			return -EINVAL;
		for (end = i + n + 1; end < len; end++)
			if (!data[end] || data[end] == '\n' || data[end] == '\r' ||
			    data[end] == 0xff)
				break;
		if (end == len || end == i + n + 1)
			return -EINVAL;
		*value = data + i + n + 1;
		*value_len = end - (i + n + 1);
		found = 1;
		i = end - 1;
	}
	return found;
}

static int rtl_factory_nibble(u8 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -EINVAL;
}

static int rtl_factory_scalar(const u8 *value, size_t len, u8 maximum, u32 *out)
{
	u32 n = 0;
	size_t i;

	if (!len || len > 3)
		return -EINVAL;
	/* Tenda stores scalar BYTE_T fields as decimal, arrays as hex. */
	for (i = 0; i < len; i++) {
		if (value[i] < '0' || value[i] > '9')
			return -EINVAL;
		n = n * 10 + value[i] - '0';
	}
	if (n > maximum)
		return -ERANGE;
	*out = n;
	return 0;
}

static int rtl_factory_h601(const u8 *data, size_t len)
{
	u8 sum = 0;
	size_t i;

	/* RD05's own library: HW_SETTING=6047, WLAN_ROOT=0x409,
	 * two packed radio records of 2507 bytes.  H601 header is six bytes,
	 * followed by that payload and an 8-bit additive checksum.
	 * Do not accept other boards' H601 layouts just because magic matches.
	 */
	if (len < 6 + 6048 || memcmp(data, "H601", 4) ||
	    data[4] != 0x17 || data[5] != 0xa0)
		return -EINVAL;
	for (i = 6; i < 6 + 6048; i++)
		sum += data[i];
	return sum ? -EBADMSG : 0;
}

static int rtl_factory_apply(struct Dot11RFEntry *rf, unsigned int format,
			     unsigned int radio, unsigned int paths, bool band5,
			     const u8 *data, size_t len,
			     struct rtl_factory_stats *stats)
{
	u8 decoded[MAX_5G_CHANNEL_NUM];
	unsigned int i, j, tssi_tables = 0;
	char key[80];

	memset(stats, 0, sizeof(*stats));
	if (!data || !len || format == RTL_FACTORY_NONE)
		return 0;
	if (radio > 1 || (paths != 2 && paths != 4) ||
	    (format != RTL_FACTORY_TENDA_KV && format != RTL_FACTORY_RD05_H601))
		return -EINVAL;
	if (format == RTL_FACTORY_RD05_H601) {
		if (paths != 2 || rtl_factory_h601(data, len))
			return -EINVAL;
	}

	for (i = 0; i < sizeof(rtl_factory_fields) / sizeof(rtl_factory_fields[0]); i++) {
		const struct rtl_factory_field *f = &rtl_factory_fields[i];
		const u8 *value;
		u8 *target = (u8 *)rf + f->offset;
		size_t n, count;
		u32 number;
		int ret;

		if (f->path > paths || (f->band && f->band != (band5 ? 5 : 2)))
			continue;
		if (format == RTL_FACTORY_TENDA_KV) {
			snprintf(key, sizeof(key), "HW_WLAN%u_%s", radio, f->key);
			ret = rtl_factory_kv(data, len, key, &value, &n);
			if (!ret)
				continue;
			if (ret < 0)
				goto reject;
		} else {
			value = data + 6 + 0x409 + radio * 2507 + f->rd05_offset;
			n = f->scalar ? 1 : f->size;
		}
		if (f->scalar) {
			if (format == RTL_FACTORY_TENDA_KV) {
				if (rtl_factory_scalar(value, n, f->maximum, &number))
					goto reject;
			} else {
				number = *value;
				if (number > f->maximum)
					goto reject;
			}
			if (f->scalar == 2 && number &&
			    tssi_tables != (band5 ? paths : 2 * paths))
				goto reject;
			if (f->scalar == 1)
				memcpy(target, &number, sizeof(number));
			else
				*target = number;
		} else {
			count = format == RTL_FACTORY_TENDA_KV ? n / 2 : n;
			/* Old MW5 5G base tables have channels 1..177.  Preserve the
			 * remaining vendor/eFuse entries; arbitrary short tables fail. */
			if (count != f->size &&
			    !(band5 && f->size == 196 && count == 177 &&
			      !strncmp(f->key, "TX_POWER_5G_HT40_1S_", 20)))
				goto reject;
			if (format == RTL_FACTORY_TENDA_KV) {
				if (n % 2)
					goto reject;
				for (j = 0; j < count; j++) {
					int hi = rtl_factory_nibble(value[2 * j]);
					int lo = rtl_factory_nibble(value[2 * j + 1]);
					if (hi < 0 || lo < 0)
						break;
					decoded[j] = (hi << 4) | lo;
				}
				if (j != count)
					goto reject;
			} else {
				memcpy(decoded, value, count);
			}
			for (j = 0; j < count && decoded[j] == 0xff; j++)
				;
			if (j == count)
				goto reject;
			memcpy(target, decoded, count);
			if (!strncmp(f->key, "TX_POWER_TSSI_", 14))
				tssi_tables++;
		}
		stats->applied++;
		continue;
reject:
		stats->rejected++;
	}
	return 0;
}

#endif
