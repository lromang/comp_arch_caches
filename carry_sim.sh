#! /bin/bash

## ---------------------------
## Impact of Cache Size
## ---------------------------

## Starting conditions
cache_size=8
hit_rate_prev=0
assoc=$(($cache_size/4))
block_size=4

## name of trace file
trace=$1
var=$(echo $1 | awk -F '.' '{print $1}')
file="output/cache_size_$var.csv"

## First evaluation

sim=$(./sim -bs $block_size -a $assoc -wb -wa -ds $cache_size -is $cache_size  "./trazas/$trace" |
             grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
row=$(echo $sim | sed 's/ /,/g')
echo "cache_size, block_size, associativity, inst_hit, data_hit" > $file
echo "$cache_size,$block_size,$assoc,$row" >> $file

## While
flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)

while [ $flag -eq 0 ]
do
    ## Update hit_rate, cache_size, assoc, sim and row
    hit_rate_prev=$(echo $sim | cut -d ' ' -f1)
    cache_size=$(($cache_size*2))
    assoc=$(($cache_size/4))
    sim=$(./sim -bs $block_size -a $assoc -wb -wa -ds $cache_size -is $cache_size  "./trazas/$trace" |
                 grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
    row=$(echo $sim | sed 's/ /,/g')

    ## Write results
    echo "$cache_size,$block_size,$assoc,$row" >> $file

    ## Update flag
    flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)
done

## ---------------------------
## Impact of Block Size
## ---------------------------

## Starting conditions
cache_size=8192
hit_rate_prev=0
assoc=2
block_size=4
file="output/block_size_$var.csv"

## File header
echo "cache_size, block_size, associativity, inst_hit, data_hit" > $file

## While
while [ $block_size -lt 4096 ]
do
    ## Update hit_rate, cache_size, assoc, sim and row
    sim=$(./sim -bs $block_size -a $assoc -wb -wa -ds $cache_size -is $cache_size  "./trazas/$trace" |
                 grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
    hit_rate_prev=$(echo $sim | cut -d ' ' -f1)
    row=$(echo $sim | sed 's/ /,/g')
    block_size=$(($block_size*2))

    ## Write results
    echo "$cache_size,$block_size,$assoc,$row" >> $file

    ## Update flag
    flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)
done

## ---------------------------
## Impact of Associativity
## ---------------------------

## Starting conditions
cache_size=8192
hit_rate_prev=0
assoc=1
block_size=128
file="output/associativity_$var.csv"

## File header
echo "cache_size, block_size, associativity, inst_hit, data_hit" > $file

## While
while [ $assoc -lt 64 ]
do
    ## Update hit_rate, cache_size, assoc, sim and row
    sim=$(./sim -bs $block_size -a $assoc -wb -wa -ds $cache_size -is $cache_size  "./trazas/$trace" |
                 grep hit | awk -F '(' '{print $2}' | sed -r 's/[^0-9.]//g')
    hit_rate_prev=$(echo $sim | cut -d ' ' -f1)
    row=$(echo $sim | sed 's/ /,/g')
    assoc=$(($assoc*2))

    ## Write results
    echo "$cache_size,$block_size,$assoc,$row" >> $file

    ## Update flag
    flag=$(echo $hit_rate_prev '==' $(echo $sim | cut -d ' ' -f1) | bc -l)
done
## ---------------------------
## Memory Bandwidth (write)
## ---------------------------
## Starting conditions
cache_size=8192 #16384
hit_rate_prev=0
assoc=2 #4
block_size=128 #64
file="output/bandWidth_write_$var.csv"


## File header
echo "cache_size, block_size, associativity, write_policy , demand_fetch, copies_back" > $file

## for
for  i in `seq 1`
do
    ## Write through
    sim_through=$(../mike/sim -bs $block_size -a $assoc -wt -nw -ds $cache_size -is $cache_size  "./trazas/$trace" |
                         grep -E "(fetch|back)" | awk -F ':' '{print $2}' | sed -r 's/[^0-9.]//g')
    row_through=$(echo $sim_through | sed 's/ /,/g')

    ## Write back
    sim_back=$(../mike/sim -bs $block_size -a $assoc -wb -nw -ds $cache_size -is $cache_size  "./trazas/$trace" |
                         grep -E "(fetch|back)" | awk -F ':' '{print $2}' | sed -r 's/[^0-9.]//g')
    row_back=$(echo $sim_back | sed 's/ /,/g')

    ## Write results
    echo "$cache_size,$block_size,$assoc,wt,$row_through" >> $file
    echo "$cache_size,$block_size,$assoc,wb,$row_back"    >> $file
done

## ---------------------------
## Memory Bandwidth (allocate)
## ---------------------------
## Starting conditions
cache_size=16384 #16384
hit_rate_prev=0
assoc=2 #4
block_size=64 #64
file="output/bandWidth_allocate_$var.csv"

## File header
echo "cache_size, block_size, associativity, allocate_policy ,demand_fetch, copies_back" > $file

## for
for  i in `seq 1`
do
    ## Write through
    sim_allocate=$(../mike/sim -bs $block_size -a $assoc -wb -wa -ds $cache_size -is $cache_size  "./trazas/$trace" |
                         grep -E "(fetch|back)" | awk -F ':' '{print $2}' | sed -r 's/[^0-9.]//g')
    row_allocate=$(echo $sim_allocate | sed 's/ /,/g')

    ## Write back
    sim_non_allocate=$(../mike/sim -bs $block_size -a $assoc -wb -nw -ds $cache_size -is $cache_size  "./trazas/$trace" |
                         grep -E "(fetch|back)" | awk -F ':' '{print $2}' | sed -r 's/[^0-9.]//g')
    row_non_allocate=$(echo $sim_non_allocate | sed 's/ /,/g')

    ## Write results
    echo "$cache_size,$block_size,$assoc,wa,$row_allocate" >> $file
    echo "$cache_size,$block_size,$assoc,nw,$row_non_allocate"    >> $file
done
