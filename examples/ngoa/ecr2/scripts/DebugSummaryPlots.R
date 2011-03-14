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
        .base.directory <- "E:/tools/omnetpp/inet-hnrl/examples/ngoa/ecr2",
        .base.directory <- "~/inet-hnrl/examples/ngoa/ecr2")
source(paste(.base.directory, "scripts/calculateCI.R", sep="/"))
source(paste(.base.directory, "scripts/getMonotoneSpline.R", sep="/"))
source(paste(.base.directory, "scripts/ggplot2Arrange.R", sep="/"))
source(paste(.base.directory, "scripts/groupMeansAndCIs.R", sep="/"))

### define variables and functions
.debug.wd <- paste(.base.directory, "results/TestOfDedicatedAccess/Debug", sep="/")
.debug_3.base <- "Debug_3"
.debug_4.base <- "Debug_4"
.N1.wd <- paste(.base.directory, "results/TestOfDedicatedAccess/N1_br1000_rtt10", sep="/")
.N1_hv.base <- "N1_br1000_rtt10_http_video"
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

### generate summary plots for dedicated access with Debug_3 configuration
.debug_3.data <- paste(.debug.wd, paste(.debug_3.base, "data", sep="."), sep="/")
.df <- read.csv(.debug_3.data, header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame
.debug_3.df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})
.debug_3.plots <- list()
for (.i in 1:7) {
    .df <- subset(.debug_3.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    if (prod(is.na(.df$mean)) == 1) {
        ## all values are missing
        .debug_3.plots[[.i]] <- FALSE
    }
    else {
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=n, colour=factor(n), x=dr, y=mean)) + geom_line()
        ## .p <- .p + xlab("Line Rate [Gbps]") + ylab(.labels.measure[.i])
        .p <- .p + xlab("Line Rate [Mbps]") + ylab(.labels.measure[.i])
        ## .p <- .p + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean), size=.pt_size)
        .p <- .p + geom_point(aes(group=n, shape=factor(n), x=dr, y=mean), size=.pt_size) + scale_shape_manual("n", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("n")
        .debug_3.plots[[.i]] <- .p
    }
}

### generate summary plots for dedicated access with Debug_4 configuration
.debug_4.data <- paste(.debug.wd, paste(.debug_4.base, "data", sep="."), sep="/")
.df <- read.csv(.debug_4.data, header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame
.debug_4.df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})
.debug_4.plots <- list()
for (.i in 1:7) {
    .df <- subset(.debug_4.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    if (prod(is.na(.df$mean)) == 1) {
        ## all values are missing
        .debug_4.plots[[.i]] <- FALSE
    }
    else {
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=n, colour=factor(n), x=dr, y=mean)) + geom_line()
        .p <- .p + xlab("Line Rate [Mbps]") + ylab(.labels.measure[.i])
        ## .p <- .p + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean), size=.pt_size)
        .p <- .p + geom_point(aes(group=n, shape=factor(n), x=dr, y=mean), size=.pt_size) + scale_shape_manual("n", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("n")
        .debug_4.plots[[.i]] <- .p
    }
}

### generate summary plots for dedicated access with N1_br1000_rtt10_http_video configuration
.N1_hv.data <- paste(.N1.wd, paste(.N1_hv.base, "data", sep="."), sep="/")
.df <- read.csv(.N1_hv.data, header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame
.N1_hv.df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})
.N1_hv.plots <- list()
for (.i in 1:7) {
    .df <- subset(.N1_hv.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    if (prod(is.na(.df$mean)) == 1) {
        ## all values are missing
        .N1_hv.plots[[.i]] <- FALSE
    }
    else {
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=n, colour=factor(n), x=dr, y=mean)) + geom_line()
        .p <- .p + xlab("Line Rate [Gbps]") + ylab(.labels.measure[.i])
        ## .p <- .p + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean), size=.pt_size)
        .p <- .p + geom_point(aes(group=n, shape=factor(n), x=dr, y=mean), size=.pt_size) + scale_shape_manual("n", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("n")
        .N1_hv.plots[[.i]] <- .p
    }
}
