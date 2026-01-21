#!/bin/bash
# Project 1 - Directory + File Creation Script
# This script creates a main directory using the current date and time,
# generates subdirectories and text files, and logs each action to script.log.

# Name of the log file used to record script activity
LOG_FILE="script.log"

# Function: log_msg
# Purpose: Writes a timestamped message to the log file and prints it to the terminal
log_msg() {
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# Log the start of the script
log_msg "Script started"

# Generate a main directory name using the current date and time
# Format: YYYY-MM-DD_HH-MM-SS
MAIN_DIR="$(date '+%Y-%m-%d_%H-%M-%S')"

# Create the main directory
mkdir -p "$MAIN_DIR"

# Error handling: check if directory creation failed
if [ $? -ne 0 ]; then
  log_msg "ERROR: Could not create main directory: $MAIN_DIR"
  exit 1
fi

# Log successful creation of the main directory
log_msg "Main directory created: $MAIN_DIR"

# Array containing programming languages to write into text files
# Each file will receive one language from this list
LANGUAGES=("Python" "Java" "C" "C++" "JavaScript" "Ruby" "Go" "Rust" "Swift" "Kotlin")

# Loop to create subdirectories file101 through file110
for dir_num in $(seq 101 110); do
  subdir="file${dir_num}"

  # Create the subdirectory inside the main directory
  mkdir -p "$MAIN_DIR/$subdir"

  # Error handling for subdirectory creation
  if [ $? -ne 0 ]; then
    log_msg "ERROR: Could not create subdirectory: $MAIN_DIR/$subdir"
    exit 1
  fi

  # Log subdirectory creation
  log_msg "Subdirectory created: $MAIN_DIR/$subdir"

  # Inner loop to create files tuser501.txt through tuser510.txt
  for i in $(seq 1 10); do
    # Calculate file number (501–510)
    file_num=$((500 + i))

    # Build file name
    file_name="tuser${file_num}.txt"

    # Select programming language based on loop index
    language="${LANGUAGES[$((i - 1))]}"

    # Write the programming language into the file
    echo "$language" > "$MAIN_DIR/$subdir/$file_name"

    # Error handling for file creation
    if [ $? -ne 0 ]; then
      log_msg "ERROR: Could not create file: $MAIN_DIR/$subdir/$file_name"
      exit 1
    fi

    # Log file creation
    log_msg "Created file: $subdir/$file_name"
  done
done

# Log successful completion of the script
log_msg "Script finished successfully"

# Print final messages to the terminal
echo "Done. Output folder: $MAIN_DIR"
echo "Log file: $(pwd)/$LOG_FILE"
