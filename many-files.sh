#!/bin/bash

mkdir mnt/numbers

for n in {1..300}; 
do
    echo "$n" > "mnt/numbers/"$n".num"
done

for n in {1..30};
do
    cat "mnt/numbers/"$((n*10))".nums"
done

ls mnt/numbers | wc -l 

for n in {1..150};
do
	rm "mnt/"$((n*2))".num"
done

ls mnt/numbers | wc -l
