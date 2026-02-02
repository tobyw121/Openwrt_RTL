cmake_minimum_required(VERSION 2.6)

PROJECT(netifd C)
ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -Wmissing-declarations -Wno-misleading-indentation -Wno-format-truncation)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

IF(APPLE)
  INCLUDE_DIRECTORIES(/opt/local/include)
  LINK_DIRECTORIES(/opt/local/lib)
ENDIF()

SET(SOURCES
	main.c utils.c system.c tunnel.c
	interface.c interface-ip.c interface-event.c
	iprule.c proto.c proto-static.c proto-shell.c
	config.c device.c bridge.c bonding.c vlan.c alias.c
	macvlan.c ubus.c connect.c)

IF(__3G4G__)
	SET(SOURCES  ${SOURCES} backup.c backup_config.c)
ENDIF()

IF(__3G4G_V2__)
	SET(SOURCES  ${SOURCES} backup_v2.c backup_config_v2.c)
ENDIF()

IF(__WIRELESS__)
	SET(SOURCES ${SOURCES} wireless.c)
ENDIF()

find_library(json NAMES json-c json)
SET(LIBS
	ubox ubus uci ${json} blobmsg_json)

IF (NOT DEFINED LIBNL_LIBS)
  FIND_LIBRARY(libnl NAMES libnl-3 libnl nl-3 nl)
  SET(LIBNL_LIBS ${libnl})
ENDIF()

IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux" AND NOT DUMMY_MODE)
	SET(SOURCES ${SOURCES} system-linux.c)
	SET(LIBS ${LIBS} ${LIBNL_LIBS})
ELSE()
	ADD_DEFINITIONS(-DDUMMY_MODE=1)
	SET(SOURCES ${SOURCES} system-dummy.c)
ENDIF()

IF(DEBUG)
  ADD_DEFINITIONS(-DDEBUG -g3)
  IF(NO_OPTIMIZE)
    ADD_DEFINITIONS(-O0)
  ENDIF()
ENDIF()

IF(__ARM__)
    ADD_DEFINITIONS(-D__ARM__)
ENDIF()

IF(KEEP_BRIDGE)
    ADD_DEFINITIONS(-DKEEP_BRIDGE)
ENDIF()

IF(NETIFD_HAS_DOWNUP_BUG)
    ADD_DEFINITIONS(-DNETIFD_HAS_DOWNUP_BUG)
ENDIF()

IF(NETIFD_WITH_BCM_IPTV)
    ADD_DEFINITIONS(-DNETIFD_WITH_BCM_IPTV)
ENDIF()

IF(NETIFD_WITH_V6PLUS)
    ADD_DEFINITIONS(-DNETIFD_WITH_V6PLUS)
ENDIF()

ADD_EXECUTABLE(netifd ${SOURCES})

TARGET_LINK_LIBRARIES(netifd ${LIBS})

INSTALL(TARGETS netifd
	RUNTIME DESTINATION sbin
)
