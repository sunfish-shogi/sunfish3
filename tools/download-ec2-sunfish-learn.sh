#!/bin/bash

if [ $# -lt 2 ]; then
	echo "Usage: bash ${0##*/} SSH_KEY.pem EC2_HOST"
	exit 1
fi

SSH_KEY=$1
EC2_HOST=$2

SUNFISH_DIR="sunfish3"
MATERIAL_TXT="material.txt"
EVAL_BIN="eval.bin"
EVAL_BIN_GZ="${EVAL_BIN}.gz"
LEAN_LOG="learn.log"

ssh -i ${SSH_KEY} -t -t ec2-user@${EC2_HOST} <<EOF
cd ${SUNFISH_DIR}
gzip -c ${EVAL_BIN} > ${EVAL_BIN_GZ}
exit
EOF

scp -i ${SSH_KEY} ec2-user@${EC2_HOST}:~/${SUNFISH_DIR}/${MATERIAL_TXT} ${MATERIAL_TXT}
scp -i ${SSH_KEY} ec2-user@${EC2_HOST}:~/${SUNFISH_DIR}/${EVAL_BIN_GZ} ${EVAL_BIN_GZ}
scp -i ${SSH_KEY} ec2-user@${EC2_HOST}:~/${SUNFISH_DIR}/${LEAN_LOG} ${LEAN_LOG}
