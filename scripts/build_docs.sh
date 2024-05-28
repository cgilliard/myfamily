#!/bin/bash

echo "Building docs";
make doc
git pull
git clone https://anything:$(GithubPat)@github.com/cgilliard/myfamily.git myfamilydocs
cd myfamilydocs
rm -rf docs/html
cp -rp ../doc/html/* docs/html
git config user.name "Pipelines-Bot"
git config --global user.email "pipelinesbot.noreply@example.com"
git config --global user.name "Pipelines-Bot"
git push https://$(GithubPat)@github.com/cgilliard/myfamily.git
