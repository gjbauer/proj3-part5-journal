#!/bin/bash

mkdir mnt/numbers

for n in {1..300}; 
do
    echo "$n" > "mnt/numbers/"$n".nums"
done

for n in {1..30}; 
do
    cat "mnt/numbers/"$((n*10))".nums"
done

for n in {1..4};
do
	cat "mnt/numbers/"$((n*7))".nums"
done

ls mnt/numbers | wc -l

for n in {1..4};
do
	rm "mnt/numbers/"$((n*7))".nums"
done

ls mnt/numbers | wc -l
