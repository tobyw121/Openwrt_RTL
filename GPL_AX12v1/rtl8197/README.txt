		REALTEK 11axRouter SDK(based on linux-4.4)-v3.6.0
		-----------------------------------------------------

Package List
============
  1. rtl819x.tar.bz2                          - containing the source code of v3.6.0 sdk
  2. rtl819x-SDK-v3.6.0-bootcode.tar.bz2
  3. README.txt                              - this file
  4. INSTALL.txt                             - how to build code 
  5. Document.tar.bz2                         - containing the documents for this SDK
  6. image.tar.bz2                            - containning the images of each kind of combination.
                                             - The images is specially builded for release.
                                             - It's combines the default configuration with firmware in order to avoid the MIB conflicts.

Environment
===========
  CentOS Linux release 7.6.1810 (Core), Ubuntu 16.04 are recommended

Install the linux-4.4 sdk package
==================================
  1. Copy 'rtl819x.tar.bz2' to a file directory on a Linux PC
  2. Type 'tar -jxvf rtl819x.tar.bz2' to extract the package
 
Install the bootcode package
============================
  Type 'tar -jxvf rtl819x-SDK-v3.6.0-bootcode.tar.bz2' to extract the package

build the linux kernel/rootfs/bootcode
======================================
  follow the INSTALL.txt file
