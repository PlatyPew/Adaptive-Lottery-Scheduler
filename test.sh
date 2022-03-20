#!/bin/sh
PROGRAM="./assignment"

if [ ! -f "$PROGRAM" ]; then
    echo "$PROGRAM not found"
	exit 1
fi

for FILE in test/testcase0*.txt; do
	$PROGRAM $FILE || exit 1
    echo
done
