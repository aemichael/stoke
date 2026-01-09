#!/bin/bash

TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
DIR="results"
TAG="untagged"
SEP="-------------------###-------------------"

TARGET=$1
PREVIOUS=$2
SYNTH_CONF="config/synthesize.conf"
TCS_CONF="config/testcase.conf"

function usage
{
    echo "Usage: ./run_stoke.sh [ -h | --help (displays this message) ]
			   target (assembly file containing the target instruction(s) for equivalence)
			   previous (assembly file containing an existing transform to optimize or repair)
			   [ -s | --synthesis-conf <configuration file for search and verification (default: config/synthesize.conf)> ]
			   [ -c | --testcase-conf <configuration file for testcase generation (default: config/testcase.conf)> ]
			   [ -o | --output <outer directory for output files (default: results)> ]
			   [ -t | --tag <tag to prepend to timestamped directory name (default: untagged)> ]
			   [ -n | --no-timestamp (omit timestamp from output directory path) ]"
    exit 2
}

PARSED_ARGS=$(getopt -o "hs:c:o:t:n" -l "help,synthesis-conf:,testcase-conf:,output:,tag:,no-timestamp" -n run_stoke.sh -- "$@")

if [[ $? -ne 0 ]]; then
       echo "Error parsing args"
       usage
fi

eval set -- "$PARSED_ARGS"
unset PARSED_ARGS

while true; do
    case "$1" in
	'-h' | '--help')
	    usage
	    ;;
	'-s' | '--synthesis-conf')
	    SYNTH_CONF=$2
	    shift 2
	    continue
	    ;;
	'-c' | '--testcase-conf')
	    TCS_CONF=$2
	    shift 2
	    continue
	    ;;
	'-o' | '--output')
	    DIR=$2
	    shift 2
	    continue
	    ;;
	'-t' | '--tag')
	    TAG=$2
	    shift 2
	    continue
	    ;;
	'-n' | '--no-timestamp')
	    TIME=""
	    shift 1
	    continue
	    ;;
	'--')
	    shift
	    break
	    ;;
	*)
	    echo "Unknown option $1"
	    usage
	    ;;
    esac
    shift
done

# Set up output directory, copy important files
OUTPUT_DIR="$DIR/$TAG/$TIME"
mkdir -p $OUTPUT_DIR

cp run_stoke.sh $OUTPUT_DIR
cp $TARGET "$OUTPUT_DIR/target.s"
cp $PREVIOUS "$OUTPUT_DIR/previous.s"
cp $SYNTH_CONF $OUTPUT_DIR
cp $TCS_CONF $OUTPUT_DIR

LOG_FILE="$OUTPUT_DIR/run_stoke.log"

# Generate testcases
TCS_FILE="$OUTPUT_DIR/tcs"

echo "Generating testcases..." | tee $LOG_FILE

echo "/home/stoke/stoke/bin/tcgen_leakage --target $TARGET --output $TCS_FILE --config $TCS_CONF" > $LOG_FILE
echo "" >> $LOG_FILE

/home/stoke/stoke/bin/tcgen_leakage --target $TARGET --output $TCS_FILE --config $TCS_CONF &>> $LOG_FILE
if [[ $? -ne 0 ]]; then
	echo "Error in testcase generation. See $LOG_FILE for details"
	exit 1
fi

# Run synthesis

RESULT_FILE="$OUTPUT_DIR/synth_result.s"
RESULT_DIR=$OUTPUT_DIR/results
mkdir -p $RESULT_DIR

echo "Running synthesis. Logging output to $LOG_FILE" | tee -a $LOG_FILE

echo -e "\n$SEP\n" >> $LOG_FILE
echo "/home/stoke/stoke/bin/stoke_search --out $RESULT_FILE --results $RESULT_DIR --target $TARGET --init previous --previous $PREVIOUS --testcases $TCS_FILE --config $SYNTH_CONF" >> $LOG_FILE
echo "" >> $LOG_FILE

/home/stoke/stoke/bin/stoke_search --out $RESULT_FILE --results $RESULT_DIR --target $TARGET --init previous --previous $PREVIOUS --testcases $TCS_FILE --config $SYNTH_CONF &>> $LOG_FILE
if [[ $? -ne 0 ]]; then
	if [[ -d $RESULT_DIR && $(ls $RESULT_DIR | wc -l) -ne 0 ]]; then 
		echo "Search reported failure. Results found in $RESULT_DIR. Copying latest to $RESULT_FILE"
		latest=$(ls $RESULT_DIR -1 | tail -1)
		cp $RESULT_DIR/$latest $RESULT_FILE
	else
		echo "Error in search. See $LOG_FILE for details"
		exit 1
	fi
else
	echo "Search finished successfully with transform in $RESULT_FILE" 
fi
