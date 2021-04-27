#!/bin/bash
echo "Building gSOAP"
GSOAP_INSTALL_DIR="./gsoap-2.8"
GSOAP_DIR=$GSOAP_INSTALL_DIR/gsoap

SOAPCPP2=$GSOAP_DIR/src/soapcpp2
WSDL2H=$GSOAP_DIR/wsdl/wsdl2h
GSOAP_CONFIGURE="--disable-c-locale"

# build
if [ ! -f $SOAPCPP2 ] || [ ! -f $WSDL2H ]; then \
        cd $GSOAP_INSTALL_DIR; \
        ./configure $GSOAP_CONFIGURE && \
        ls gsoap/ && \
        sed -i 's/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//\/\/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//g' ./gsoap/stdsoap2.cpp && \
        sed -i 's/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//\/\/err = strerror_r(err, soap->msgbuf, sizeof(soap->msgbuf)); \/\* XSI-compliant \*\//g' ./gsoap/stdsoap2.c && \
        make -j1; \
        cd ..;\
fi
