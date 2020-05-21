#!/bin/bash
set -eu
LC_ALL=C egrep -o "[a-zA-Z]+" "$1" | tr '[:upper:]' '[:lower:]' | awk '{a[$1]++} END {for(k in a) print a[k], k}' | sort -k1,1rn -k2,2 > "$2"
