#!/bin/bash

LOG_DIR_TOP=search_logs
TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
LOG_DIR=$LOG_DIR_TOP/$TIME
SEARCH_LOG=synthesis.log

if [ ! -d "$LOG_DIR_TOP" ]; then
	mkdir $LOG_DIR_TOP
fi
mkdir $LOG_DIR

cp synthesize.conf $LOG_DIR
cp Makefile $LOG_DIR
cp target.s $LOG_DIR
cp main.cc $LOG_DIR

make clean
make orig extract
cp -r bins $LOG_DIR

make testcase_leakage
cp -r tcs $LOG_DIR

START_TIME=$(date +%s)
make synthesize &> $LOG_DIR/$SEARCH_LOG
END_TIME=$(date +%s)

ELAPSED=$(($END_TIME - $START_TIME))
echo "Time: $ELAPSED" >> $LOG_DIR/$SEARCH_LOG
