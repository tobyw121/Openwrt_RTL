## Process this file with automake to produce Makefile.in


EXTRA_DIST = BUGS COPYING TODO INSTALL README src/rlwrap.h src/redblack.h src/malloc_debug.h src/completion.rb doc/rlwrap.man.in test/testclient\
             test/testit completions/ftp completions/testclient completions/coqtop
TESTS      = test/testit
SUBDIRS    = doc src
pkgdata_DATA  = completions/ftp completions/testclient completions/coqtop

clean-local: 
	rm -f *~ 

