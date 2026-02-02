/*
 * iwinfo - Wireless Information Library - Madwifi Backend
 *
 *   Copyright (C) 2009-2010 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The iwinfo library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The iwinfo library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the iwinfo library. If not, see http://www.gnu.org/licenses/.
 */

#include "iwinfo.h"
//#include "iwinfo/wext_scan.h"
#include "iwinfo/nl80211.h"
#include "iwinfo/api/intel.h"

const static char map[][2][IFNAMSIZ] = 
{
    {"wl01", "wlan0"},
    {"wl02", "wlan0.0"},
    {"wl11", "wlan1"},
    {"wl12", "wlan1.0"},
	{"wl21", "wlan2"},
	{"wl22", "wlan2.0"},
};

/* We made ifname convert here. It's ugly, but ... by wxl */
static char* lantiq_ifname(const char* ifname)
{
    int idx = 0;
    int num = sizeof(map) / sizeof(char [2][IFNAMSIZ]);
    if(ifname) 
    {
        for(idx = 0; idx < num; idx++) 
        {
            if(!strcmp(ifname, map[idx][0])) 
            {
                return map[idx][1];
            }
        }
    }
    return ifname;
}

static char * hostapd_info(const char *ifname)
{
	char device[6];
	char path[1024] = { 0 };
	static char buf[16384] = { 0 };
	FILE *conf;

	// setting up the device
	strncpy(device, ifname, 5);
	device[5]='\0';

	snprintf(path, sizeof(path), "/tmp/hostapd_%s.conf", device);

	if ((conf = fopen(path, "r")) != NULL)
	{
		fread(buf, sizeof(buf) - 1, 1, conf);
		fclose(conf);

		return buf;
	}

	return NULL;
}

static char * getval_from_hostapd_conf(const char *ifname, const char *buf, const char *key)
{
	int i, len;
	char lkey[64] = { 0 };
	const char *ln = buf;
	static char lval[256] = { 0 };
	char command[MAX_LEN_RES_VALUE];
	int matched_if = ifname ? 0 : 1;

	for( i = 0, len = strlen(buf); i < len; i++ )
	{
		if (!lkey[0] && (buf[i] == ' ' || buf[i] == '\t'))
		{
				ln++;
		}
		else if (!lkey[0] && (buf[i] == '='))
		{
				if ((&buf[i] - ln) > 0)
					memcpy(lkey, ln, MIN(sizeof(lkey) - 1, &buf[i] - ln));
		}
		else if (buf[i] == '\n')
		{
			if (lkey[0])
			{
				memcpy(lval, ln + strlen(lkey) + 1,
					MIN(sizeof(lval) - 1, &buf[i] - ln - strlen(lkey) - 1));

				if ((ifname != NULL) &&
					(!strcmp(lkey, "interface") || !strcmp(lkey, "bss")) )
				{
					matched_if = !strcmp(lval, ifname);
				}
				else if (matched_if && !strcmp(lkey, key))
				{
					return lval;
				}
			}

			ln = &buf[i+1];
			memset(lkey, 0, sizeof(lkey));
			memset(lval, 0, sizeof(lval));
		}
	}

	return NULL;
}

int get_encryption(const char *ifname, struct iwinfo_crypto_entry *c)
{
	char *host_conf;
	char *param;
	char command[MAX_LEN_RES_VALUE];

	host_conf = hostapd_info(ifname);
	if (!host_conf){
		sprintf(command, "echo 'failed to read hostapd conf file:' > /dev/console");
		system(command);
		return FAIL;
	}

	// for wep we use the hostapd conf file
	param = getval_from_hostapd_conf(ifname, host_conf, "wep_key0");
	if(param)
	{ /* check if this is wep */
		c->enabled = 1;
		c->auth_suites = IWINFO_KMGMT_NONE;
		c->auth_algs = IWINFO_AUTH_OPEN;
		c->wpa_version = 0;
		c->pair_ciphers = 0;
		c->group_ciphers = 0;

		return SUCCESS;
	}

	param = getval_from_hostapd_conf(ifname, host_conf, "wpa");
	if(param) {
		c->wpa_version = param[0] - '0';
	}

	param = getval_from_hostapd_conf(ifname, host_conf, "wpa_key_mgmt");
	if(param) {
		if (strncmp(param, "WPA-EAP", 3) == 0){
			c->auth_suites |= IWINFO_KMGMT_8021x;
		} else {
			c->auth_suites |= IWINFO_KMGMT_PSK;
		}
		c->enabled = 1;
	}

	param = getval_from_hostapd_conf(ifname, host_conf, "auth_algs");
	if(param) {
		c->auth_algs=param[0] - '0';
	}

	param = getval_from_hostapd_conf(ifname, host_conf, "wpa_pairwise");
	if(param) {
		if (strncmp(param, "TKIP", 5) == 0) {
			c->pair_ciphers |= IWINFO_CIPHER_TKIP;
			c->group_ciphers |= IWINFO_CIPHER_TKIP;
		} else if (strncmp(param, "CCMP", 5) == 0) {
			c->pair_ciphers |= IWINFO_CIPHER_CCMP;
			c->group_ciphers |= IWINFO_CIPHER_CCMP;
		} else {
			c->pair_ciphers |= IWINFO_CIPHER_CCMP | IWINFO_CIPHER_TKIP;
			c->group_ciphers |= IWINFO_CIPHER_CCMP | IWINFO_CIPHER_TKIP;
		}
	}

	return SUCCESS;
}
int intel_probe(const char *ifname)
{
	if (strstr(lantiq_ifname(ifname), "wlan") != NULL)
		return TRUE;

	return FALSE;
}

int intel_get_txpower(const char *ifname, int *buf)
{
	return nl80211_ops.txpower(lantiq_ifname(ifname), buf);
}

int intel_get_bitrate(const char *ifname, int *buf)
{
	return nl80211_ops.bitrate(lantiq_ifname(ifname), buf);
}

int intel_get_signal(const char *ifname, int *buf)
{
	return nl80211_ops.signal(lantiq_ifname(ifname), buf);
}

int intel_get_country(const char *ifname, char *buf)
{
	return nl80211_ops.country(lantiq_ifname(ifname), buf);
}

int intel_get_encryption(const char *ifname, char *buf)
{
	return get_encryption(lantiq_ifname(ifname), (struct iwinfo_crypto_entry *)buf);
}

#define IWINFO_CMD_STRING_LENGTH 256
static int get_activetime_from_hostapd(const char *ifname, unsigned char* mac)
{
	FILE *fp = NULL;
	int ret = 0;
	char cmd[IWINFO_CMD_STRING_LENGTH] = "\0";
	snprintf(cmd, IWINFO_CMD_STRING_LENGTH, "/opt/lantiq/bin/hostapd_cli -i%s sta %02x:%02x:%02x:%02x:%02x:%02x | sed -n \"s/^connected_time=\\(.*\\)/\\1/p\"", 
				lantiq_ifname(ifname), mac[0]&0xff, mac[1]&0xff, mac[2]&0xff,
				mac[3]&0xff, mac[4]&0xff, mac[5]&0xff);

	fp = popen(cmd, "r");
	if (fp)
	{
		char line[100] = {'\0'};
		while (fgets(line, 100, fp) != '\0')
		{
			sscanf(line, "%d", &ret);
		}

		pclose(fp);
		fp = NULL;
	}

	return ret;
}

int intel_get_assoclist(const char *ifname, char *buf, int *len)
{
	int ret = nl80211_ops.assoclist(lantiq_ifname(ifname), buf, len);
	int i = 0;
	int slen = *len;
	int sta_num = (int)(slen/(sizeof(struct iwinfo_assoclist_entry)));
	struct iwinfo_assoclist_entry *e = (struct iwinfo_assoclist_entry *)buf;
	for (i = 0; i < sta_num; i++,e++)
	{
		e->rx_rate.rate *=10; // Mbps
		e->tx_rate.rate *=10; // Mbps
		e->inactive = get_activetime_from_hostapd(ifname, e->mac) * 1000; // ms
	}
	return ret;
}

int intel_get_freqlist(const char *ifname, char *buf, int *len)
{
	return nl80211_ops.freqlist(lantiq_ifname(ifname), buf, len);
}

int intel_get_mode(const char *ifname, int *buf)
{
	return nl80211_ops.mode(lantiq_ifname(ifname), buf);
}

/* Using WEXT implementation */

int intel_get_hwmodelist(const char *ifname, int *buf)
{
	//return wext_ops.hwmodelist(lantiq_ifname(ifname), buf);
	char *if_name=lantiq_ifname(ifname);
	char chans[IWINFO_BUFSIZE] = { 0 };
	struct iwinfo_freqlist_entry *e = NULL;
	int len = 0;

	*buf = 0;

	if( !wext_get_freqlist(if_name, chans, &len) )
	{
		for( e = (struct iwinfo_freqlist_entry *)chans; e->channel; e++ )
		{
			if( e->channel <= 14 )
			{
				*buf |= IWINFO_80211_B;
				*buf |= IWINFO_80211_G;
				*buf |= IWINFO_80211_N;
				*buf |= IWINFO_80211_AX;
			}
			else
			{
				*buf |= IWINFO_80211_A;
				*buf |= IWINFO_80211_N;
				*buf |= IWINFO_80211_AC;
				*buf |= IWINFO_80211_AX;
			}
		}

		return 0;
	}

	return -1;
}

int intel_get_channel(const char *ifname, int *buf)
{
	return wext_ops.channel(lantiq_ifname(ifname), buf);
}

int intel_get_frequency(const char *ifname, int *buf)
{
	return wext_ops.frequency(lantiq_ifname(ifname), buf);
}

int intel_get_frequency_offset(const char *ifname, int *buf)
{
	return nl80211_ops.frequency_offset(lantiq_ifname(ifname), buf);
}

int intel_get_txpower_offset(const char *ifname, int *buf)
{
	return nl80211_ops.txpower_offset(lantiq_ifname(ifname), buf);
}

int intel_get_noise(const char *ifname, int *buf)
{
	return wext_ops.noise(lantiq_ifname(ifname), buf);
}

int intel_get_quality(const char *ifname, int *buf)
{
	return wext_ops.quality(lantiq_ifname(ifname), buf);
}

int intel_get_quality_max(const char *ifname, int *buf)
{
	return wext_ops.quality_max(lantiq_ifname(ifname), buf);
}

int intel_get_mbssid_support(const char *ifname, int *buf)
{
	return wext_ops.mbssid_support(lantiq_ifname(ifname), buf);
}

int intel_get_ssid(const char *ifname, char *buf)
{
	return wext_ops.ssid(lantiq_ifname(ifname), buf);
}

int intel_get_bssid(const char *ifname, char *buf)
{
	return wext_ops.bssid(lantiq_ifname(ifname), buf);
}

int intel_get_hardware_id(const char *ifname, char *buf)
{
	return wext_ops.hardware_id(lantiq_ifname(ifname), buf);
}

int intel_get_hardware_name(const char *ifname, char *buf)
{
	return wext_ops.hardware_name(lantiq_ifname(ifname), buf);
}

int intel_get_txpwrlist(const char *ifname, char *buf, int *len)
{
	return wext_ops.txpwrlist(lantiq_ifname(ifname), buf, len);
}

int intel_get_countrylist(const char *ifname, char *buf, int *len)
{
	return wext_ops.countrylist(lantiq_ifname(ifname), buf, len);
}

int intel_get_scanlist(const char *ifname, char *buf, int *len)
{
	return wext_ops.scanlist(lantiq_ifname(ifname), buf, len);
}

int intel_get_beacon_int(const char *ifname, int *buf)
{
	return nl80211_ops.beacon_int(lantiq_ifname(ifname), buf);
}

void intel_close(void)
{
	/* Nop */
}

static const struct iwinfo_ops intel_ops = {
	.name             = "intel",
	.probe            = intel_probe,

	.mbssid_support   = intel_get_mbssid_support,
	//.htmodelist       = intel_get_htmodelist,
	//.phyname          = intel_get_phyname,

	.channel          = intel_get_channel,
	.frequency        = intel_get_frequency,
	.frequency_offset = intel_get_frequency_offset,
	.txpower          = intel_get_txpower,
	.txpower_offset   = intel_get_txpower_offset,
	.bitrate          = intel_get_bitrate,
	.signal           = intel_get_signal,
	.noise            = intel_get_noise,
	.quality          = intel_get_quality,
	.quality_max      = intel_get_quality_max,
	.hwmodelist       = intel_get_hwmodelist,
	.mode             = intel_get_mode,
	.ssid             = intel_get_ssid,
	.bssid            = intel_get_bssid,
	.country          = intel_get_country, //c
	.hardware_id      = intel_get_hardware_id,
	.hardware_name    = intel_get_hardware_name,
	.encryption       = intel_get_encryption,
	.assoclist        = intel_get_assoclist, //a
	.txpwrlist        = intel_get_txpwrlist,
	.scanlist         = intel_get_scanlist,
	.freqlist         = intel_get_freqlist, //f
	.countrylist      = intel_get_countrylist,
	.beacon_int       = intel_get_beacon_int,

	.close            = intel_close,
};
}
