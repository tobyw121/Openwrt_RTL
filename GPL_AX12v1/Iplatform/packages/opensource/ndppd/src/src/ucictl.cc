#include <iostream>
#include <string.h>
#include "ndppd.h"

NDPPD_NS_BEGIN

int uci_get_config_option(char* config, char* sec_name, char* option,
	char* result, int size)
{
	int res = 0;
	struct uci_context* uciCtx = NULL;
	struct uci_package* pkg = NULL;
	struct uci_element* element = NULL;
	struct uci_section* section = NULL;
	const char* value = NULL;

	uciCtx = uci_alloc_context();
	uci_set_confdir(uciCtx, "/etc/config");
	//uci_set_savedir(uciCtx, "/var/state");
	if (UCI_OK != uci_load(uciCtx, config, &pkg))
	{
		uci_perror(uciCtx, config);
		logger::debug() << "uci: load " << config << " config failed.";
		goto err;
	}

	section = uci_lookup_section(uciCtx, pkg, sec_name);
	if (NULL == section)
	{
		uci_perror(uciCtx, sec_name);
		logger::debug() << "uci: get " << sec_name << " section failed.";
		goto ret;
	}

	value = uci_lookup_option_string(uciCtx, section, option);
	if (NULL == value)
	{
		logger::debug() << "uci: get " << option << " option failed.";
		goto ret;
	}
	logger::debug() << option << " of " << sec_name << " in " <<
		config << " is " << value;

	res = strlen(value);
	strncpy(result, value, size);
	result[size - 1] = '\0';

ret:
	uci_unload(uciCtx, pkg);
err:
	uci_free_context(uciCtx);
	uciCtx = NULL;

	return res;
}

/* uci get network.wanv6/internet.proto */
std::string uci_get_proto()
{
	char proto[15] = {0};
	int option_len = 0;
	char net[15] = "network";
	char wan[15] = "wanv6";
	char iet[15] = "internet";
	char pro[15] = "proto";
	option_len = uci_get_config_option(net, wan, pro, proto, sizeof(proto));
	if (option_len <= 0)
	{
		logger::debug() << "Proto: NULL";
		option_len = uci_get_config_option(net, iet, pro, proto, sizeof(proto));
	}
	else
	{
		logger::debug() << "Proto: " << proto;
	}
	return proto;
}

/* uci get network.wanv6/internet.ifname */
std::string uci_get_ifname()
{
	char ifname[15] = {0};
	int option_len = 0;
	char net[15] = "network";
	char wan[15] = "wanv6";
	char iet[15] = "internet";
	char ifn[15] = "ifname";
	option_len = uci_get_config_option(net, wan, ifn, ifname, sizeof(ifname));
	if (option_len <= 0)
	{
		logger::debug() << "Ifname: NULL";
		option_len = uci_get_config_option(net, iet, ifn, ifname, sizeof(ifname));
	}
	else
	{
		logger::debug() << "Ifname: " << ifname;
	}
	return ifname;
}

NDPPD_NS_END