#
# Realtek Semiconductor Corp.
# To use this package, BRIDGE_NF_EBTABLES in the Linux kernel must be enabled.
# The locatation of BRIDGE_NF_EBTABLES is:
#	-> Networking support                                                                                                                            │  
#	  -> Networking options                                                                                                                                         │  
#	    -> Network packet filtering framework (Netfilter)                                                                                         │  
#	       -> Ethernet Bridge tables (ebtables) support (BRIDGE_NF_EBTABLES)
#

.PHONY: all romfs clean

all:
	make -j12 -C ebtables ;

romfs:
	make install -C ebtables;

clean:
	make clean -C ebtables;
