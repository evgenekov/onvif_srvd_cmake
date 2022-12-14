#!/bin/bash
echo "Building gSOAP"
GSOAP_INSTALL_DIR=$1
GSOAP_DIR=$GSOAP_INSTALL_DIR/gsoap
GSOAP_IMPORT_DIR=$GSOAP_DIR/import
PATCH_DIR=$2

SOAPCPP2=$GSOAP_DIR/src/soapcpp2
WSDL2H=$GSOAP_DIR/wsdl/wsdl2h
GSOAP_CONFIGURE="--disable-c-locale"
GENERATED_DIR="./generated"

echo "Apply Patch"
cd $GSOAP_DIR && patch -p0 < $PATCH_DIR/stdsoapError.patch
cd ..

# build
if [ ! -f $SOAPCPP2 ] || [ ! -f $WSDL2H ]; then \
        echo "In Build Stage"
        cd $GSOAP_INSTALL_DIR && \
        ./configure $GSOAP_CONFIGURE && \
        ls gsoap/ && \
        sed -i 's/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//\/\/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//g' ./gsoap/stdsoap2.c && \
        make -j4; \
        cd ..;\
fi

echo "Generating Files"
for file in wsdl/*.wsdl
do
    WSDL_FILES="${WSDL_FILES} $file"
done

for file in wsdl/*.xsd
do
    WSDL_FILES="${WSDL_FILES} $file"
done

echo "Create Generated Directory"
mkdir -p $GENERATED_DIR
echo "Generate Files"
$WSDL2H -d -t ./wsdl/typemap.dat -o $GENERATED_DIR/onvif.h $WSDL_FILES
echo '#import "wsse.h" ' >> $GENERATED_DIR/onvif.h
echo "First Half"
$SOAPCPP2 -j -L -x -S -d $GENERATED_DIR -I$GSOAP_DIR:$GSOAP_IMPORT_DIR $GENERATED_DIR/onvif.h

echo "Apply Patch"
cd $GSOAP_DIR && patch -p0 < $PATCH_DIR/stdsoapLoopFix.patch
cd ..

