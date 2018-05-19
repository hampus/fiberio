#!/bin/bash
#
# Run this from the build directory to generate lcov coverage reports.
#
# The build directory should be a subdirectory inside the source tree for this
# script to work.
#

SCRIPT_DIR=$(dirname "$(readlink -f "$BASH_SOURCE")")
REPORT_PATH=$(readlink -f .)/lcov-html/index.html

lcov --capture --directory $SCRIPT_DIR --no-external --output-file coverage.info

genhtml coverage.info --output-directory lcov-html

echo
echo "Coverage HTML report at file://$REPORT_PATH"
