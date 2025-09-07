#!/bin/bash

# Get the current working directory
PWD=$(pwd)

# Clean to ensure all commands are shown, then run make dry-run
make clean
# Run make dry-run, extract gcc compilation commands with -c
# Then parse and generate JSON
make -n all | grep -E "^gcc.* -c" | awk -v dir="$PWD" '
BEGIN { print "["; first=1 }

{
  # Get the full command line, remove leading whitespace
  line = $0
  sub(/^\s*/, "", line)

  # Find the file after -c
  split(line, parts, " ")
  for(i=1; i<=length(parts); i++) {
    if(parts[i] == "-c") {
      file = parts[i+1]
      break
    }
  }

  # Print comma separator if not first entry
  if (!first) print "  ,"

  first=0

  # Print JSON entry
  print "  {"
  print "    \"directory\": \"" dir "\","
  print "    \"command\": \"" line "\","
  print "    \"file\": \"" file "\""
  print "  }"
}

END {
  print "]"
}' > ./build/compile_commands.json

echo "compile_commands.json generated successfully."
