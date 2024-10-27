#!bin/bash

DIR_NAME="lab1_build"
EXE_NAME="lab1"
PID_PATH="/var/run/lab1_daemon.pid"


sudo touch $PID_PATH; sudo chmod 0666 $PID_PATH

mkdir $DIR_NAME
cd $DIR_NAME
cmake ..; make
cd ..
cp $DIR_NAME/$EXE_NAME .
rm -r $DIR_NAME
