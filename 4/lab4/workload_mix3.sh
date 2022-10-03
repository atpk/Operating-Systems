#!/bin/sh 
./arithoh.sh &
./pipe.sh &
./arithoh.sh &
./pipe.sh &
wait
