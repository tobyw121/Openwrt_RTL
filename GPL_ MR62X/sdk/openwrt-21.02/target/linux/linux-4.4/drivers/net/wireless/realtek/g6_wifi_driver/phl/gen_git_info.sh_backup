#!/bin/bash
if [ -z "$1" ]; then
	TARGET="$( cd "$( dirname "$0" )" && pwd )"
else
	TARGET=$1
fi
#echo 222=$TARGET
if [ -d "$TARGET/.git" ] && [ -d "$TARGET/phl" ]; then
	if [ -f "$TARGET/phl/phl_git_info.h" ]; then rm "$TARGET/phl/phl_git_info.h"; fi
	# -- GET SHA1 & TAG INFO --
	cd $TARGET
	RTK_CORE_SHA1=`git rev-parse HEAD`
	RTK_CORE_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_CORE_TAGINFO:0:4}" != tags ] && RTK_CORE_TAGINFO=${RTK_CORE_TAGINFO##*/}
	cd $TARGET/phl
	RTK_PHL_SHA1=`git rev-parse HEAD`
	RTK_PHL_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_PHL_TAGINFO:0:4}" != tags ] && RTK_PHL_TAGINFO=${RTK_PHL_TAGINFO##*/}
	cd $TARGET/phl/hal_g6/mac
	RTK_HALMAC_SHA1=`git rev-parse HEAD`
	RTK_HALMAC_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_HALMAC_TAGINFO:0:4}" != tags ] && RTK_HALMAC_TAGINFO=${RTK_HALMAC_TAGINFO##*/}
	cd $TARGET/phl/hal_g6/mac/fw_ax
	RTK_FW_SHA1=`git rev-parse HEAD`
	RTK_FW_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_FW_TAGINFO:0:4}" != tags ] && RTK_FW_TAGINFO=${RTK_FW_TAGINFO##*/}
	cd $TARGET/phl/hal_g6/phy/bb
	RTK_HALBB_SHA1=`git rev-parse HEAD`
	RTK_HALBB_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_HALBB_TAGINFO:0:4}" != tags ] && RTK_HALBB_TAGINFO=${RTK_HALBB_TAGINFO##*/}
	cd $TARGET/phl/hal_g6/phy/rf
	RTK_HALRF_SHA1=`git rev-parse HEAD`
	RTK_HALRF_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_HALRF_TAGINFO:0:4}" != tags ] && RTK_HALRF_TAGINFO=${RTK_HALRF_TAGINFO##*/}
	cd $TARGET/phl/hal_g6/btc
	RTK_BTC_SHA1=`git rev-parse HEAD`
	RTK_BTC_TAGINFO=`git describe --abbrev=0 --tags --all`
	[ "${RTK_BTC_TAGINFO:0:4}" != tags ] && RTK_BTC_TAGINFO=${RTK_BTC_TAGINFO##*/}
	# -- PRINT HEADER --
	cat "$TARGET/phl/phl_git_info_header.txt" >> "$TARGET/phl/phl_git_info.h"
	# -- PRINT SHA1 & TAG INFO --
	echo "#define RTK_CORE_SHA1   ""\"$RTK_CORE_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_PHL_SHA1    ""\"$RTK_PHL_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALMAC_SHA1 ""\"$RTK_HALMAC_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_FW_SHA1 ""\"$RTK_FW_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALBB_SHA1  ""\"$RTK_HALBB_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALRF_SHA1  ""\"$RTK_HALRF_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_BTC_SHA1    ""\"$RTK_BTC_SHA1\"" >> "$TARGET/phl/phl_git_info.h"
	echo -en '\n' >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_CORE_TAGINFO     ""\"$RTK_CORE_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_PHL_TAGINFO      ""\"$RTK_PHL_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALMAC_TAGINFO   ""\"$RTK_HALMAC_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_FW_TAGINFO   ""\"$RTK_FW_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALBB_TAGINFO    ""\"$RTK_HALBB_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_HALRF_TAGINFO    ""\"$RTK_HALRF_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo "#define RTK_BTC_TAGINFO      ""\"$RTK_BTC_TAGINFO\"" >> "$TARGET/phl/phl_git_info.h"
	echo -en '\n' >> "$TARGET/phl/phl_git_info.h"
	echo "#endif /* __PHL_GIT_INFO_H__ */" >> "$TARGET/phl/phl_git_info.h"
else
	if [ -f "$TARGET/phl_git_info.h" ]; then rm "$TARGET/phl_git_info.h"; fi
	# -- PRINT HEADER --
	cat "$TARGET/phl_git_info_header.txt" >> "$TARGET/phl_git_info.h"
	# -- PRINT SHA1 & TAG INFO --
	echo "#define RTK_CORE_SHA1   \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_PHL_SHA1    \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALMAC_SHA1 \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_FW_SHA1 \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALBB_SHA1  \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALRF_SHA1  \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_BTC_SHA1    \"0\"" >> "$TARGET/phl_git_info.h"
	echo -en '\n' >> "$TARGET/phl_git_info.h"
	echo "#define RTK_CORE_TAGINFO     \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_PHL_TAGINFO      \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALMAC_TAGINFO   \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_FW_TAGINFO   \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALBB_TAGINFO    \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_HALRF_TAGINFO    \"0\"" >> "$TARGET/phl_git_info.h"
	echo "#define RTK_BTC_TAGINFO      \"0\"" >> "$TARGET/phl_git_info.h"
	echo -en '\n' >> "$TARGET/phl_git_info.h"
	echo "#endif /* __PHL_GIT_INFO_H__ */" >> "$TARGET/phl_git_info.h"
fi

