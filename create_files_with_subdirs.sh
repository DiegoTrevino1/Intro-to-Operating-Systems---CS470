#!/bin/bash
# Project 1 - Directory + File Creation Script
# Creates a main folder named by date/time, then subfolders/file + text files.
# Also writes a log of what happened to script.log.

LOG_FILE="script.log"

# Write a timestamped message to the log and also show it in the terminal
log_msg() {
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

log_msg "Script started"

# Main folder name: YYYY-MM-DD_HH-MM-SS (keeps it readable + unique)
MAIN_DIR="$(date '+%Y-%m-%d_%H-%M-%S')"

mkdir -p "$MAIN_DIR"
log_msg "Main directory created: $MAIN_DIR"

# 10 languages (one per file: tuser501.txt -> tuser510.txt)
LANGUAGES=(
  "Python"
  "Java"
  "C"
  "C++"
  "JavaScript"
  "Ruby"
  "Go"
  "Rust"
  "Swift"
  "Kotlin"
)

# Create subfolders file101..file110
for dir_num in $(seq 101 110); do
  subdir="file${dir_num}"
  mkdir -p "$MAIN_DIR/$subdir"
  log_msg "Subdirectory created: $MAIN_DIR/$subdir"

  # Create files tuser501.txt..tuser510.txt in each subfolder
  for i in $(seq 1 10); do
    file_num=$((500 + i))                 # 501..510
    file_name="tuser${file_num}.txt"
    language="${LANGUAGES[$((i - 1))]}"   # index 0..9

    echo "$language" > "$MAIN_DIR/$subdir/$file_name"
    log_msg "Created file: $subdir/$file_name"
  done
done

log_msg "Script finished successfully"
echo "Done. Output folder: $MAIN_DIR"
echo "Log file: $(pwd)/$LOG_FILE"
