#!/bin/bash
if [ -z "$1" ] ; then
	
	TARGET="$( cd "$( dirname "$0" )" && pwd )"
else
	TARGET=$1
fi

if [ -d "$TARGET/.git" ] ; then
	if [ -f "$TARGET/git_info2.h" ]; then 
		rm "$TARGET/git_info2.h"; 
	fi

	# -- GET SHA1 & TAG INFO --
	cd $TARGET
	RTK_MAIN_SHA1=`git rev-parse HEAD`
	RTK_MAIN_TAGINFO=`git describe --tags --long --always`

	#CMN_INFO_DIR="$(TARGET)/cmn_info_file"
	cd $TARGET/cmn_info_file
	RTK_CMN_INFO_SHA1=`git rev-parse HEAD`
	RTK_CMN_INFO_TAGINFO=`git describe --tags --long --always`
	
	#PHYDM_DIR=$(TARGET)/phydm
	cd $TARGET/phydm
	RTK_PHYDM_SHA1=`git rev-parse HEAD`
	RTK_PHYDM_TAGINFO=`git describe --tags --long --always`

	#PHYDM_HALRF_DIR=$(PHYDM_DIR)/halrf
	cd $TARGET/phydm/halrf
	RTK_PHYDM_HALRF_SHA1=`git rev-parse HEAD`
	RTK_PHYDM_HALRF_TAGINFO=`git describe --tags --long --always`
	
	#	get git_info_prefix to headfile
	cat "$TARGET/git_info_prefix" >> "$TARGET/git_info2.h"
	sed -i 's/__GIT_INFO_H__/__GIT_INFO2_H__/g' $TARGET/git_info2.h
	# top dir
	echo "#define RTK_MAIN_SHA1   ""\"$RTK_MAIN_SHA1\"" >> "$TARGET/git_info2.h"
	echo "#define RTK_MAIN_TAGINFO     ""\"$RTK_MAIN_TAGINFO\"" >> "$TARGET/git_info2.h"
	echo -en '\n' >> "$TARGET/git_info2.h"

	# cmn_info
	echo "#define RTK_CMN_INFO_SHA1   ""\"$RTK_CMN_INFO_SHA1\"" >> "$TARGET/git_info2.h"
	echo "#define RTK_CMN_INFO_TAGINFO     ""\"$RTK_CMN_INFO_TAGINFO\"" >> "$TARGET/git_info2.h"
	echo -en '\n' >> "$TARGET/git_info2.h"

	# phydm
	echo "#define RTK_PHYDM_SHA1   ""\"$RTK_PHYDM_SHA1\"" >> "$TARGET/git_info2.h"
	echo "#define RTK_PHYDM_TAGINFO     ""\"$RTK_PHYDM_TAGINFO\"" >> "$TARGET/git_info2.h"
	echo -en '\n' >> "$TARGET/git_info2.h"

	# phydm\halrf
	echo "#define RTK_PHYDM_HALRF_SHA1   ""\"$RTK_PHYDM_HALRF_SHA1\"" >> "$TARGET/git_info2.h"
	echo "#define RTK_PHYDM_HALRF_TAGINFO     ""\"$RTK_PHYDM_HALRF_TAGINFO\"" >> "$TARGET/git_info2.h"
	echo -en '\n' >> "$TARGET/git_info2.h"
	
	#end of header file
	echo "#endif" >> "$TARGET/git_info2.h"
else
	echo "no .git file exist check"
	echo "no .git file exist check"
	echo "no .git file exist check"
fi