## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libspp.a

SUBDIRS = Stream6 Session

if BUILD_PROCPIDSTATS
PROCPIDSTATS_SOURCE = sfprocpidstats.c sfprocpidstats.h
endif

libspp_a_SOURCES = spp_arpspoof.c spp_arpspoof.h spp_bo.c spp_bo.h \
spp_rpc_decode.c spp_rpc_decode.h  \
$(PROCPIDSTATS_SOURCE) \
portscan.c portscan.h \
spp_sfportscan.c spp_sfportscan.h \
str_search.c str_search.h \
spp_stream6.c spp_stream6.h \
spp_session.c spp_session.h \
session_api.c session_api.h \
stream_api.c stream_api.h \
spp_normalize.c spp_normalize.h \
normalize.c normalize.h \
sip_common.h cip_common.h


INCLUDES = @INCLUDES@
