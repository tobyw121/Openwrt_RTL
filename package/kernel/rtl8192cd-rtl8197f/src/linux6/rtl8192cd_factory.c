// SPDX-License-Identifier: GPL-2.0-only
/* Restore OEM board RF parameters after vendor/eFuse defaults, without WEXT. */
#include "../8192cd.h"
#include "rtl8192cd_linux6.h"
#include "rtl8192cd_factory.h"

void rtl8192cd_linux6_apply_factory(struct rtl8192cd_priv *priv)
{
	struct rtl_factory_stats stats;
	unsigned int paths = 2;
	bool band5;
	int ret;

	if (!priv->pshare || !priv->pshare->linux_dev ||
	    !IS_ROOT_INTERFACE(priv) || !priv->pshare->linux6_factory_len)
		return;
	switch (GET_CHIP_VER(priv)) {
	case VERSION_8197F:
		band5 = false;
		break;
	case VERSION_8814B:
		paths = 4;
		fallthrough;
	case VERSION_8812F:
	case VERSION_8822B:
		band5 = true;
		break;
	default:
		return;
	}
	ret = rtl_factory_apply(&priv->pmib->dot11RFEntry,
				priv->pshare->linux6_factory_format,
				priv->pshare->linux6_radio_index, paths, band5,
				priv->pshare->linux6_factory,
				priv->pshare->linux6_factory_len, &stats);
	priv->pshare->linux6_factory_applied = stats.applied;
	priv->pshare->linux6_factory_rejected = stats.rejected + !!ret;
	if (ret)
		dev_warn(priv->pshare->linux_dev,
			 "invalid factory RF layout; keeping vendor/eFuse defaults\n");
	else
		dev_info(priv->pshare->linux_dev,
			 "factory RF: radio=%u applied=%u rejected=%u\n",
			 priv->pshare->linux6_radio_index, stats.applied, stats.rejected);
}
