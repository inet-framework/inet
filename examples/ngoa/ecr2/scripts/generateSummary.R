#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for generating summary plots and statistics of data
### extracted from OMNeT++ scala files for the simulation of candidate
### NGOA architectures
###
### (C) 2010 Kyeong Soo (Joseph) Kim
###


### load add-on packages and external source files
library(fda)
library(ggplot2)
library(plyr)
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/calculateCI.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/getMonotoneSpline.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/ggplot2Arrange.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/groupMeansAndCIs.R")


### define variables and functions
## working directories for dedicated access (da) and hybrid PON (hp)
.da.wd <- "~/inet-hnrl/examples/ngoa/ecr2/results/DedicatedAccess/N16"
.hp.wd <- "~/inet-hnrl/examples/ngoa/ecr2/results/HybridPon"

## base names (without extensions) of data files
.da.base <- "N16_br1000_rtt10"
.hp.base <- "N16_dr10_br1000_rtt10"

## misc.
.ylabels <- c("Average Delay of FTP Sessions [sec]",
              "Average Throughput of FTP Sessions [Byte/sec]",
              "Mean Transfer Rate of FTP Sessions [Byte/sec]",
              "Average Delay of HTTP Sessions [sec]",
              "Average Throughput of HTTP Sessions [Byte/sec]",
              "Mean Transfer Rate of HTTP Sessions [Byte/sec]",
              "Average Decodable Frame Rate of Streaming Videos (Q)")


#########################################################################
### processing data from DedicatedAccess simulation
#########################################################################
#setwd(.da.wd)
.da.data <- paste(.da.wd, paste(.da.base, "data", sep="."), sep="/")
.df <- read.csv(.da.data, header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame
.da.df <- ddply(.df, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})

## generate plots
.da.plots <- list()
for (.i in 1:7) {
    .df <- subset(.da.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(gropu=n, colour=factor(n), x=dr, y=mean)) + geom_line()
    .p <- .p + xlab("Line Rate [Gbps]") + ylab(.ylabels[.i])
    .p <- .p + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean))
    .p <- .p + geom_errorbar(.limits, width=0.1)
    ## print(.p)
    .da.plots[[.i]] <- .p
}

## save plots to a pdf file
.da.pdf <- paste(.da.wd, paste(.da.base, "pdf", sep="."), sep="/")
pdf(file=.da.pdf, paper="letter")
arrange(.da.plots[[1]], .da.plots[[2]], .da.plots[[3]], ncol=1)
arrange(.da.plots[[4]], .da.plots[[5]], .da.plots[[6]], ncol=1)
print(.da.plots[[7]])
dev.off()

## save important objects to a file for later use
.da.save <- paste(.da.wd, paste(.da.base, "RData", sep="."), sep="/")
save(.da.df, .da.plots, file=.da.save)


#########################################################################
### processing data from HybridPon simulation
#########################################################################
#setwd(.hp.wd)
.hp.data <- paste(.hp.wd, paste(.hp.base, "data", sep="."), sep="/")
.df <- read.csv(.hp.data, header=TRUE);
.df <- .df[order(.df$N, .df$n, .df$dr, .df$tx, .df$br, .df$rtt, .df$repetition), ] # order data frame
.hp.df <- ddply(.df, c(.(n), .(tx)), function(df) {return(GetMeansAndCiWidths(df))})

## generate plots
.hp.plots <- list()
for (.i in 1:7) {
    .df <- subset(.hp.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=tx, colour=factor(tx), x=n, y=mean)) + geom_line()
    .p <- .p + xlab("Number of Hosts per ONU") + ylab(.ylabels[.i])
    .p <- .p + geom_point(aes(group=tx, colour=factor(tx), x=n, y=mean))
    .p <- .p + geom_errorbar(.limits, width=0.1)
    ## print(.p)
    .hp.plots[[.i]] <- .p
}

## save plots to a pdf file
.hp.pdf <- paste(.hp.wd, paste(.hp.base, "pdf", sep="."), sep="/")
pdf(file=.hp.pdf, paper="letter")
arrange(.hp.plots[[1]], .hp.plots[[2]], .hp.plots[[3]], ncol=1)
arrange(.hp.plots[[4]], .hp.plots[[5]], .hp.plots[[6]], ncol=1)
print(.hp.plots[[7]])
dev.off()

## save important objects to a file for later use
.hp.save <- paste(.hp.wd, paste(.hp.base, "RData", sep="."), sep="/")
save(.hp.df, .hp.plots, file=.hp.save)
