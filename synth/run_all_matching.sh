#!/bin/bash

NAME=run_all_matching.sh

TOP_DIR="results"
TAG=""
JOBS=4

PREV_DIR=""
SYNTH_CONF="config/synth_permissive.conf"
TCS_CONF="config/testcase.conf"
TS_FLAG=""
SKIP_NO_PREV=1

function usage
{
    echo "Usage: ./$NAME [ -h | --help (displays this message) ]
               INST_REGEX                    awk regex pattern for target instruction(s)
               TARGETS                       Directory with target instruction assembly
               [ -p | --previous <dir>    ]  Directory with initial rewrites (optional)
               [ -s | --synth-conf <file> ]  STOKE search config file (default: $SYNTH_CONF)
               [ -c | --tcs-conf <file>   ]  STOKE testcase config file (default: $TCS_CONF)
               [ -j | --jobs <num>        ]  Max parallel jobs (default: $JOBS)
               [ -o | --out <dir>         ]  Directory for results and logs (default: $TOP_DIR)
               [ -t | --tag <tag>         ]  Tag to include in output path (optional)
               [ --no-timestamp           ]  Exclude timestamp from results path
               [ --allow-missing-previous ]  Use with --previous to run jobs for instructions without a provided
                                             previous transform. Default behavior is to skip instructions without
                                             a previous transform when --previous is passed."
    exit 2
}

PARSED_ARGS=$(getopt -o "hp:s:c:j:o:t:" -l "help,previous:,synth-conf:,tcs-conf:,jobs:,out:,tag:,no-timestamp,allow-missing-previous" -n $NAME -- "$@")

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
        PREV_DIR=$2
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
    '-j' | '--jobs')
        JOBS=$2
        shift 2
        continue
        ;;
    '-o' | '--out')
        TOP_DIR=$2
        shift 2
        continue
        ;;
    '-t' | '--tag')
        TAG=$2
        shift 2
        continue
        ;;
    '--no-timestamp')
        TS_FLAG="--no-timestamp"
        shift 1
        continue
        ;;
    '--allow-missing-previous')
        SKIP_NO_PREV=0
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

INST_REGEX=$1
TARGETS=$2

if [[ -z "$INST_REGEX" ]]; then
	echo "Error: Missing required argument INST_REGEX"
	usage
elif [[ -z "$TARGETS" ]]; then
	echo "Error: Missing required argument TARGETS"
	usage
fi

# Sanity checks
if [[ ! -d $TARGETS ]]; then
	echo "Directory $TARGETS not found"
	usage
elif [[ ! -z $PREV_DIR && ! -d $PREV_DIR ]]; then
	echo "Directory $PREV_DIR not found"
	usage
elif [[ ! -f $SYNTH_CONF ]]; then
	echo "File $SYNTH_CONF not found"
	usage
elif [[ ! -f $TCS_CONF ]]; then
	echo "File $TCS_CONF not found"
	usage
fi

# Parse instruction regular expression
case $INST_REGEX in
'byte')
    INST_REGEX="B-"
    ;;
'word')
    INST_REGEX="W-"
    ;;
'long')
    INST_REGEX="L-"
    ;;
'quad')
    INST_REGEX="Q-"
    ;;
'arith')
    INST_REGEX="ADD/||/SUB"
    ;;
'bitwise')
    INST_REGEX="AND/||/OR.-R.*-R.*/||/XOR"
    ;;
'shifts')
    INST_REGEX="SHL/||/SHR/||/SAR"
    ;;
'shifts-1')
    INST_REGEX="SHL.*-ONE/||/SHR.*-ONE/||/SAR.*-ONE"
    ;;
'shifts-cl')
    INST_REGEX="SHL.*-CL/||/SHR.*-CL/||/SAR.*-CL"
    ;;
'rotate')
    INST_REGEX="RO"
    ;;
'imul')
    INST_REGEX="IMUL.*-R.*-R.*"
    ;;
*)
    ;;
esac

INST_LIST=$(ls $TARGETS | awk "/$INST_REGEX/"' {print substr($1, 1, index($1, ".") - 1)}' | sort | uniq)

echo "[$NAME] Target instructions matched: "
echo $INST_LIST
ERR=0

# Make top directory
if [ ! -d "$TOP_DIR" ]; then
    mkdir $TOP_DIR
fi

# Run stoke jobs
for inst in $INST_LIST
do
    # Wait for current jobs if at max
    while (( $(jobs -r | wc -l) >= JOBS )); do
        wait -n
        if [[ $? -ne 0 && $? -ne 5 ]]; then
            # run_stoke.sh exits with 5 on failure, other nonzero code on actual error
            ERR=1
        fi
    done

    # Start next job
    target=$TARGETS/$inst.s

    if [[ ! -f $target ]]; then
        echo "[$NAME] WARNING: Target $target not found; skipping"
        continue
    fi
    
    prev_arg=""

    if [[ ! -z $PREV_DIR ]]; then
        previous=$PREV_DIR/$inst.s
        prev_arg="--previous $previous"
        if [[ ! -f $previous ]]; then
            if [[ $SKIP_NO_PREV -eq 0 ]]; then
                echo "[$NAME] Previous transform $previous not found. Running without it (--allow-missing-previous)"
                prev_arg=""
            else
                echo "[$NAME] WARNING: Previous transform $previous not found; skipping"
                continue
            fi
        fi
    fi

    echo "[$NAME] Starting $inst job: ./run_stoke.sh $target $prev_arg -s $SYNTH_CONF -c $TCS_CONF -o $TOP_DIR -t $TAG/$inst $TS_FLAG"
    ./run_stoke.sh $target $prev_arg -s $SYNTH_CONF -c $TCS_CONF -o $TOP_DIR -t $TAG/$inst $TS_FLAG
done

echo "[$NAME] Waiting for jobs to finish..."
while (( $(jobs -r | wc -l) > 0 )); do
    wait -n
    if [[ $? -ne 0 ]]; then
        ERR=1
    fi
done

if [[ $ERR -ne 0 ]]; then
    echo "[$NAME] Error occurred during jobs. Check logs in $TOP_DIR"
    exit 1
fi
echo "[$NAME] Done"
