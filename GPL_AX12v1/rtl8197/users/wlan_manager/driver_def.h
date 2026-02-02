#ifndef _DRIVER_DEFINE_H_
#define _DRIVER_DEFINE_H_

#define BIT(x)			(1 << (x))

#if 1
/* G6_WIFI_DRIVER */
enum band_type {
	BAND_ON_24G	= 0,
	BAND_ON_5G	= 1,
	BAND_ON_6G	= 2,
	BAND_MAX,
};

#define BAND_CAP_2G    BIT(BAND_ON_24G)
#define BAND_CAP_5G    BIT(BAND_ON_5G)
#define BAND_CAP_6G    BIT(BAND_ON_6G)

enum WIFI_FRAME_TYPE {
	WIFI_MGT_TYPE       = (0),
	WIFI_CTRL_TYPE      = (BIT(2)),
	WIFI_DATA_TYPE      = (BIT(3)),
	WIFI_QOS_DATA_TYPE	= (BIT(7) | BIT(3)),
};

enum WIFI_FRAME_SUBTYPE {

	/* below is for mgt frame */
	WIFI_ASSOCREQ       = (0 | WIFI_MGT_TYPE),
	WIFI_ASSOCRSP       = (BIT(4) | WIFI_MGT_TYPE),
	WIFI_REASSOCREQ     = (BIT(5) | WIFI_MGT_TYPE),
	WIFI_REASSOCRSP     = (BIT(5) | BIT(4) | WIFI_MGT_TYPE),
	WIFI_PROBEREQ       = (BIT(6) | WIFI_MGT_TYPE),
	WIFI_PROBERSP       = (BIT(6) | BIT(4) | WIFI_MGT_TYPE),
	WIFI_BEACON         = (BIT(7) | WIFI_MGT_TYPE),
	WIFI_ATIM           = (BIT(7) | BIT(4) | WIFI_MGT_TYPE),
	WIFI_DISASSOC       = (BIT(7) | BIT(5) | WIFI_MGT_TYPE),
	WIFI_AUTH           = (BIT(7) | BIT(5) | BIT(4) | WIFI_MGT_TYPE),
	WIFI_DEAUTH         = (BIT(7) | BIT(6) | WIFI_MGT_TYPE),
	WIFI_ACTION         = (BIT(7) | BIT(6) | BIT(4) | WIFI_MGT_TYPE),
	WIFI_ACTION_NOACK   = (BIT(7) | BIT(6) | BIT(5) | WIFI_MGT_TYPE),

	/* below is for control frame */
	WIFI_BF_REPORT_POLL = (BIT(6) | WIFI_CTRL_TYPE),
	WIFI_NDPA           = (BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
	WIFI_BAR            = (BIT(7) | WIFI_CTRL_TYPE),
	WIFI_PSPOLL         = (BIT(7) | BIT(5) | WIFI_CTRL_TYPE),
	WIFI_RTS            = (BIT(7) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),
	WIFI_CTS            = (BIT(7) | BIT(6) | WIFI_CTRL_TYPE),
	WIFI_ACK            = (BIT(7) | BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
	WIFI_CFEND          = (BIT(7) | BIT(6) | BIT(5) | WIFI_CTRL_TYPE),
	WIFI_CFEND_CFACK    = (BIT(7) | BIT(6) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),

	/* below is for data frame */
	WIFI_DATA           = (0 | WIFI_DATA_TYPE),
	WIFI_DATA_CFACK     = (BIT(4) | WIFI_DATA_TYPE),
	WIFI_DATA_CFPOLL    = (BIT(5) | WIFI_DATA_TYPE),
	WIFI_DATA_CFACKPOLL = (BIT(5) | BIT(4) | WIFI_DATA_TYPE),
	WIFI_DATA_NULL      = (BIT(6) | WIFI_DATA_TYPE),
	WIFI_CF_ACK         = (BIT(6) | BIT(4) | WIFI_DATA_TYPE),
	WIFI_CF_POLL        = (BIT(6) | BIT(5) | WIFI_DATA_TYPE),
	WIFI_CF_ACKPOLL     = (BIT(6) | BIT(5) | BIT(4) | WIFI_DATA_TYPE),
	WIFI_QOS_DATA_NULL	= (BIT(6) | WIFI_QOS_DATA_TYPE),
};

#endif

#endif

