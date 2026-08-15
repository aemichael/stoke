#!/bin/bash

TIME=$(TZ='America/Los_Angeles' date +%F-%H:%M:%S-%Z)
OUT_TOP="results"
TAG=""
SEP="-------------------###-------------------"

STOKE_ROOT=$(realpath ..)
NAME=run_stoke.sh

PREVIOUS=""
SYNTH_CONF="config/synth_permissive.conf"
TCS_CONF="config/testcase.conf"
EXTRA_SEARCH_ARGS=""

function usage
{
    echo "Usage: ./$NAME [ -h | --help (displays this message) ]
               TARGET                        Assembly file with the target instruction(s)
               [ -p | --previous <file>   ]  Assembly file with initial transform 
               [ -s | --synth-conf <file> ]  Config file for search (default: $SYNTH_CONF)
               [ -c | --tcs-conf <file>   ]  Config file for testcase generation (default: $TCS_CONF)
               [ -o | --out <dir>         ]  Output directory for results (default: $OUT_TOP)
               [ -t | --tag <tag>         ]  Tag to include in output path (optional)
               [ --search-args <args>     ]  Extra arguments to pass to STOKE search. Be careful not to duplicate
                                             args in the config file.
               [ --no-timestamp           ]  Omit timestamp from output path"
    exit 2
}

PARSED_ARGS=$(getopt -o "hp:s:c:o:t:" -l "help,previous:,synth-conf:,tcs-conf:,out:,tag:,search-args:,no-timestamp" -n run_stoke.sh -- "$@")

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
    '-p' | '--previous')
        PREVIOUS=$2
        shift 2
        continue
        ;;
    '-s' | '--synth-conf')
        SYNTH_CONF=$2
        shift 2
        continue
        ;;
    '-c' | '--tcs-conf')
        TCS_CONF=$2
        shift 2
        continue
        ;;
    '-o' | '--out')
        OUT_TOP=$2
        shift 2
        continue
        ;;
    '-t' | '--tag')
        TAG=$2
        shift 2
        continue
        ;;
	'--search-args')
	    EXTRA_SEARCH_ARGS=$2
	    shift 2
	    continue
	    ;;
    '--no-timestamp')
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

TARGET=$1
if [[ -z "$TARGET" ]]; then
    echo "Error: Missing required argument TARGET"
    usage
fi

# Sanity checks
if [[ ! -f $TARGET ]]; then
    echo "File $TARGET not found"
    usage
elif [[ ! -z $PREVIOUS && ! -f $PREVIOUS ]]; then
    echo "File $PREVIOUS not found"
    usage
elif [[ ! -f $SYNTH_CONF ]]; then
    echo "File $SYNTH_CONF not found"
    usage
elif [[ ! -f $TCS_CONF ]]; then
    echo "File $TCS_CONF not found"
    usage
fi

# Set up output directory, copy important files
OUTPUT_DIR="$OUT_TOP/$TAG/$TIME"
mkdir -p $OUTPUT_DIR

cp $NAME $OUTPUT_DIR
cp $TARGET "$OUTPUT_DIR/target.s"
cp $SYNTH_CONF $OUTPUT_DIR
cp $TCS_CONF $OUTPUT_DIR
cp $STOKE_ROOT/src/validator/leakage_ranges.h $OUTPUT_DIR

PREVIOUS_ARG="--previous $PREVIOUS"
if [[ -z $PREVIOUS ]]; then
    PREVIOUS_ARG=""
else
    cp $PREVIOUS "$OUTPUT_DIR/previous.s"
fi

LOG_FILE="$OUTPUT_DIR/run_stoke.log"
TARGET_SHORT=$(basename $TARGET)

# Generate testcases
TCS_FILE="$OUTPUT_DIR/tcs"

echo "[$NAME - $TARGET_SHORT] Generating testcases..." | tee $LOG_FILE

echo ">> $STOKE_ROOT/bin/tcgen_leakage --target $TARGET --output $TCS_FILE --config $TCS_CONF" > $LOG_FILE
echo "" >> $LOG_FILE

/home/stoke/stoke/bin/tcgen_leakage --target $TARGET --output $TCS_FILE --config $TCS_CONF &>> $LOG_FILE
if [[ $? -ne 0 ]]; then
    echo "[$NAME - $TARGET_SHORT] Testcase generation failed. See $LOG_FILE for details"
    exit 1
fi

# Run synthesis

RESULT_FILE="$OUTPUT_DIR/synth_result.s"
RESULT_DIR=$OUTPUT_DIR/results
COST_FILE="$OUTPUT_DIR/costs.csv"
mkdir -p $RESULT_DIR

echo "[$NAME - $TARGET_SHORT] Running synthesis. Logging output to $LOG_FILE" | tee -a $LOG_FILE

echo -e "\n$SEP\n" >> $LOG_FILE
echo ">> $STOKE_ROOT/bin/stoke_search --out $RESULT_FILE --cost_output $COST_FILE \
  --results $RESULT_DIR --target $TARGET --init previous $PREVIOUS_ARG \
  --testcases $TCS_FILE --config $SYNTH_CONF $EXTRA_SEARCH_ARGS" >> $LOG_FILE
echo "" >> $LOG_FILE

SECONDS=0

$STOKE_ROOT/bin/stoke_search --out $RESULT_FILE --cost_output $COST_FILE \
    --results $RESULT_DIR --target $TARGET --init previous $PREVIOUS_ARG \
    --testcases $TCS_FILE --config $SYNTH_CONF $EXTRA_SEARCH_ARGS &>> $LOG_FILE

EXIT_CODE=$?
LAST_LINE=$(tail -n 1 $LOG_FILE)

# Record duration
DURATION=$(grep "Total search time" $LOG_FILE |  awk ""' {print substr($4, 1, index($4, "s") - 1)}')
echo $DURATION > $OUTPUT_DIR/seconds.txt

if [ "$LAST_LINE" == "FATAL ERROR: Search terminated unsuccessfully; unable to discover a new rewrite!" ]; then
    if [[ -d $RESULT_DIR && $(ls $RESULT_DIR | wc -l) -ne 0 ]]; then 
        echo "[$NAME - $TARGET_SHORT] WARNING: Search reported failure, but results found in $RESULT_DIR. Using latest verified result"
        cp $(realpath $(ls $RESULT_DIR | tail -1)) $RESULT_FILE
    else
        echo "[$NAME - $TARGET_SHORT] FAILURE: Search unsuccessful (target: $TARGET, previous: $PREVIOUS)"
    fi
elif [[ $EXIT_CODE -ne 0 ]]; then
    echo "[$NAME - $TARGET_SHORT] ERROR: Unrecognized search error. See $LOG_FILE"
    exit 1
else
    echo "[$NAME - $TARGET_SHORT] SUCCESS: Search finished with transform in $RESULT_FILE"
fi
