#!/bin/sh

DIR=`pwd`

make clean

rsync -avz --delete --exclude 'build.sh' ${DIR} pi@192.168.137.245:/home/pi/

ssh pi@192.168.137.245 'cd /home/pi/cmpe245/ && make all'
