#!/bin/bash

for f in `ls ../problem/plaintext`
do
	if [[ $f =~ ^(.*)\.txt$ ]]
	then
		./a.out < ../problem/plaintext/$f > ${BASH_REMATCH[1]}.isl
	fi
done
