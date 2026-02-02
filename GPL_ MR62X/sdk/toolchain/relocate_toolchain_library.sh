#! /bin/sh

if [ -z "$1" ] ; then 
	printf "Usage: $0 TOOLCHAIN_ROOT_PATH\n"
	exit -1
fi

TOOLCHAIN_PATH="$1"
OLD_TOOLCHAIN_PREFIX="$2"
NEW_TOOLCHAIN_PREFIX="$3"

ALL_FILES=$(grep -rl ${OLD_TOOLCHAIN_PREFIX} ${TOOLCHAIN_PATH})

ASCII_FILES=$(for i in ${ALL_FILES}; do file $i | grep "text" | cut -d':' -f1; done)

NOT_ASCII_FILES=$(for i in ${ALL_FILES}; do file $i | grep -v "text" | cut -d':' -f1; done)

ELF_EXECUTABLE_FILES=$(for i in $(echo $NOT_ASCII_FILES ); do file $i | grep -E "ELF[0-9| |-|a-z|A-Z|-]*(shared object|executable)" | cut -d':' -f1 ; done)

LIBTOOL_FILES=$(for i in $(echo $NOT_ASCII_FILES ); do file $i | grep 'libtool library file' | cut -d':' -f1 ; done)

echo process ascii file
for i in $(echo ${ASCII_FILES} ${LIBTOOL_FILES})
do
	echo process: "${i}"
	sed -i "s|${OLD_TOOLCHAIN_PREFIX}|${NEW_TOOLCHAIN_PREFIX}/|g" "$i"
done

echo process elf file
curr_path=
for i in $(echo ${ELF_EXECUTABLE_FILES})
do
	echo process: "${i}"
	curr_path=$(patchelf --print-rpath ${i})
	curr_path=$(echo $curr_path | sed "s|${OLD_TOOLCHAIN_PREFIX}|${NEW_TOOLCHAIN_PREFIX}/|g")
	
	if [ -n "$curr_path" ]; then
		patchelf --set-rpath $curr_path $i;
	fi
	
done

