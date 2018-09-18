#!/bin/bash

if type clang-format-5.0 2> /dev/null; then
	CLANG_FORMAT=clang-format-5.0
elif type clang-format 2> /dev/null; then
	VERSION=$(clang-format --version)
	if [[ $VERSION != *5.0* ]]; then
		echo "clang-format is not 5.0 (returned ${VERSION})"
		exit 1
	fi
	CLANG_FORMAT=clang-format
else
	echo "clang-format 5.0 was not found"
	exit 1
fi

$CLANG_FORMAT -i --style=file src/*.c src/*.h
