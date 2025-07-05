#!/bin/bash

# Polylla Test Suite
# Comprehensive testing script with visual feedback

# Colors and emojis
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test counters
declare -A test_counts
declare -A test_passed

# Initialize counters
test_counts[triangle_neigh]=0
test_counts[triangle_ele]=0
test_counts[poly_basic]=0
test_counts[off_basic]=0
test_counts[gpu]=0
test_counts[smoothing]=0
test_counts[regions]=0
test_counts[combined]=0
test_counts[edge_cases]=0
test_counts[poly_advanced]=0
test_counts[error_handling]=0

test_passed[triangle_neigh]=0
test_passed[triangle_ele]=0
test_passed[poly_basic]=0
test_passed[off_basic]=0
test_passed[gpu]=0
test_passed[smoothing]=0
test_passed[regions]=0
test_passed[combined]=0
test_passed[edge_cases]=0
test_passed[poly_advanced]=0
test_passed[error_handling]=0

# Configuration
TIMEOUT=60
# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Polylla binary path relative to script location
POLYLLA_BIN="$SCRIPT_DIR/../build/Polylla"
TEST_DIR="test_files"
LOG_FILE="test_results.log"

# Make log file path absolute before changing directory
LOG_FILE=$(realpath "$LOG_FILE")

# Clear previous log
> "$LOG_FILE"

echo -e "${BLUE}🧪 Polylla Test Suite${NC}"
echo -e "${BLUE}=====================${NC}"
echo

# Function to validate basic file format
validate_basic_format() {
    local off_file="$1"
    local json_file="$2"
    local test_name="$3"
    
    echo "Basic validation for: $test_name" >> "$LOG_FILE"
    
    # Basic OFF validation - just check header and structure
    if [[ -f "$off_file" ]]; then
        local first_line=$(head -n 1 "$off_file" 2>/dev/null)
        if [[ "$first_line" != "OFF" ]]; then
            echo "FAIL: OFF file does not start with 'OFF' header" >> "$LOG_FILE"
            return 1
        fi
        
        local second_line=$(sed -n '2p' "$off_file" 2>/dev/null)
        if [[ ! "$second_line" =~ ^[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+$ ]]; then
            echo "FAIL: OFF file has invalid second line format" >> "$LOG_FILE"
            return 1
        fi
    fi
    
    # Basic JSON validation - just check if it's valid JSON
    if [[ -f "$json_file" ]]; then
        if ! python3 -m json.tool "$json_file" > /dev/null 2>&1; then
            echo "FAIL: JSON file is not valid JSON format" >> "$LOG_FILE"
            return 1
        fi
    fi
    
    echo "Basic validation PASSED" >> "$LOG_FILE"
    return 0
}

# Function to clean output files for a specific test
clean_output_files() {
    local expected_output="$1"
    local test_name="$2"
    
    echo "Cleaning previous output files for: $expected_output" >> "$LOG_FILE"
    
    # List of input files that should NEVER be deleted
    local input_files=(
        "pikachu_triangle.off"
        "pikachu.1.node" "pikachu.1.ele" "pikachu.1.neigh"
        "pikachu_regiones.1.node" "pikachu_regiones.1.ele" "pikachu_regiones.1.neigh"
        "pikachu_poly.poly" "pikachu_regiones_poly.poly"
    )
    
    # Check if the expected output file is actually an input file
    local is_input_file=false
    for input_file in "${input_files[@]}"; do
        if [[ "$expected_output.off" == "$input_file" ]]; then
            is_input_file=true
            echo "Skipping deletion of input file: $expected_output.off" >> "$LOG_FILE"
            break
        fi
    done
    
    # Remove ONLY the specific output files for this test (but NOT input files)
    # Regular output files
    if [[ "$is_input_file" == "false" ]] && [[ -f "$expected_output.off" ]]; then
        rm -f "$expected_output.off"
        echo "Removed previous $expected_output.off" >> "$LOG_FILE"
    fi
    
    if [[ -f "$expected_output.json" ]]; then
        rm -f "$expected_output.json"
        echo "Removed previous $expected_output.json" >> "$LOG_FILE"
    fi
    
    # _polylla suffix files (for OFF inputs) - these are always safe to delete
    if [[ -f "${expected_output}_polylla.off" ]]; then
        rm -f "${expected_output}_polylla.off"
        echo "Removed previous ${expected_output}_polylla.off" >> "$LOG_FILE"
    fi
    
    if [[ -f "${expected_output}_polylla.json" ]]; then
        rm -f "${expected_output}_polylla.json"
        echo "Removed previous ${expected_output}_polylla.json" >> "$LOG_FILE"
    fi
}

# Function to clean ALL test outputs at the start
clean_all_test_outputs() {
    echo "Cleaning all previous test output files..." >> "$LOG_FILE"
    echo "Current directory: $(pwd)" >> "$LOG_FILE"
    
    local removed_count=0
    
    # Strategy: Remove ONLY files that end with _polylla.off or _polylla.json
    # These are DEFINITELY output files from OFF inputs
    echo "Removing all *_polylla.off and *_polylla.json files..." >> "$LOG_FILE"
    
    for file in *_polylla.off *_polylla.json; do
        if [[ -f "$file" ]]; then
            echo "Removing: $file" >> "$LOG_FILE"
            rm -f "$file"
            ((removed_count++))
        fi
    done
    
    # Also remove specific known output files (but NOT input files)
    local specific_outputs=(
        "pikachu.1.off" "pikachu.1.json"
        "pikachu_regiones.1.off" "pikachu_regiones.1.json"
        "pikachu_poly.off" "pikachu_poly.json"
        "pikachu_regiones_poly.off" "pikachu_regiones_poly.json"
        "pikachu_triangle_polylla.off" "pikachu_triangle_polylla.json"
    )
    
    echo "Removing specific known output files..." >> "$LOG_FILE"
    for file in "${specific_outputs[@]}"; do
        if [[ -f "$file" ]]; then
            echo "Removing: $file" >> "$LOG_FILE"
            rm -f "$file"
            ((removed_count++))
        fi
    done
    
    echo "Cleaned $removed_count test output files" >> "$LOG_FILE"
    
    # Log what files remain
    echo "Files remaining after cleanup:" >> "$LOG_FILE"
    ls -la *.off *.json 2>/dev/null >> "$LOG_FILE" || echo "No .off or .json files found" >> "$LOG_FILE"
}

# Function to run a test
run_test() {
    local test_name="$1"
    local test_type="$2"
    local cmd="$3"
    local expected_output="$4"
    
    ((test_counts[$test_type]++))
    
    echo -e "  ${test_name}..."
    echo -e "    ${CYAN}Command:${NC} $cmd"
    echo -n "    Result: "
    
    # Clean previous output files
    clean_output_files "$expected_output" "$test_name"
    
    # Log the command
    echo "=== TEST: $test_name ===" >> "$LOG_FILE"
    echo "Command: $cmd" >> "$LOG_FILE"
    echo "Expected output: $expected_output" >> "$LOG_FILE"
    
    # Run the command with timeout, capture all output
    if timeout $TIMEOUT bash -c "$cmd" >> "$LOG_FILE" 2>&1; then
        # Check if expected output files exist
        echo "Checking for files: $expected_output.off and $expected_output.json" >> "$LOG_FILE"
        echo "Files in current directory:" >> "$LOG_FILE"
        ls -la *.off *.json 2>/dev/null >> "$LOG_FILE" || echo "No .off or .json files found in current directory" >> "$LOG_FILE"
        
        # Check for files in current directory (which should be test directory now)
        local output_off=""
        local output_json=""
        
        # Check for regular output files first
        if [[ -f "$expected_output.off" ]] && [[ -f "$expected_output.json" ]]; then
            output_off="$expected_output.off"
            output_json="$expected_output.json"
        # Check for _polylla suffix files (for OFF input files)
        elif [[ -f "${expected_output}_polylla.off" ]] && [[ -f "${expected_output}_polylla.json" ]]; then
            output_off="${expected_output}_polylla.off"
            output_json="${expected_output}_polylla.json"
        fi
        
        if [[ -n "$output_off" ]] && [[ -n "$output_json" ]]; then
            # Check if files have content
            if [[ -s "$output_off" ]] && [[ -s "$output_json" ]]; then
                # Basic format validation
                if validate_basic_format "$output_off" "$output_json" "$test_name"; then
                    echo -e "${GREEN}✅ PASS${NC}"
                    ((test_passed[$test_type]++))
                    echo "Result: PASS" >> "$LOG_FILE"
                else
                    echo -e "${RED}❌ FAIL${NC} (format validation failed)"
                    echo "Result: FAIL - Format validation failed" >> "$LOG_FILE"
                fi
            else
                echo -e "${RED}❌ FAIL${NC} (empty output)"
                echo "Result: FAIL - Empty output files" >> "$LOG_FILE"
            fi
        else
            echo -e "${RED}❌ FAIL${NC} (missing output)"
            echo "Result: FAIL - Missing output files" >> "$LOG_FILE"
        fi
    else
        echo -e "${RED}❌ FAIL${NC} (crash/timeout)"
        echo "Result: FAIL - Crash or timeout" >> "$LOG_FILE"
    fi
    
    echo "" >> "$LOG_FILE"
    echo  # Add blank line for readability
}

# Function to run a test that should fail
run_fail_test() {
    local test_name="$1"
    local test_type="$2"
    local cmd="$3"
    
    ((test_counts[$test_type]++))
    
    echo -e "  ${test_name}..."
    echo -e "    ${CYAN}Command:${NC} $cmd"
    echo -n "    Result: "
    
    # Log the command
    echo "=== FAIL TEST: $test_name ===" >> "$LOG_FILE"
    echo "Command: $cmd" >> "$LOG_FILE"
    echo "Expected: Should fail" >> "$LOG_FILE"
    
    # Run the command with timeout, suppress stdout/stderr
    if timeout $TIMEOUT bash -c "$cmd" > /dev/null 2>> "$LOG_FILE"; then
        echo -e "${RED}❌ FAIL${NC} (should have failed)"
        echo "Result: FAIL - Command succeeded when it should have failed" >> "$LOG_FILE"
    else
        echo -e "${GREEN}✅ PASS${NC} (failed as expected)"
        ((test_passed[$test_type]++))
        echo "Result: PASS - Failed as expected" >> "$LOG_FILE"
    fi
    
    echo "" >> "$LOG_FILE"
    echo  # Add blank line for readability
}

# Function to validate input files exist
validate_input_files() {
    echo "Validating input files..." >> "$LOG_FILE"
    
    local missing_files=()
    
    # Check Triangle files
    local triangle_files=(
        "pikachu.1.node" "pikachu.1.ele" "pikachu.1.neigh"
        "pikachu_regiones.1.node" "pikachu_regiones.1.ele" "pikachu_regiones.1.neigh"
    )
    
    for file in "${triangle_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$file")
        fi
    done
    
    # Check Poly files
    local poly_files=(
        "pikachu_poly.poly"
        "pikachu_regiones_poly.poly"
    )
    
    for file in "${poly_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$file")
        fi
    done
    
    # Check OFF files
    local off_files=(
        "pikachu_triangle.off"
    )
    
    for file in "${off_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$file")
        fi
    done
    
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        echo -e "${RED}❌ Missing input files:${NC}"
        for file in "${missing_files[@]}"; do
            echo -e "  ${RED}  - $file${NC}"
        done
        echo -e "${YELLOW}💡 Please ensure all test input files are present in the test_files directory${NC}"
        exit 1
    fi
    
    echo "All input files validated successfully" >> "$LOG_FILE"
    echo -e "${GREEN}✅ All input files present${NC}"
}

# Check if Polylla binary exists
if [[ ! -f "$POLYLLA_BIN" ]]; then
    echo -e "${RED}❌ Error: Polylla binary not found at $POLYLLA_BIN${NC}"
    echo -e "${YELLOW}💡 Please compile first: cd build && make${NC}"
    exit 1
fi

# Change to test directory for simpler relative paths
cd "$TEST_DIR"

# Update Polylla binary path for the new working directory
POLYLLA_BIN="../../build/Polylla"

# Check again after changing directory
if [[ ! -f "$POLYLLA_BIN" ]]; then
    echo -e "${RED}❌ Error: Polylla binary not found at $POLYLLA_BIN after changing to test directory${NC}"
    echo -e "${YELLOW}💡 Please compile first: cd build && make${NC}"
    exit 1
fi

# Validate input files
validate_input_files

# Clean all previous test outputs
clean_all_test_outputs

echo -e "${PURPLE}📁 Triangle Files Tests (.node + .ele + .neigh)${NC}"
echo -e "${PURPLE}================================================${NC}"

# Triangle files with .neigh
run_test "Pikachu basic (with .neigh)" "triangle_neigh" \
    "$POLYLLA_BIN --neigh pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "Pikachu regiones (with .neigh)" "triangle_neigh" \
    "$POLYLLA_BIN --neigh pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

echo

echo -e "${CYAN}📁 Triangle Files Tests (.node + .ele only)${NC}"
echo -e "${CYAN}===========================================${NC}"

# Triangle files without .neigh
run_test "Pikachu basic (without .neigh)" "triangle_ele" \
    "$POLYLLA_BIN --ele pikachu.1.node pikachu.1.ele" \
    "pikachu.1"

run_test "Pikachu regiones (without .neigh)" "triangle_ele" \
    "$POLYLLA_BIN --ele pikachu_regiones.1.node pikachu_regiones.1.ele" \
    "pikachu_regiones.1"

echo

echo -e "${YELLOW}📁 Poly Files Tests${NC}"
echo -e "${YELLOW}===================${NC}"

# Poly files (requires Triangle)
run_test "Pikachu poly basic" "poly_basic" \
    "$POLYLLA_BIN --poly pikachu_poly.poly" \
    "pikachu_poly"

run_test "Pikachu regiones poly" "poly_basic" \
    "$POLYLLA_BIN --poly pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

echo

echo -e "${GREEN}📄 OFF Files Tests${NC}"
echo -e "${GREEN}==================${NC}"

# OFF files tests
run_test "Pikachu OFF basic" "off_basic" \
    "$POLYLLA_BIN --off pikachu_triangle.off" \
    "pikachu_triangle"

run_test "Pikachu OFF with laplacian smoothing" "off_basic" \
    "$POLYLLA_BIN --off --smooth laplacian --iterations 10 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "Pikachu OFF with edge-ratio smoothing" "off_basic" \
    "$POLYLLA_BIN --off --smooth laplacian-edge-ratio --iterations 10 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "Pikachu OFF with distmesh smoothing" "off_basic" \
    "$POLYLLA_BIN --off --smooth distmesh --target-length 500 --iterations 10 pikachu_triangle.off" \
    "pikachu_triangle"

echo

echo -e "${PURPLE}🚀 GPU Acceleration Tests${NC}"
echo -e "${PURPLE}========================${NC}"

# Basic GPU tests
run_test "GPU OFF basic" "gpu" \
    "$POLYLLA_BIN --off --gpu pikachu_triangle.off" \
    "pikachu_triangle"

run_test "GPU Triangle with .neigh" "gpu" \
    "$POLYLLA_BIN --neigh --gpu pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

# GPU with smoothing
run_test "GPU OFF with laplacian smoothing" "gpu" \
    "$POLYLLA_BIN --off --gpu --smooth laplacian --iterations 10 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "GPU OFF with edge-ratio smoothing" "gpu" \
    "$POLYLLA_BIN --off --gpu --smooth laplacian-edge-ratio --iterations 10 pikachu_triangle.off" \
    "pikachu_triangle"

# GPU with regions
run_test "GPU Triangle with regions" "gpu" \
    "$POLYLLA_BIN --neigh --gpu --region pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

# GPU combined options
run_test "GPU OFF with regions and smoothing" "gpu" \
    "$POLYLLA_BIN --off --gpu --region --smooth laplacian --iterations 5 pikachu_triangle.off" \
    "pikachu_triangle"

echo

echo -e "${GREEN}🎨 Smoothing Tests${NC}"
echo -e "${GREEN}==================${NC}"

# Smoothing tests
run_test "Laplacian smoothing" "smoothing" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations 10 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "Laplacian edge-ratio smoothing" "smoothing" \
    "$POLYLLA_BIN --neigh --smooth laplacian-edge-ratio --iterations 10 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "DistMesh smoothing" "smoothing" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length 500 --iterations 10 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

echo

echo -e "${BLUE}🗺️  Region Tests${NC}"
echo -e "${BLUE}================${NC}"

# Region tests
run_test "Pikachu with regions" "regions" \
    "$POLYLLA_BIN --neigh --region pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

run_test "Poly with regions" "regions" \
    "$POLYLLA_BIN --poly --region pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

echo

echo -e "${PURPLE}🔧 Combined Options Tests${NC}"
echo -e "${PURPLE}=========================${NC}"

# Combined options tests
run_test "Regions + Laplacian smoothing" "combined" \
    "$POLYLLA_BIN --neigh --region --smooth laplacian --iterations 5 pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

run_test "Regions + Edge-ratio smoothing" "combined" \
    "$POLYLLA_BIN --neigh --region --smooth laplacian-edge-ratio --iterations 5 pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

run_test "Poly + Regions + DistMesh" "combined" \
    "$POLYLLA_BIN --poly --region --smooth distmesh --target-length 500 --iterations 5 pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

run_test "OFF + Regions + Edge-ratio smoothing" "combined" \
    "$POLYLLA_BIN --off --region --smooth laplacian-edge-ratio --iterations 15 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "Triangle + Regions + DistMesh + High iterations" "combined" \
    "$POLYLLA_BIN --neigh --region --smooth distmesh --target-length 300 --iterations 20 pikachu_regiones.1.node pikachu_regiones.1.ele pikachu_regiones.1.neigh" \
    "pikachu_regiones.1"

run_test "Poly + Quality + Regions + Laplacian" "combined" \
    "$POLYLLA_BIN -p:pq30nzAa --region --smooth laplacian --iterations 10 pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

echo

echo -e "${YELLOW}🔍 Edge Cases Tests${NC}"
echo -e "${YELLOW}===================${NC}"

# Edge cases with different parameters
run_test "Smoothing with 0 iterations" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations 0 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "Smoothing with 1 iteration" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations 1 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "DistMesh with small target length" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length 100 --iterations 5 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "DistMesh with large target length" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length 2000 --iterations 5 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "OFF with 0 iterations" "edge_cases" \
    "$POLYLLA_BIN --off --smooth laplacian --iterations 0 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "OFF with 1 iteration" "edge_cases" \
    "$POLYLLA_BIN --off --smooth laplacian --iterations 1 pikachu_triangle.off" \
    "pikachu_triangle"

run_test "High iteration smoothing" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations 100 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "DistMesh with very small target length" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length 50 --iterations 5 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "DistMesh with very large target length" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length 5000 --iterations 5 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

run_test "Edge-ratio with 0 iterations" "edge_cases" \
    "$POLYLLA_BIN --neigh --smooth laplacian-edge-ratio --iterations 0 pikachu.1.node pikachu.1.ele pikachu.1.neigh" \
    "pikachu.1"

echo

echo -e "${CYAN}🔧 Advanced Poly Tests${NC}"
echo -e "${CYAN}======================${NC}"

# Advanced poly file tests with custom Triangle arguments
run_test "Poly with quality constraints" "poly_advanced" \
    "$POLYLLA_BIN -p:pq30nz pikachu_poly.poly" \
    "pikachu_poly"

run_test "Poly with area constraints" "poly_advanced" \
    "$POLYLLA_BIN -p:pa8000nz pikachu_poly.poly" \
    "pikachu_poly"

run_test "Poly with quality + area" "poly_advanced" \
    "$POLYLLA_BIN -p:pq15a10000nz pikachu_poly.poly" \
    "pikachu_poly"

run_test "Poly regions with quality" "poly_advanced" \
    "$POLYLLA_BIN -p:pq30nzAa --region pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

run_test "Poly with minimum angle constraint" "poly_advanced" \
    "$POLYLLA_BIN -p:pq20nz pikachu_poly.poly" \
    "pikachu_poly"

run_test "Poly with maximum area constraint" "poly_advanced" \
    "$POLYLLA_BIN -p:pa5000nz pikachu_poly.poly" \
    "pikachu_poly"

run_test "Poly with quality + area + regions" "poly_advanced" \
    "$POLYLLA_BIN -p:pq25a8000nzAa --region pikachu_regiones_poly.poly" \
    "pikachu_regiones_poly"

run_test "Poly with high quality constraint" "poly_advanced" \
    "$POLYLLA_BIN -p:pq30nz pikachu_poly.poly" \
    "pikachu_poly"

echo

echo -e "${RED}⚠️  Error Handling Tests${NC}"
echo -e "${RED}=========================${NC}"

# Tests that should fail
run_fail_test "Non-existent .node file" "error_handling" \
    "$POLYLLA_BIN --neigh nonexistent.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Non-existent .ele file" "error_handling" \
    "$POLYLLA_BIN --neigh pikachu.1.node nonexistent.ele pikachu.1.neigh"

run_fail_test "Non-existent .neigh file" "error_handling" \
    "$POLYLLA_BIN --neigh pikachu.1.node pikachu.1.ele nonexistent.neigh"

run_fail_test "Non-existent .poly file" "error_handling" \
    "$POLYLLA_BIN --poly nonexistent.poly"

run_fail_test "Invalid smoothing method" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth invalid_method pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Invalid target length (negative)" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length -100 pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Invalid iterations (negative)" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations -5 pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Invalid iterations (non-numeric)" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations abc pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Multiple input types specified" "error_handling" \
    "$POLYLLA_BIN --off --neigh pikachu_triangle.off pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Non-existent OFF file" "error_handling" \
    "$POLYLLA_BIN --off nonexistent.off"

run_fail_test "Invalid OFF file extension" "error_handling" \
    "$POLYLLA_BIN --off pikachu.txt"

run_fail_test "Invalid target length (non-numeric)" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length abc pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Missing smoothing method" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Missing iterations value" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth laplacian --iterations pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_fail_test "Missing target-length value" "error_handling" \
    "$POLYLLA_BIN --neigh --smooth distmesh --target-length pikachu.1.node pikachu.1.ele pikachu.1.neigh"

run_test "Single .node file with --neigh" "edge_cases" \
    "$POLYLLA_BIN --neigh pikachu.1.node" \
    "pikachu.1"

echo

# Final summary
echo -e "${BLUE}📊 Test Results Summary${NC}"
echo -e "${BLUE}=======================${NC}"

total_tests=0
total_passed=0

for test_type in triangle_neigh triangle_ele poly_basic off_basic gpu smoothing regions combined edge_cases poly_advanced error_handling; do
    case $test_type in
        triangle_neigh) icon="📁" name="Triangle (.neigh)" ;;
        triangle_ele) icon="📁" name="Triangle (.ele only)" ;;
        poly_basic) icon="📄" name="Poly Files" ;;
        off_basic) icon="📄" name="OFF Files" ;;
        gpu) icon="🚀" name="GPU Acceleration" ;;
        smoothing) icon="🎨" name="Smoothing" ;;
        regions) icon="🗺️" name="Regions" ;;
        combined) icon="🔧" name="Combined Options" ;;
        edge_cases) icon="🔍" name="Edge Cases" ;;
        poly_advanced) icon="🔧" name="Advanced Poly" ;;
        error_handling) icon="⚠️" name="Error Handling" ;;
    esac
    
    passed=${test_passed[$test_type]}
    total=${test_counts[$test_type]}
    
    if [[ $total -gt 0 ]]; then
        total_tests=$((total_tests + total))
        total_passed=$((total_passed + passed))
        
        if [[ $passed -eq $total ]]; then
            echo -e "  ${icon} ${name}: ${GREEN}${passed}/${total} ✅${NC}"
        elif [[ $passed -eq 0 ]]; then
            echo -e "  ${icon} ${name}: ${RED}${passed}/${total} ❌${NC}"
        else
            echo -e "  ${icon} ${name}: ${YELLOW}${passed}/${total} ⚠️${NC}"
        fi
    fi
done

echo
echo -e "${BLUE}Overall Result:${NC}"
if [[ $total_passed -eq $total_tests ]]; then
    echo -e "  ${GREEN}🎉 ALL TESTS PASSED! (${total_passed}/${total_tests})${NC}"
    exit_code=0
elif [[ $total_passed -eq 0 ]]; then
    echo -e "  ${RED}💥 ALL TESTS FAILED! (${total_passed}/${total_tests})${NC}"
    exit_code=1
else
    echo -e "  ${YELLOW}⚠️  SOME TESTS FAILED (${total_passed}/${total_tests})${NC}"
    exit_code=1
fi

echo
echo -e "${CYAN}📝 Detailed log saved to: ${LOG_FILE}${NC}"

exit $exit_code
