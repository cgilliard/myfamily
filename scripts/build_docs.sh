#!/bin/bash

echo "Building docs";
make doc
git clone https://anything:$1@github.com/cgilliard/myfamily.git myfamilydocs
cd myfamilydocs
rm -rf docs/html
cp -rp ../doc/html/* docs/html
git config user.name "Pipelines-Bot"
git checkout main
git config --global user.email "pipelinesbot.noreply@example.com"
git config --global user.name "Pipelines-Bot"
git add --all
git commit -m"Pipelines-Bot: Updated repo (via build_docs script) Source Version is $2";
git push https://$1@github.com/cgilliard/myfamily.git