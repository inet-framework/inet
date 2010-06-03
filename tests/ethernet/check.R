#!/usr/bin/R
#R file:

options(width=120)
library("omnetpp")

idlelimit <- 5.0
usedlimit <- 97.0


idle <- loadDataset(add(type='scalar', files='TwoHosts1-0.sca', select='name("rx channel idle *")'))
used <- loadDataset(add(type='scalar', files='TwoHosts1-0.sca', select='name("rx channel utilization *")'))

cat("\nOMNETPP TEST RESULT: ")

if(max(idle$scalars$value) <= idlelimit)
{
    cat("IDLE OK\n")
} else {
    cat("IDLE BAD:\n")
    print(idle$scalars[idle$scalars$value > idlelimit,])
}

cat("\nOMNETPP TEST RESULT: ")

if(min(used$scalars$value) >= usedlimit)
{
    cat("USED OK\n")
} else {
    cat("USED BAD:\n")
    print(used$scalars[used$scalars$value < usedlimit,])
}

cat("\n")

