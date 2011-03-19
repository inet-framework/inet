#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for generating summary plots for all performance measures
### in the data extracted from OMNeT++ scala files for the simulation
### of candidate NGOA architectures
###
### (C) 2011 Kyeong Soo (Joseph) Kim
###


### load add-on packages and external source files
library(ggplot2)
library(plyr)
library(xtable)
ifelse (Sys.getenv("OS") == "Windows_NT",
        .base.directory <- "E:/tools/omnetpp/inet-hnrl/examples/ngoa/comparison_framework",
        .base.directory <- "~/inet-hnrl/examples/ngoa/comparison_framework")
source(paste(.base.directory, "scripts/calculateCI.R", sep="/"))
source(paste(.base.directory, "scripts/getMonotoneSpline.R", sep="/"))
source(paste(.base.directory, "scripts/ggplot2Arrange.R", sep="/"))
source(paste(.base.directory, "scripts/groupMeansAndCIs.R", sep="/"))

### define variables and functions
.rf_N1.wd <- paste(.base.directory, "results/Reference/N1", sep="/")
.rf_N1.base <- "N1"
.labels.traffic <- c("FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
.labels.measure <- c("Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Decodable Frame Rate (Q)")

### customize
.old <- theme_set(theme_bw())
.pt_size <- 3.5

### generate summary plots for reference architecture with N=1
.rf_N1.data <- paste(.rf_N1.wd, paste(.rf_N1.base, "data", sep="."), sep="/")
.df <- read.csv(.rf_N1.data, header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame
.rf_N1.df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})
.rf_N1.plots <- list()
for (.i in 1:7) {
    .df <- subset(.rf_N1.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line()
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
    ## .p <- .p + geom_point(aes(group=dr, colour=factor(dr), x=n, y=mean), size=.pt_size)
    .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("dr", values=0:9)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("dr")
    .rf_N1.plots[[.i]] <- .p
}

### generate summary plots for hybrid PON with N=16
.hp.data <- paste(.hp.wd, paste(.hp.base, "data", sep="."), sep="/")
.hp.org <- read.csv(.hp.data, header=TRUE);
.hp.org <- .hp.org[order(.hp.org$N, .hp.org$n, .hp.org$dr, .hp.org$tx, .hp.org$br, .hp.org$rtt, .hp.org$repetition), ] # order data frame
.hp.df <- ddply(.hp.org, c(.(n), .(tx)), function(df) {return(GetMeansAndCiWidths(df))})
.hp.plots <- list()
for (.i in 1:7) {
    .df <- subset(.hp.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=tx, colour=factor(tx), x=n, y=mean)) + geom_line()
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
    ## .p <- .p + geom_point(aes(group=tx, colour=factor(tx), x=n, y=mean), size=.pt_size)
    .p <- .p + geom_point(aes(group=tx, shape=factor(tx), x=n, y=mean), size=.pt_size) + scale_shape_manual(expression(N[tx]), values=0:4)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete(expression(N[tx]))
    .hp.plots[[.i]] <- .p
}
