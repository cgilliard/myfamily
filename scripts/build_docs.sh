#!/bin/bash

echo "Building docs";
make doc
git clone https://anything:$1@github.com/cgilliard/myfamily.git myfamilydocs
cd myfamilydocs

rm -rf docs/html/*
cp -rp ../doc/html/* docs/html


./configure
make clean coverage
# append code cov numbers to our running totals
cat /tmp/codecov >> docs/cc.txt
./scripts/update_code_coverage.sh

git config user.name "Pipelines-Bot"
git checkout main
git config user.email "pipelinesbot.noreply@example.com"
git config user.name "Pipelines-Bot"
git add --all
git commit -am "Pipelines-Bot: Updated repo (via build_docs script) Source Version is $2";
git push https://$1@github.com/cgilliard/myfamily.git
