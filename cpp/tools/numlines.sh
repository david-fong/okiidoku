#!/bin/bash
# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: CC0-1.0
set -eu -o pipefail
cd "${0%/*}/.."
wc -l $(git ls-files | \grep -E "[.](h|hpp|cpp)$") | sort -n
