# kernel-install.sh
# Hensley, Robert
#
# Description
#
#       Installs current linux stable kernel source code into given subdirectory
#       or ~/src/linux-stable by default.
#   	
#	    The script by default downloads the current stable kernel
#       source to the ~/src/linux-stable directory using the wget command.
#
#       The -g option uses git clone for downloading the current stable source from kernel.org. 
#       
#       This script uses gpg verification to ensure the contents of the kernel
#       are safe to install.
#
#
# Specifications
#       - OS: RHEL 8.2 (Ootpa)
#       - requires root access
#       - 10 GB disk space
#       - 1 GB VRAM
# 
# Exapmles
#    1. install kernel in ~/src (default)
#                ./kernel-install.sh
#
#    2. get current kernel version
#                ./kernel-install.sh -v
#
#    3. install kernel in /usr/src/kernels with git clone option
#                ./kernel-install.sh -g -D /usr/src/kernels

#!/bin/bash

SRC_PATH="src"
gitFLAG=false

kernel=$(curl -s https://www.kernel.org/ \
        | sed -n '/stable:/,/longterm:/p' \
        | grep -oP '(?<=<strong>).*?(?=</strong>)')

kernelV=$(echo $kernel | grep -oP '[0-9]+' | head -1)

# -- Command Line Arguments --
while getopts "vhgD:" OPTION
do
	case $OPTION in
        \?)
			exit 1
			;;
        v)
			echo $kernel
            exit 0
			;;
        h)
            echo "
OPTIONS

	-g
                git clone source from kernel.org instead of wget or curl

	-v
                Version of new stable kernel but does not install it.

	-h
                Help should display options with examples.

	-D Subdir
                Subdir is the fullpath of the downloaded source code directory.

EXAMPLES

        1. install kernel in ~/src (default)
                ./kernel-install.sh

        2. get current kernel version
                ./kernel-install.sh -v

        3. install kernel in /usr/src/kernels with git clone option
                ./kernel-install.sh -g -D /usr/src/kernel
"
            exit 0
            ;;
        g)
            gitFLAG=true
            ;;
		D)
			SRC_PATH=$OPTARG
			;;
	esac
done

# -- update existing kernel --
yes | sudo dnf update kernel

# -- install pre-req packages --
yes | sudo yum install wget
yes | sudo dnf group install "Development Tools"
yes | sudo dnf install ncurses-devel bison flex elfutils-libelf-devel openssl-devel

# -- ccache install --

if ! command -v ccache &> /dev/null
then 
    mkdir ~/src 
    cd ~/src 

    # download src / sign
    wget https://github.com/ccache/ccache/releases/download/v3.7.11/ccache-3.7.11.tar.xz
    wget https://github.com/ccache/ccache/releases/download/v3.7.11/ccache-3.7.11.tar.xz.asc

    # verifying packcage
    gpg --locate-keys joel@debian.org

    keyid=$(gpg --list-keys joel@debian.org \
            | sed '2q;d' \
            | grep -oP '\S+')

    echo "${keyid}:6:" | gpg --import-ownertrust

    gpg --verify ccache-3.7.11.tar.xz.asc ccache-3.7.11.tar.xz

    if ! [ $? -eq 0 ]
    then 
        echo "gpg failed to verify ccache"
        exit 1
    fi

    # unpack source code
    tar -xf ccache-3.7.11.tar.xz
    cd ccache-3.7.11

    # installation
    ./configure
    sudo make
    sudo make install
fi

# -- download kernel source --

cd ~
sudo mkdir -p -- ${SRC_PATH}
sudo chmod 777 ${SRC_PATH}
cd ${SRC_PATH}

if $gitFLAG
then
    # -- git clone method --
    git clone --depth 1 --single-branch --branch v${kernel} \
        git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git 
    cd linux-stable
else
    # -- wget method -- 
    echo "wget kernel"
    wget "https://cdn.kernel.org/pub/linux/kernel/v${kernelV}.x/linux-${kernel}.tar.xz"
    wget "https://cdn.kernel.org/pub/linux/kernel/v${kernelV}.x/linux-${kernel}.tar.sign"

    # verify package
    gpg --locate-keys gregkh@kernel.org
    keyid=$(gpg --list-keys gregkh@kernel.org \
		| sed '2q;d' \
		| grep -oP '\S+')

    echo "${keyid}:6:" | gpg --import-ownertrust
    xz -cd linux-${kernel}.tar.xz | gpg2 --verify linux-${kernel}.tar.sign -

    if ! [ $? -eq 0 ]
    then 
        echo "gpg failed to verify linux source"
        exit 1
    fi

    tar -xf linux-${kernel}.tar.xz
    cd linux-${kernel}
fi 

# -- kernel installation --
echo "-- kernel installation --"

cp -v /boot/config-$(uname -r) .config

yes "" | make localmodconfig

# allocating memory for install
sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

scripts/config --disable DEBUG_INFO # gets rid of dwarves error (maybe)
yes "" | make -j $(nproc)

sudo make bzImage
sudo make modules
sudo make modules_install

sudo rm /etc/dracut.conf.d/xen.conf # remove this not needed file (see FAQ)
sudo make install

# this sets the boot order properly
sudo grub2-mkconfig

sudo reboot
