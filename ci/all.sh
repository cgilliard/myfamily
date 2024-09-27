#!/bin/bash

# Activate venv for macOS tests
if [ "$(uname)" == "Darwin" ]; then
  source venv/bin/activate
  ./configure
  make clean all
  ./bin/fam init test1
  if [ ! -d ~/.fam/resources ]; then
    echo "Error: ~/.fam/resources directory does not exist"
    exit 1
  fi
  make clean testsan
fi

# Run Linux tests
if [ "$(uname)" == "Linux" ]; then
  ./configure
  make clean all
  ./bin/fam init test1
  if [ ! -d ~/.fam/resources ]; then
    echo "Error: ~/.fam/resources directory does not exist"
    exit 1
  fi
  make clean testsan
fi

# Build documentation for Linux only when the pipeline is triggered manually or on schedule
#if [ "$(uname)" == "Linux" ] && [[ "$GITHUB_EVENT_NAME" == "schedule" || "$GITHUB_EVENT_NAME" == "workflow_dispatch" ]]; then
#  ./scripts/build_docs.sh "$MyFamilyPat" "$GITHUB_SHA"
#fi

