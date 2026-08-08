#!/bin/bash
if [ -z "$1" ] ; then
	
	TARGET="$( cd "$( dirname "$0" )" && pwd )"
else
	TARGET=$1
fi

if [ -d "$TARGET/.git" ] ; then
	if [ -f "$TARGET/git_info.h" ]; then 
		rm "$TARGET/git_info.h"; 
	fi

	# -- GET SHA1 & TAG INFO --
	cd $TARGET
	MAIN_SHA1=`git rev-parse HEAD`
	MAIN_TAGINFO=`git describe --tags --long --always`

	#CMN_INFO_DIR="$(TARGET)/cmn_info_file"
	cd $TARGET/cmn_info_file
	CMN_INFO_SHA1=`git rev-parse HEAD`
	CMN_INFO_TAGINFO=`git describe --tags --long --always`
	
	#PHYDM_DIR=$(TARGET)/phydm
	cd $TARGET/phydm
	PHYDM_SHA1=`git rev-parse HEAD`
	PHYDM_TAGINFO=`git describe --tags --long --always`

	#PHYDM_HALRF_DIR=$(PHYDM_DIR)/halrf
	cd $TARGET/phydm/halrf
	PHYDM_HALRF_SHA1=`git rev-parse HEAD`
	PHYDM_HALRF_TAGINFO=`git describe --tags --long --always`
	
	#	get git_info_prefix to headfile
	cat "$TARGET/git_info_prefix" >> "$TARGET/git_info.h"
	
	# top dir
	echo "#define MAIN_SHA1   ""\"$MAIN_SHA1\"" >> "$TARGET/git_info.h"
	echo "#define MAIN_TAGINFO     ""\"$MAIN_TAGINFO\"" >> "$TARGET/git_info.h"
	echo -en '\n' >> "$TARGET/git_info.h"

	# cmn_info
	echo "#define CMN_INFO_SHA1   ""\"$CMN_INFO_SHA1\"" >> "$TARGET/git_info.h"
	echo "#define CMN_INFO_TAGINFO     ""\"$CMN_INFO_TAGINFO\"" >> "$TARGET/git_info.h"
	echo -en '\n' >> "$TARGET/git_info.h"

	# phydm
	echo "#define PHYDM_SHA1   ""\"$PHYDM_SHA1\"" >> "$TARGET/git_info.h"
	echo "#define PHYDM_TAGINFO     ""\"$PHYDM_TAGINFO\"" >> "$TARGET/git_info.h"
	echo -en '\n' >> "$TARGET/git_info.h"

	# phydm\halrf
	echo "#define PHYDM_HALRF_SHA1   ""\"$PHYDM_HALRF_SHA1\"" >> "$TARGET/git_info.h"
	echo "#define PHYDM_HALRF_TAGINFO     ""\"$PHYDM_HALRF_TAGINFO\"" >> "$TARGET/git_info.h"
	echo -en '\n' >> "$TARGET/git_info.h"
	
	#end of header file
	echo "#endif" >> "$TARGET/git_info.h"
else
	cp $TARGET/git_info2.h $TARGET/git_info.h 
	sed -i 's/__GIT_INFO2_H__/__GIT_INFO_H__/g' $TARGET/git_info.h
	sed -i 's/RTK_//g' $TARGET/git_info.h
fi