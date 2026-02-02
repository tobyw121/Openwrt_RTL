#pragma once

#include "ndppd.h"

NDPPD_NS_BEGIN

int uci_get_config_option(char* config, char* sec_name, char* option, char* result, int size);
std::string uci_get_proto();
std::string uci_get_ifname();

NDPPD_NS_END