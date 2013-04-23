#!/bin/bash

DATAFILE=${1:-nohup.out}

gnuplot <<-END
set terminal dumb
#set timefmt "%Y-%m-%d %H:%M:%S"
#set datafile separator ","
plot "< awk '{ print \$2 }' $DATAFILE" title "Temperature (c)" with lines
END
