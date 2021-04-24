# kernel-install

Install current stable version of Linux kernel from kernel.org

## Description

Installs current linux stable kernel source code into given subdirectory or `~/src/linux-stable` by default.

The script by default downloads the current stable kernel source to the `~/src/linux-stable` directory using the wget command. The -g option uses git clone for downloading the current stable source from kernel.org. 

This script uses gpg verification to ensure the contents of the kernel are safe to install.

## Specifications
- OS: RHEL 8.2 (Ootpa)
- requires root access
- 10 GB disk space
- 1 GB VRAM

## Exapmles
	#    1. install kernel in ~/src (default)
	#                ./kernel-install.sh
	#
	#    2. get current kernel version
	#                ./kernel-install.sh -v
	#
	#    3. install kernel in /usr/src/kernels with git clone option
	#                ./kernel-install.sh -g -D /usr/src/kernels

## Options

	-g 
		git clone source from kernel.org instead of wget or curl
	
	-v 
		Version of new stable kernel but does not install it.
	
	-h 
		Help should display options with examples.
	
	-D Subdir
		Subdir is the fullpath of the downloaded source code directory.

## Return Values

	0 	On success
	1 	On failure
