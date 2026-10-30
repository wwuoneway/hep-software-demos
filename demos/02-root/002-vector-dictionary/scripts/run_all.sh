#!/usr/bin/env bash
# Runs every (type, step) in its own process and prints the results.
cd "$(dirname "$0")/.." || exit 1
export ROOT_INCLUDE_PATH="$PWD/include${ROOT_INCLUDE_PATH:+:$ROOT_INCLUDE_PATH}"

for type in point pixel ghost secret; do
  case $type in
    point) echo "=== std::vector<demo::Point> (Point + std::vector<Point> have dictionaries) ===" ;;
    pixel) echo "=== std::vector<demo::Pixel> (Pixel has a dictionary, std::vector<Pixel> does not) ===" ;;
    ghost) echo "=== std::vector<demo::Ghost> (no dictionary; header visible to the interpreter) ===" ;;
    secret) echo "=== std::vector<Secret>  (no dictionary; unknown to the interpreter) ===" ;;
  esac
  for step in check ttree rntuple; do
    output=$(build/probe "$type" "$step" 2>&1)
    status=$?
    # Keep ROOT's own error lines short; the summary line comes from probe.
    echo "$output" | grep -vE '^\s*$' | cut -c1-150 | sed 's/^/  /'
    if [[ $status -ge 128 ]]; then
      echo "  ($step crashed: signal $((status - 128)))"
    fi
  done
  echo
done
