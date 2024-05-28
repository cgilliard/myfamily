#!/bin/bash

echo "Building docs";
make doc
rm -rf docs/html/*
cp -rp doc/html/* docs/html
