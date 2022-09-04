#!/bin/bash

BATCH_NAME=$1
COMMAND=$2

ALL_RULE="all: "
for f in `ls ../../problem/original`
do
	if [[ $f =~ ^(.*)\.png$ ]]
	then
	  ID=${BASH_REMATCH[1]}
	  echo "run${ID}:"
	  echo -e "\tnpm run meta -- --problemId ${ID} --batchName ${BATCH_NAME} --rotator=kawatea-dp-special --command '${COMMAND}'"
	  echo ""
	  ALL_RULE="$ALL_RULE run${ID} "
	fi
done

echo $ALL_RULE
