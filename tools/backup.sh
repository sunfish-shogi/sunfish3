#!/bin/bash

CURRENT_DIR=`pwd`
TARGET_DIR=`basename ${CURRENT_DIR}`
TARBALL=${TARGET_DIR}.tar.gz

echo "cleaning.."
make clean

cd ..
echo "write to ${TARBALL}"
echo "tar zcvf ${TARBALL} ${TARGET_DIR}"
tar zcvf ${TARBALL} ${TARGET_DIR}
