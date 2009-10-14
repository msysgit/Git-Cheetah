#!/bin/sh
kill $(ps ax  | grep Finder | grep -v grep | awk '{ print $1 }')
