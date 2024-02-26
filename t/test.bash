#!/usr/bin/env bash
# SPDX-License-Identifier: MIT License

set -e
set -u
shopt -s extglob
unset CDPATH
IFS='
	'

VERBOSE="${VERBOSE:-1}"
THIS_DIR=$(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)
COUNTER=0
ERR_COUNTER=0
EXE="${1:-${THIS_DIR}/../bf-cc}"

TEST_CASE_FLAG_MATRIX=("--interp --optimize=0"
                       "--interp --optimize=1"
                       "--interp --optimize=2"
                       "--interp --optimize=3"
                       "--compiler --optimize=0"
                       "--compiler --optimize=1"
                       "--compiler --optimize=2"
                       "--compiler --optimize=3")

function run_testcase () {
    name="$1"
    flags="$2"
    input_file="${THIS_DIR}/${name}.in"
    if ! [[ -r "${input_file}" ]]; then
        input_file='/dev/null'
    fi
    if [[ $VERBOSE -ne 0 ]]; then
        eval "${EXE}" "$flags" "${THIS_DIR}/${name}.b" < "${input_file}"
    else
        eval "${EXE}" "$flags" "${THIS_DIR}/${name}.b" < "${input_file}" 2>/dev/null
    fi
}

function test_case_flags_from_filename () {
    name="$1"
    file_flags=""
    while [[ "${name}" != "${name%.*}" ]]; do
          file_flags="${file_flags} ${name##*.}"
          name="${name%.*}"
    done
    echo "$file_flags"
}

for test_case in "${THIS_DIR}"/*.b; do
    test_case_name="$(basename "${test_case%.*}")"
    output_file="${THIS_DIR}/${test_case_name}.out"
    if ! [[ -r "${output_file}" ]]; then
        continue
    fi
    file_flags="$(test_case_flags_from_filename "$test_case_name")"
    for flags in "${TEST_CASE_FLAG_MATRIX[@]}"; do
        flags="${flags} ${file_flags}"
        flags="${flags//+([[:space:]])/ }"
        COUNTER=$((COUNTER+1))
        ERR_COUNTER=$((ERR_COUNTER+1))
        if [[ $VERBOSE -ne 0 ]]; then
            printf '%s %s\n' "$(basename "${test_case_name}")" "${flags}"
        fi
        if actual_output="$(run_testcase "${test_case_name}" "${flags}")"; then
            actual_exit=0
        else
            actual_exit=$?
        fi
        if [[ $actual_exit -ne 0 ]]; then
            if [[ $VERBOSE -ne 1 ]]; then
                printf '%s\n' "$(basename "${test_case_name}")"
            fi
            printf 'Exit %d\n' "$actual_exit"
        elif ! diff -q "${output_file}" - <<< "$actual_output" > /dev/null; then
            if [[ $VERBOSE -ne 1 ]]; then
                printf '%s\n' "$(basename "${test_case_name}")"
            fi
            diff -u "${output_file}" - <<< "$actual_output"
        else
            ERR_COUNTER=$((ERR_COUNTER-1))
            if [[ $VERBOSE -ne 0 ]]; then
                printf 'OK\n'
            fi
        fi
    done
done

printf 'Tests: %d\nOK: %d\nFailed: %d\n' "$COUNTER" "$((COUNTER-ERR_COUNTER))" "$ERR_COUNTER"

if [[ $ERR_COUNTER -eq 0 ]]; then
    exit 0
else
    exit 1
fi
