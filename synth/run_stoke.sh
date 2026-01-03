#!/bin/bash

TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
MO=$(TZ='America/Los_Angeles' date +%Y-%m)
DIR="results"
TAG=""
SEP="-------------------###-------------------"

TARGET=$1
PREVIOUS=$2
SYNTH_CONF="config/synthesize.conf"
TCS_CONF="config/testcase.conf"

function usage
{
    echo "Usage: ./run_stoke.sh [ -h | --help (displays this message) ]
			   target <assembly file containing the target instruction(s) for equivalence>
			   previous <assembly file containing an existing transform to optimize or repair> 
			   [ -s | --synthesis-conf <configuration file for search and verification> ]
			   [ -c | --testcase-conf <configuration file for testcase generation> ]
			   [ -o | --output <outer directory for output files> ]
			   [ -t | --tag <tag to prepend to timestamped directory name> ]"
    exit 2
}

PARSED_ARGS=$(getopt -o "hs:c:o:t:" -l "help,synthesis-conf:,testcase-conf:,output:,tag:" -n run_stoke.sh -- "$@")

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
OUTPUT_DIR="$DIR/$TAG_$TIME"
mkdir -p $OUTPUT_DIR

cp run_stoke.sh $OUTPUT_DIR
cp $TARGET "$OUTPUT_DIR/target_$TARGET"
cp $PREVIOUS "$OUTPUT_DIR/previous_$PREVIOUS"
cp $SYNTH_CONF $OUTPUT_DIR
cp $TCS_CONF $OUTPUT_DIR

LOG_FILE="$OUTPUT_DIR/run_stoke.log"

# Generate testcases
TCS_FILE="$OUTPUT_DIR/tcs"

echo "Generating testcases..."

echo "stoke_tcgen --target $TARGET --output $TCS_FILE --config $TCS_CONF" > $LOG_FILE
echo "" >> $LOG_FILE

stoke_tcgen --target $TARGET --output $TCS_FILE --config $TCS_CONF &>> $LOG_FILE

# Run synthesis

RESULT_FILE="$OUTPUT_DIR/synth_result.s"

echo "Running synthesis..."
echo "See $LOG_FILE for running output"

echo "" >> $LOG_FILE
echo "$SEP" >> $LOG_FILE
echo "" >> $LOG_FILE
echo "stoke_search --out $RESULT_FILE --target $TARGET --init previous --previous $PREVIOUS --testcases $TCS_FILE --config $SYNTH_CONF" >> $LOG_FILE
echo "" >> $LOG_FILE

stoke_search --out $RESULT_FILE --target $TARGET --init previous --previous $PREVIOUS --testcases $TCS_FILE --config $SYNTH_CONF &>> $LOG_FILE

echo "Done"
