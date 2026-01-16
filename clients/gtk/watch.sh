#!/bin/bash

# Set the C file to monitor
SOURCE_FILES="*.c"
OUTPUT_FILE="/tmp/shb-dev"

# Compile and run the application
compile_and_run() {
    echo -n "Detected changes.. "
    killall "$OUTPUT_FILE"
    sleep 0.5
    echo "Compiling..."
    make TARGET="$OUTPUT_FILE"
    if [ $? -eq 0 ]; then
        echo "Compilation successful. Running the app..."
        "$OUTPUT_FILE" &
    else
        echo "Compilation failed."
    fi
}

# Check if inotify-tools is installed
if ! command -v inotifywait &>/dev/null; then
    echo "Error: inotifywait is not installed. Please install it using 'apk add inotify-tools'."
    exit 1
fi

echo "Watching for changes in .c files..."
# Monitor the current directory for changes in .c files
while inotifywait -e modify,create,delete $SOURCE_FILES; do
    compile_and_run
done