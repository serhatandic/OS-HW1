#!/bin/bash

# Path to eshell directory
ESH_DIR=".."
# Executable name
ESH_EXEC="eshellv2"
# Full path to eshell executable
ESH_PATH="$ESH_DIR/$ESH_EXEC"

# Directory containing input and expected output files
INPUT_DIR="./input"
OUTPUT_DIR="./output"

PASSED=0
TOTAL=0

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to compare outputs and log results
compare_and_log() {
    local input_file="$1"
    local output_file="$2"
    local test_name="${input_file%.in}"

    if [[ "$input_file" == *"timed"* ]]; then
        # Measure execution time
        local start_time=$(date +%s.%N)

        # Execute eshell and capture output
        actual_output=$(cat "$INPUT_DIR/$input_file" | "$ESH_PATH" | head -n -1)

        local end_time=$(date +%s.%N)
        local elapsed=$(echo "$end_time - $start_time" | bc)

        # Read expected limits and output
        local expected_info=$(tail -n 1 "$OUTPUT_DIR/$output_file")
        local lower_limit=$(echo "$expected_info" | awk '{print $1}')
        local upper_limit=$(echo "$expected_info" | awk '{print $2}')
        
        expected_output=$(echo "$expected_info" | awk '{print $3}')

        # Perform time comparison using awk for floating-point support
        local time_check_pass=$(echo | awk -v elapsed="$elapsed" -v lower="$lower_limit" -v upper="$upper_limit" '
            BEGIN {
                if ((lower == "" || elapsed >= lower) && (upper == "" || elapsed <= upper)) print 1;
                else print 0;
            }
        ')

        if [[ "$actual_output" == "$expected_output" ]] && [[ "$time_check_pass" -eq 1 ]]; then
            echo -e "Test $test_name ${YELLOW}(timed)${NC}: ${GREEN}PASS${NC} (Elapsed: $elapsed, Limits: $lower_limit-$upper_limit)"
            ((PASSED++))
        else
            echo -e "Test $test_name ${YELLOW}(timed)${NC}: ${RED}FAIL${NC} (Elapsed: $elapsed, Limits: $lower_limit-$upper_limit)"
            if [[ "$actual_output" != "$expected_output" ]]; then
                echo "Expected:"
                echo "$expected_output"
                echo "Actual:"
                echo "$actual_output"
            fi
        fi
    else
        local actual_output=$(cat "$INPUT_DIR/$input_file" | "$ESH_PATH" | head -n -1)
        local expected_output=$(cat "$OUTPUT_DIR/$output_file")

        if [[ "$actual_output" == "$expected_output" ]]; then
            echo -e "Test $test_name: ${GREEN}PASS${NC}"
            ((PASSED++))
        else
            echo -e "Test $test_name: ${RED}FAIL${NC}"
            echo "Expected:"
            echo "$expected_output"
            echo "Actual:"
            echo "$actual_output"
        fi
    fi
    ((TOTAL++))
}

# Build the eshell executable using the Makefile
echo "Building eshell..."
(cd "$ESH_DIR" && make)
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build eshell. Aborting tests.${NC}"
    exit 1
fi
echo -e "${GREEN}Build successful.${NC}"

# Main loop to process each input file
for input_path in "$INPUT_DIR"/*.in; do
    # Extract just the filename from the path
    input_file=$(basename -- "$input_path")
    # Construct the expected output file name by changing the extension from .in to .out
    output_file="${input_file%.in}.out"

    if [[ -f "$OUTPUT_DIR/$output_file" ]]; then
        compare_and_log "$input_file" "$output_file"
    else
        echo "Output file for $input_file does not exist, skipping..."
    fi
done

# Calculate the percentage of passed tests
if [ "$TOTAL" -gt 0 ]; then
    percentage=$(echo "scale=2; $PASSED/$TOTAL*100" | bc)
else
    percentage=0
fi
echo -e "${CYAN}Grade: $PASSED/$TOTAL ($percentage%)${NC}"

