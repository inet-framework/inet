#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for post processing of data extracted from OMNeT++ scala files
### for the simulation of candidate NGOA architectures
###
### (C) 2011 Kyeong Soo (Joseph) Kim
###


### load add-on packages and external source files
##library(ggplot2)
library(plyr)
##library(xtable)
ifelse(Sys.getenv("OS") == "Windows_NT",
       .base.directory <- paste(Sys.getenv("INET-HNRL"), "examples/ngoa/comparison_framework", sep="/"),
       .base.directory <- "~/inet-hnrl/examples/ngoa/comparison_framework")
source(paste(.base.directory, "scripts/calculateCI.R", sep="/"))
## source(paste(.base.directory, "scripts/getMonotoneSpline.R", sep="/"))
## source(paste(.base.directory, "scripts/ggplot2Arrange.R", sep="/"))
source(paste(.base.directory, "scripts/groupMeansAndCIs.R", sep="/"))

### set directories for data files ("*.data") extracting simulation results from OMNeT++ scala files
## .rf_N1.wd <- paste(.base.directory, "results/Reference/N1", sep="/")
## .hp_N16.wd <- paste(.base.directory, "results/HybridPon/N16_10h", sep="/")
## .hp_N32.wd <- paste(.base.directory, "results/HybridPon/N32_10h", sep="/")
## .rf_N1.base <- "N1"
## .hp_N16.base <- "N16_10h"
## .hp_N32.base <- "N32_10h"
.wds <- c(paste(.base.directory, "results/Reference/N1", sep="/"),
          paste(.base.directory, "results/HybridPon/N16_10h", sep="/"),
          paste(.base.directory, "results/HybridPon/N32_10h", sep="/"))
.bases <- c("N1", "N16_10h", "N32_10h")
## .labels.traffic <- c("FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
## .labels.measure <- c("Average Session Delay [sec]",
##                      "Average Session Throughput [Byte/sec]",
##                      "Mean Session Transfer Rate [Byte/sec]",
##                      "Average Session Delay [sec]",
##                      "Average Session Throughput [Byte/sec]",
##                      "Mean Session Transfer Rate [Byte/sec]",
##                      "Average Decodable Frame Rate (Q)")
##
## ### customize
## .old <- theme_set(theme_bw())
## .pt_size <- 3.5


### process data files and export them into csv files for further processing
##for (.i in 1:3) {
for (.i in 1:2) {
    .data <- paste(.wds[.i], paste(.bases[.i], "data", sep="."), sep="/")
    .df <- read.csv(.data, header=TRUE)
    if (.i == 1) {
        ## order data frame for reference
        .df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$repetition), ]
        .df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})
    }
    else {
        ## order data frame for hybrid pon
        .df <- .df[order(.df$N, .df$n, .df$tx, .df$br, .df$repetition), ]
        .df <- ddply(.df, c(.(n), .(tx)), function(df) {return(GetMeansAndCiWidths(df))})
    }
    write.table(.df, file=paste(.wds[.i], paste(.bases[.i], "csv", sep="."), sep="/"),
                sep = ",", row.names = FALSE, col.names = TRUE, qmethod="double")
}
