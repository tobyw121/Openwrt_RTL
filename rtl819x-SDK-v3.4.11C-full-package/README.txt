		REALTEK 11nRouter SDK(based on linux-3.10)- v3.4.11C
		----------------------------------------------------

Package List
============
  1. rtl819x.tar.gz          - containing the source code of v3.4.11C sdk
  2. rtl819x-SDK-v3.4.11C-bootcode-97F.tar.gz
  3. rtl819x-SDK-v3.4.11C-bootcode-98C.tar.gz
  4. README.txt              - this file
  5. INSTALL.txt             - how to build code 
  6. Document.tar.gz         - containing the documents for this SDK
  7. image.tar.gz            - containning the images of each kind of combination.
	                         - The images is specially builded for release.
	 	                     - It's combines the default configuration with firmware in order to avoid the MIB conflicts.
	
	image/fw_98C_8814_8194.bin           - gateway image for 8198C+8814+8194
	image/fw_97F_8812BR_8367.bin         - gateway image for 8197F+8812BR+8367

	image/98C_8814_8194_nfjrom           - MP image for 8198C+8814+8194
	image/97F_8812BR_8367_nfjrom         - MP image for 8197F+8812BR+8367

	image/boot_98C.bin                   - bootloader for 8198C
	image/boot_8197FN_FS.bin             - bootloader for 8197FS
	image/boot_8197FS-8367RB.bin         - bootloader for 8197FS+8367

Environment
===========
  Fedora 9, Ubuntu 8.10/9.10 are recommended

Install the linux-3.10 sdk package
==================================
  1. Copy 'rtl819x.tar.gz' to a file directory on a Linux PC
  2. Type 'tar -zxvf rtl819x.tar.gz' to extract the package
 
Install the bootcode package
============================
  Type 'tar -zxvf rtl819x-SDK-v3.4.11C-bootcode-97F.tar.gz' to extract the package

build the linux kernel/rootfs/bootcode
======================================
  follow the INSTALL.txt file
