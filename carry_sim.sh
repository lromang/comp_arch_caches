#! /bin/bash

# Starting conditions
cache_size=4
hit_rate_prev=0
assoc=$(($cache_size/4))

# First evaluation
sim=$(./sim -bs 16 -a $assoc -wb -wa -ds $cache_size -is $cache_size  ./trazas/spice.trace | grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
row=$(echo $sim | sed 's/ /,/g')
echo "cache_size, block_size, associativity, inst_hit, data_hit" > block_size.csv
echo "$cache_size,4,$assoc,$row" >> block_size.csv

# While
flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)

while [ $flag -eq 0 ]
do
    # Update hit_rate, cache_size, assoc, sim and row
    hit_rate_prev=$(echo $sim | cut -d ' ' -f1)
    cache_size=$(($cache_size*2))
    assoc=$(($cache_size/4))
    sim=$(./sim -bs 4 -a $assoc -wb -wa -ds $cache_size -is $cache_size  ./trazas/spice.trace |
                 grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
    row=$(echo $sim | sed 's/ /,/g')

    # Write results
    echo "$cache_size,4,$assoc,$row" >> block_size.csv

    echo $hit_rate_prev
    echo $(echo $sim | cut -d ' ' -f1)
    # Update flag
    flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)
    echo $flag
done
