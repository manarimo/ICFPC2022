#!/bin/bash

aws lambda invoke --function-name icfpc2022-runner --cli-binary-format raw-in-base64-out --payload '{\"executablePath\": \"ai/a.out\"}' a.txt