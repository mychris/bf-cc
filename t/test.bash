#!/usr/bin/env bash

set -e
set -u
unset CDPATH
IFS='
	'

VERBOSE="${VERBOSE:-1}"
THIS_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
COUNTER=0
ERR_COUNTER=0

for test_case in "${THIS_DIR}"/*.b; do
    test_case_name="${test_case%.*}"
    output_file="${test_case_name}.out"
    if [[ -r "${output_file}" ]]; then
        input_file="${test_case%.*}.in"
        if ! [[ -r "${input_file}" ]]; then
            input_file='/dev/null'
        fi
        COUNTER=$((COUNTER+1))
        ERR_COUNTER=$((ERR_COUNTER+1))
        if [[ $VERBOSE -ne 0 ]]; then
            printf '%s\n' "$(basename "${test_case_name}")"
        fi
        set +e
        actual_output="$("${THIS_DIR}"/../bf-cc -O2 "${test_case}" < "${input_file}" 2>/dev/null)"
        actual_exit=$?
        set -e
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
    fi
done

printf 'Tests: %d\nOK: %d\nFailed: %d\n' "$COUNTER" "$((COUNTER-ERR_COUNTER))" "$ERR_COUNTER"

if [[ $ERR_COUNTER -eq 0 ]]; then
    exit 0
else
    exit 1
fi
