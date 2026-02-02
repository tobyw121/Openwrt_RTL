#!/bin/bash
TARGET="$( cd "$( dirname "$0" )" && pwd )"
#echo 111=$TARGET
if [ -d "$TARGET/.git" ]; then
	$TARGET/phl/gen_git_info.sh $TARGET
else
	$TARGET/phl/gen_git_info.sh
fi
cd $TARGET

