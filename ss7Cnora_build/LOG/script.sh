#!/usr/bin/env bash
my=$(stat ./2018-05-03.log -c %s)
if (($my>0)); then
	echo $my
else
	echo "not bad"
fi
