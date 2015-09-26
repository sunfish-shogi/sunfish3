#!/bin/bash

if [ $# -lt 3 ]; then
	echo "Usage: bash ${0##*/} SSH_KEY.pem EC2_HOST KIFU.tar.gz"
	exit 1
fi

SSH_KEY=$1
EC2_HOST=$2
KIFU_TARBALL=$3
KIFU_TARBALL_NAME=`basename ${KIFU_TARBALL}`

TARGET_BRANCH=master

YUM_PACKAGES="git gcc-c++ cmake"
SUNFISH_REPO="https://github.com/sunfish-shogi/sunfish3.git"
SUNFISH_DIR="sunfish3"
CONF_FILE="learn.conf"

scp -i ${SSH_KEY} ${KIFU_TARBALL} ec2-user@${EC2_HOST}:~/
scp -i ${SSH_KEY} ${CONF_FILE} ec2-user@${EC2_HOST}:~/

ssh -i ${SSH_KEY} -t -t ec2-user@${EC2_HOST} <<EOF
sudo yum install -y ${YUM_PACKAGES}
git clone --branch ${TARGET_BRANCH} --depth 1 ${SUNFISH_REPO} ${SUNFISH_DIR}
cd ${SUNFISH_DIR}
make learn -j
tar zxf ~/${KIFU_TARBALL_NAME}
cp ~/${CONF_FILE} ./${CONF_FILE}
./sunfish -l > /dev/null 2>&1 &
exit
EOF
