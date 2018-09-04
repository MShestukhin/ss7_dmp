#!/bin/bash

if [[ -n $(find ./ -name "dmp" -mmin +5) ]]
then
    echo 'file exist'
fi
