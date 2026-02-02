/*	the way to increase or decrease mac address 	*/
/*	Author: Tristan Xiao	*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAC_LEN 18

char help[] = "mac_addr [mode] [mac_address]\n\t[mode]:\t\t\t1 - increase\t2 - decrease\n"
"\t[mac_address] format:\t00-11-22-33-44-55\n";

//usage:#define mac_addr_trivial_usage
//usage:       "COMMAND [MODE] [MAC ADDRESS]"
//usage:#define mac_addr_full_usage "\n\n"
//usage:       "Increase or decrease mac address\n"
//usage:     "\n	mode		1 - increase"
//usage:     "\n			2 - decrease"

int mac_addr(char **mac, int mode)
{
	char *savep, *src, *endptr;
	char *dp;
	int i;
	int tmp[6] = {0};
	int add;
	add = (mode == 1) ? 1 : -1;

	for (src = *mac, i = 0; i < (MAC_LEN / 3); i ++, src = NULL) {
		dp = strtok_r(src, "-", &savep);
		if (dp == NULL)
			break;
		tmp[i] = strtol(dp, &endptr, 16);
		if ((tmp[i] > 0xff) || (tmp[i] < 0x0))
			goto err;
	}
	if (mode == 1) {
		if ((tmp[5] + 1) > 0xff) {
			tmp[5] = 0x0;
			if ((tmp[4] + 1) > 0xff) {
				tmp[4] = 0x0;
				if ((tmp[3] + 1) > 0xff)
					tmp[3] = 0x0;
				else
					tmp[3] += 1;
			}
			else
				tmp[4] += 1;
		}
		else
			tmp[5] += 1;
	}
	else {
		if (tmp[5] == 0) {
			tmp[5] = 0xff;
			if (tmp[4] == 0) {
				tmp[4] = 0xff;
				if (tmp[3] == 0) {
					tmp[3] = 0xff;
				}
				else
					tmp[3] -= 1;
			}
			else
				tmp[4] -= 1;
		}
		else
			tmp[5] -= 1;
	}

	sprintf(*mac, "%02x-%02x-%02x-%02x-%02x-%02x", tmp[0], tmp[1], tmp[2],
			tmp[3], tmp[4], tmp[5]);
	return 0;
err:
	return -1;
}

int mac_addr_main(int argc, char *argv[])
{
	char *mac = NULL;
	int mode = 0;
	int ret = 0;

	if (argc != 3) 
		goto err;

	mode = atoi(argv[1]);
	mac = malloc(sizeof(char) * MAC_LEN); 
	if (mac == NULL)
		goto err;
	memset(mac, 0, MAC_LEN); 
	memcpy(mac, argv[2], MAC_LEN);

	ret = mac_addr(&mac, mode);
	if (ret)
		printf("00-00-00-00-00-00\n");
	else
		printf("%s\n", mac);

	free(mac);
	return 0;
err:
	printf("Usage:\n%s\n", help);
	return -1;
}

