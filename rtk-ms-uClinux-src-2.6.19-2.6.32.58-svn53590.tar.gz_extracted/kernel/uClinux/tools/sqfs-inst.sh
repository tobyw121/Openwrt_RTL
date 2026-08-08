#!/bin/sh
#
# A tool to simplify Makefiles that need to put something
# into the SQFS
#
# Copyright (C) David McCullough, 2002,2003
#
#############################################################################

# Provide a default PATH setting to avoid potential problems...
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:$PATH"

usage()
{
cat << !EOF >&2
$0: [options] [src] dst
    -v          : output actions performed.
    -e env-var  : only take action if env-var is set to "y".
    -o option   : only take action if option is set to "y".
    -p perms    : chmod style permissions for dst.
    -d          : make dst directory if it doesn't exist
    -S          : don't strip after installing
    -a text     : append text to dst.
    -A pattern  : only append text if pattern doesn't exist in file
    -l link     : dst is a hard link to 'link'.
    -s sym-link : dst is a sym-link to 'sym-link'.

    if "src" is not provided,  basename is run on dst to determine the
    source in the current directory.

    multiple -e and -o options are ANDed together.  To achieve an OR affect
    use a single -e/-o with 1 or more y/n/"" chars in the condition.

    if src is a directory,  everything in it is copied recursively to dst
    with special files removed (currently CVS and Subversion dirs).
!EOF
	exit 1
}

#############################################################################

setperm()
{
	rc=0
	# Always start with write access for the user so that files can be
	# updated/overwritten during make romfs
	chmod u+w ${SQFSDIR}${dst}
	if [ "$perm" ]
	then
		[ "$v" ] && echo "chmod ${perm} ${SQFSDIR}${dst}"
		chmod ${perm} ${SQFSDIR}${dst}
		rc=$?
	fi
	return $rc
}

#############################################################################

file_copy()
{
	rc=0
	if [ -d "${src}" ]
	then
		[ "$v" ] && echo "CopyDir ${src} ${SQFSDIR}${dst}"
		(
			cd ${src} || return 1
			V=
			[ "$v" ] && V=v
			find . -print | grep -E -v '/CVS|/\.svn' | cpio -p${V}dumL ${SQFSDIR}${dst}
			rc=$?
			# And make sure these files are still writable
			find . -print | grep -E -v '/CVS|/\.svn' | ( cd ${SQFSDIR}${dst}; xargs chmod u+w )
		)
	else
		if [ -d ${dst} ]; then
			dstfile=${SQFSDIR}${dst}/`basename ${src}`
			ln_target=${SQFSROMDIR}${dst}/`basename ${src}`
			ln_name=${ROMFSDIR}${dst}/`basename ${src}`
		else
			dstfile=${SQFSDIR}${dst}
			ln_target=${SQFSROMDIR}${dst}
			ln_name=${ROMFSDIR}${dst}
		fi
		rm -f ${dstfile}
		[ "$v" ] && echo "cp ${src} ${dstfile}"
		cp ${src} ${dstfile} && setperm ${dstfile}
		ln -sf ${ln_target} ${ln_name}
		rc=$?
		if [ $rc -eq 0 -a -n "$strip" ]; then
			${STRIPTOOL} ${dstfile} 2>/dev/null
			${STRIPTOOL} -R .comment -R .note ${dstfile} 2>/dev/null
		fi
	fi
	return $rc
}

#############################################################################

file_append()
{
	touch ${SQFSDIR}${dst} || return 1
	if [ -z "${pattern}" ] && grep -F "${src}" ${SQFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File entry already installed."
	elif [ "${pattern}" ] && egrep "${pattern}" ${SQFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File pattern already installed."
	else
		[ "$v" ] && echo "Installing entry into ${SQFSDIR}${dst}."
		echo "${src}" >> ${SQFSDIR}${dst} || return 1
	fi
	setperm ${SQFSDIR}${dst}
	return $?
}

#############################################################################

hard_link()
{
	rm -f ${SQFSDIR}${dst}
	[ "$v" ] && echo "ln ${src} ${SQFSDIR}${dst}"
	ln ${SQFSDIR}${src} ${SQFSDIR}${dst}
	return $?
}

#############################################################################

sym_link()
{
	rm -f ${SQFSDIR}${dst}
	[ "$v" ] && echo "ln -s ${src} ${SQFSDIR}${dst}"
	ln -sf ${src} ${SQFSDIR}${dst}
	return $?
}

#############################################################################
#
# main program entry point
#

if [ -z "$SQFSDIR" ]
then
	echo "SQFSDIR is not set" >&2
	usage
	exit 1
fi

v=
option=y
pattern=
perm=
func=file_copy
mdir=
src=
dst=
strip=1

while getopts 'dSve:o:A:p:a:l:s:' opt "$@"
do
	case "$opt" in
	v) v="1";                           ;;
	d) mdir="1";                        ;;
	S) strip=;							;;
	o) option="$OPTARG";                ;;
	e) eval option=\"\$$OPTARG\";       ;;
	p) perm="$OPTARG";                  ;;
	a) src="$OPTARG"; func=file_append; ;;
	A) pattern="$OPTARG";               ;;
	l) src="$OPTARG"; func=hard_link;   ;;
	s) src="$OPTARG"; func=sym_link;    ;;

	*)  break ;;
	esac
#
#	process option here to get an ANDing effect
#
	case "$option" in
	*[mMyY]*) # this gives OR effect, ie., nYn
		;;
	*)
		[ "$v" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac
done

shift `expr $OPTIND - 1`

case $# in
1)
	dst="$1"
	if [ -z "$src" ]
	then
		src="`basename $dst`"
	fi
	;;
2)
	if [ ! -z "$src" ]
	then
		echo "Source file already provided" >&2
		exit 1
	fi
	src="$1"
	dst="$2"
	;;
*)
	usage
	;;
esac

if [ "$mdir" -a ! -d "`dirname ${SQFSDIR}${dst}`/." ]
then
	mkdir -p "`dirname ${SQFSDIR}${dst}`/." || exit 1
fi

$func || exit 1

exit 0

#############################################################################
