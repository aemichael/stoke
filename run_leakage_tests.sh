#!/bin/bash

TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
LOG_DIR_TOP=leakage_test_logs
LOG_DIR=$LOG_DIR_TOP/$TIME
VAL_LOG_FILE=$LOG_DIR/validator_tests.log
COST_LOG_FILE=$LOG_DIR/cost_tests.log

if [ ! -d "$LOG_DIR_TOP" ]; then
	mkdir $LOG_DIR_TOP
fi

mkdir $LOG_DIR

echo "Running leakage validator tests: stoke_test --gtest_filter=*LeakageValidator* &> $VAL_LOG_FILE"

stoke_test --gtest_filter=*LeakageValidator* &> $VAL_LOG_FILE

echo "Running leakage cost tests: stoke_test --gtest_filter=*LeakageCost* &> $COST_LOG_FILE"

stoke_test --gtest_filter=*LeakageCost* &> $COST_LOG_FILE

echo "Done"
