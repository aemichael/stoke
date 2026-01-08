#!/bin/bash

DIR="results"
TAG="untagged"
JOBS=1

INST_REX=$1
TARGET_DIR=$2
PREV_DIR=$3
SYNTH_CONF="config/synthesize.conf"
TCS_CONF="config/testcase.conf"
TS_FLAG=""
SKIP_NO_PREV=0

function usage
{
    echo "Usage: ./run_all_matching.sh [ -h | --help (displays this message) ]
			   inst <awk regex pattern for instruction to synthesize. Use '.' for all>
			   target_dir (directory containing target assembly for each target instruction)
			   previous_dir (directory containing attempted assembly transforms for each target instruction)
			   [ -j | --jobs <num jobs (default: 1)> ]
			   [ -s | --synthesis-conf <configuration file for search and verification (default: config/synthesize.conf)> ]
			   [ -c | --testcase-conf <configuration file for testcase generation (default: config/testcase.conf)> ]
			   [ -o | --output <outer directory for output files (default: results)> ]
			   [ -t | --tag <tag to prepend to timestamped directory name (default: untagged)> ]
			   [ -n | --no-timestamp (omit timestamp from output directory path) ]
			   [ -k | --skip-missing-previous (do not attempt to synthesize targets that lack a previous transform) ]"
    exit 2
}

PARSED_ARGS=$(getopt -o "hj:s:c:o:t:nk" -l "help,jobs:,synthesis-conf:,testcase-conf:,output:,tag:,no-timestamp,skip-missing-previous" -n run_all_matching.sh -- "$@")

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
	'-j' | '--jobs')
	    JOBS=$2
	    shift 2
	    continue
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
	    TS_FLAG="-n"
	    shift 1
	    continue
	    ;;
	'-k' | '--skip-missing-previous')
	    SKIP_NO_PREV=1
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

# Parse instruction regular expression
case $INST_REX in
'byte')
	INST_REX="B-"
	;;
'word')
	INST_REX="W-"
	;;
'long')
	INST_REX="L-"
	;;
'quad')
	INST_REX="Q-"
	;;
'arith')
	INST_REX="ADD/||/SUB"
	;;
'bitwise')
	INST_REX="AND/||/OR/||/XOR"
	;;
'shifts')
	INST_REX="SHL/||/SHR/||/SAR"
	;;
'shifts-1')
	INST_REX="SHL.*-ONE/||/SHR.*-ONE/||/SAR.*-ONE"
	;;
'shifts-cl')
	INST_REX="SHL.*-CL/||/SHR.*-CL/||/SAR.*-CL"
	;;
*)
	;;
esac

if [ ! -d "$DIR" ]; then
    mkdir $DIR
fi

INST_LIST=$(ls $TARGET_DIR | awk "/$INST_REX/"' {print substr($1, 1, index($1, ".") - 1)}' | sort | uniq)

echo "Instructions matched: "
echo $INST_LIST
ERR=0

# Run stoke jobs
for inst in $INST_LIST
do
    while (( $(jobs -r | wc -l) >= JOBS )); do
        wait -n
		if [[ $? -ne 0 ]]; then
			ERR=1
		fi
    done
    target=$TARGET_DIR/$inst.s
    previous=$PREV_DIR/$inst.s
    if [[ ! -f $target ]]; then
        echo "Target not found for $inst; skipping"
    else if [[ ! -f $previous ]]; then
		if [[ $SKIP_NO_PREV -ne 0 ]]; then
			echo "Previous transform not found for $inst; skipping (--skip-missing-previous passed)"
		else
			echo "Previous transform not found for $inst. Treating target as previous transform"
			previous=$target
			echo "Starting $inst job: ./run_stoke.sh $target $previous -s $SYNTH_CONF -c $TCS_CONF -o $DIR -t $TAG/$inst $TS_FLAG"
			./run_stoke.sh $target $previous -s $SYNTH_CONF -c $TCS_CONF -o $DIR -t $TAG/$inst $TS_FLAG &
		fi
    else
        echo "Starting $inst job: ./run_stoke.sh $target $previous -s $SYNTH_CONF -c $TCS_CONF -o $DIR -t $TAG/$inst $TS_FLAG"
        ./run_stoke.sh $target $previous -s $SYNTH_CONF -c $TCS_CONF -o $DIR -t $TAG/$inst $TS_FLAG &
    fi; fi
done

echo "Waiting for jobs to finish..."
while (( $(jobs -r | wc -l) > 0 )); do
    wait -n
	if [[ $? -ne 0 ]]; then
		ERR=1
	fi
done

if [[ $ERR -ne 0 ]]; then
	echo "Error occurred during jobs. Check log for details"
	exit 1
fi
echo "Jobs finished successfully"
