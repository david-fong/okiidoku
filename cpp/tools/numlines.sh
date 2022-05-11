#!/bin/bash
set -eu -o pipefail
cd "${0%/*}/.."
wc -l $(git ls-files | \grep -E "[.](h|hpp|cpp)$") | sort -n
