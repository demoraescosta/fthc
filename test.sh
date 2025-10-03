#!/usr/bin/env bash

TOTAL=0
PASSED=0
FAILED=0

function test_case () {
    TEST=$1
    FILE=$2
    OPTIONS=$3

    TOTAL=$((TOTAL+1))
    START=$(date +%s.%N)
    RESULT=$(./fthc $OPTIONS $FILE 2> /dev/null)
    END=$(date +%s.%N)
    TIME=$(echo "$END - $START" | bc)
    DIFF=$(diff tests/$TEST.expected <(echo "$RESULT"))
    echo running fthc $OPTIONS $FILE 
    if [[ -z "$DIFF" ]]; then 
        PASSED=$((PASSED+1)) 
        echo "${TEST^^}: PASSED"
        echo "      $TIME seconds"
    else
        FAILED=$((FAILED+1)) 
        echo "${TEST^^}: FAILED"
        echo "      $TIME seconds"
        echo DIFF: -------------
        echo $DIFF
        echo -------------------
        echo
    fi
    echo
}

echo Running tests...
echo 

test_case "usage" "" ""
test_case "sum"   "tests/sum.fth" "-i"
test_case "conditionals"   "tests/conditionals.fth" "-i"

echo Results:
echo "    Passed: $PASSED"
echo "    Failed: $FAILED"
echo "    ____________"
echo "    Total:  $TOTAL"

