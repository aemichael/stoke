#!/bin/bash

TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
LOG_DIR_TOP=leakage_test_logs
LOG_DIR=$LOG_DIR_TOP/$TIME
LOG_FILE=$LOG_DIR/leakage_test.log

if [ ! -d "$LOG_DIR_TOP" ]; then
	mkdir $LOG_DIR_TOP
fi

mkdir $LOG_DIR

echo "Running tests:\nstoke_test --gtest_filter=*LeakageTest* &> $LOG_FILE"

stoke_test --gtest_filter=*LeakageTest* &> $LOG_FILE

echo "Done"
