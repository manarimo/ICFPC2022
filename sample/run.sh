#!/bin/bash

echo $PROBLEM_ID
echo $PROBLEM_DIR
echo $OUTPUT_FILE

./a.out

echo "hoge" > $OUTPUT_FILE