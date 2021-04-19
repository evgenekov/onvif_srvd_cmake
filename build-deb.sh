#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

CURRENT_DIR=`pwd`
OUT_DIR=$CURRENT_DIR
if [[ $# -eq 1 ]]; then
    OUT_DIR=`readlink -f $1`
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
CMAKE_ROOT_DIR=$SCRIPT_DIR/..

cd $SCRIPT_DIR
SOFTWARE_VERSION=`./version.sh`

BUILD_DIR=$OUT_DIR
DEB_DIR=$BUILD_DIR/onvif-srvd-$SOFTWARE_VERSION
ETC_DIR=$DEB_DIR/etc
LIB_DIR=$DEB_DIR/lib
BIN_DIR=$DEB_DIR/usr/sbin


WSDD_DIR=$BUILD_DIR/wsdd
mkdir -p $OUT_DIR $BUILD_DIR

echo "Build Onvif_Srvd"
make clean
make WSSE_ON=1

echo "Build WSDD"
cd $OUT_DIR/wsdd
if [ ! -d $OUT_DIR/wsdd/SDK ]
then
    echo "No gSOAP Files in repo, copy and build"
    cp -r $OUT_DIR/SDK $OUT_DIR/wsdd
    cp -r $OUT_DIR/gsoap-2.8 $OUT_DIR/wsdd
    make
else
    echo "Files already there, clean up wsdd directory and send files again"
    rm -r $OUT_DIR/wsdd/SDK
    rm -r $OUT_DIR/wsdd/gsoap-2.8
    make clean
    cp -r $OUT_DIR/SDK $OUT_DIR/wsdd
    cp -r $OUT_DIR/gsoap-2.8 $OUT_DIR/wsdd
    make
fi

echo "Create .deb Package"
cd $OUT_DIR
mkdir -p $DEB_DIR/DEBIAN $BIN_DIR $ETC_DIR/onvif_srvd/ $LIB_DIR/systemd/system/

echo "Package: onvif-srvd" > $DEB_DIR/DEBIAN/control
echo "Version:" $SOFTWARE_VERSION >> $DEB_DIR/DEBIAN/control
echo "Maintainer: Niall Mullins <niall.mullins@rinicom.com>" >> $DEB_DIR/DEBIAN/control
echo "Architecture: amd64" >> $DEB_DIR/DEBIAN/control
echo "Description: Onvif Daemon" >> $DEB_DIR/DEBIAN/control
echo "Depends: flex,bison,byacc,make,m4,openssl,libssl-dev,zlib1g-dev,libcrypto++8" >> $DEB_DIR/DEBIAN/control

echo "#!/bin/bash" > $DEB_DIR/DEBIAN/postinst
echo "ldconfig" >> $DEB_DIR/DEBIAN/postinst

echo "echo \"Please, select a network interface:\""  >> $DEB_DIR/DEBIAN/postinst
echo "cd /sys/class/net && select foo in *; do interface=\$foo;  break; done"  >> $DEB_DIR/DEBIAN/postinst
echo "echo \$interface selected"  >> $DEB_DIR/DEBIAN/postinst
echo "sed -i \"s/eth0/\$interface/g\" /etc/onvif_srvd/config.cfg"  >> $DEB_DIR/DEBIAN/postinst
echo "sed -i \"s/eth0/\$interface/g\" /lib/systemd/system/wsdd.service"  >> $DEB_DIR/DEBIAN/postinst
echo "systemctl daemon-reload" >> $DEB_DIR/DEBIAN/postinst
echo "systemctl enable onvif_srvd.service" >> $DEB_DIR/DEBIAN/postinst
echo "systemctl start onvif_srvd.service" >> $DEB_DIR/DEBIAN/postinst
echo "systemctl enable wsdd.service" >> $DEB_DIR/DEBIAN/postinst
echo "systemctl start wsdd.service" >> $DEB_DIR/DEBIAN/postinst
chmod +x $DEB_DIR/DEBIAN/postinst

echo "#!/bin/bash" > $DEB_DIR/DEBIAN/postrm
echo "systemctl stop onvif_srvd.service" >> $DEB_DIR/DEBIAN/postrm
echo "systemctl stop wsdd.service" >> $DEB_DIR/DEBIAN/postrm
echo "systemctl daemon-reload" >> $DEB_DIR/DEBIAN/postrm
chmod +x $DEB_DIR/DEBIAN/postrm

cp -a $OUT_DIR/config.cfg $ETC_DIR/onvif_srvd/
cp -a $OUT_DIR/onvif_srvd $BIN_DIR/
cp -a $OUT_DIR/start_scripts/*.service $LIB_DIR/systemd/system/

cp -a $OUT_DIR/wsdd/wsdd $BIN_DIR/
cp -a $OUT_DIR/wsdd/start_scripts/*.service $LIB_DIR/systemd/system/


dpkg-deb --build $DEB_DIR
cd $CURRENT_DIR

echo "Done! You may remove: "
echo $DEB_DIR

