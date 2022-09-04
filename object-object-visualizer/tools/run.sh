#!/bin/bash

for f in `ls ../../problem/original`
do
	if [[ $f =~ ^(.*)\.png$ ]]
	then
		npm run meta -- --problemId ${BASH_REMATCH[1]} --batchName kawatea-dp-meta --command ../../kawatea/a.out --rotator=kawatea-dp-special
	fi
done
