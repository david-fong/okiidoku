#!/bin/bash
set -eu -o pipefail
wc -l $(git ls-files | \grep -E "[.](h|hpp|cpp)$") | sort -n
