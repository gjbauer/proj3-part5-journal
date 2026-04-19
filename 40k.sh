#!/bin/bash

printf '=This string is fourty characters long.=%.0s' {1..1000} > mnt/hello.txt

cat mnt/hello.txt
