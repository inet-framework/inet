#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for post processing of data extracted from OMNeT++ scala files
### for the simulation of candidate NGOA architectures
###
### (C) 2010 Kyeong Soo (Joseph) Kim
###


### load add-on packages and external source files
library(fda)
library(ggplot2)
library(plyr)
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/calculateCI.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/ggplot2Arrange.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/getMonotoneSpline.R")


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

## utility functions
GetMeanAndCiWidth <- function(df, col) {
###
### Computes the sample mean and the confidence interval half-width.
###
### Args:
###   df:   a data frame.
###   col:  the column of the data frame whose mean and confidence interval
###         half-width are to be calculated.
###
### Returns:
###   The sample mean and the half-width of 95-percent confidence interval
###   df$col.
###
    return (data.frame(mean=mean(df[col], na.rm=TRUE),
                       ci.width=CalculateCI(df[col])))
}

GetMeansAndCiWidths <- function(df) {
###
### Computes the sample means and the confidence interval half-widths.
###
### Args:
###   df:   a data frame.
###
### Returns:
###   The sample mean and the half-width of 95-percent confidence interval
###   df$col.
###
    return (
            data.frame(
                       ftp.delay.mean=mean(df$ftp.delay, na.rm=TRUE),
                       ftp.delay.ci.width=CalculateCI(df$ftp.delay),
                       ftp.throughput.mean=mean(df$ftp.throughput, na.rm=TRUE),
                       ftp.throughput.ci.width=CalculateCI(df$ftp.throughput),
                       ftp.transferrate.mean=mean(df$ftp.transferrate, na.rm=TRUE),
                       ftp.transferrate.ci.width=CalculateCI(df$ftp.transferrate),
                       http.delay.mean=mean(df$http.delay, na.rm=TRUE),
                       http.delay.ci.width=CalculateCI(df$http.delay),
                       http.throughput.mean=mean(df$http.throughput, na.rm=TRUE),
                       http.throughput.ci.width=CalculateCI(df$http.throughput),
                       http.transferrate.mean=mean(df$http.transferrate, na.rm=TRUE),
                       http.transferrate.ci.width=CalculateCI(df$http.transferrate),
                       video.dfr.mean=mean(df$video.dfr, na.rm=TRUE),
                       video.dfr.ci.width=CalculateCI(df$video.dfr)
                       )
            )
}


#########################################################################
### processing data from DedicatedAccess simulation
#########################################################################
setwd(.da.wd)
.da.data <- paste(.da.wd, paste(.da.base, "data", sep="."), sep="/")
.da.org <- read.csv(.da.data, header=TRUE)
.da.org <- .da.org[order(.da.org$N, .da.org$n, .da.org$dr, .da.org$br, .da.org$rtt, .da.org$repetition), ] # order data frame
.da.df <- ddply(.da.org, c(.(n), .(dr)), function(df) {return(GetMeansAndCiWidths(df))})

## generate plots
.da.plots <- list()
.da.dfs <- list()
.x <- seq(1, 10, 0.1)
.l <- length(.x)
.n.range <- unique(da.df$n)
## for (.i in 1:7) {
for (.i in 4:4) {    
    ## .df <- subset(hp.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    .df <- subset(.da.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")

    ## get monotone spline curves
    for (.n in .n.range) {
        .dr <- subset(.da.org, n==.n)$dr
        .measure <- subset(.da.org, n==.n)[[.i+6]]
        .res <- getMonotoneSpline(.dr, .measure)
        .y <- .res$beta[1] + .res$beta[2]*eval.monfd(.x, .res$Wfdobj)
        .df <- rbind.fill(list(.df, data.frame(n=rep(.n, .l), dr=.x, interpolated=.y)))
    }
    .df <- merge(remove_missing(.df[c('n', 'dr', 'mean', 'ci.width')]),
                 remove_missing(.df[c('n', 'dr', 'interpolated')]), all=T)  # merge to remove duplicated rows with NAs
    .da.dfs[[.i]] <- .df

    .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
    .p <- ggplot(data=.df, aes(gropu=n, colour=factor(n), x=dr, y=interpolated)) + geom_line()
    .p <- .p + xlab("Number of Hosts per ONU") + ylab(.ylabels[.i])
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

## ## get monotone spline curves
## .x <- seq(1, 10, 0.1)
## .l <- length(.x)
## .n.range <- unique(da.df$n)
## for (.n in 1:10) {
##     .dr <- subset(.df, n==.n)$dr
##     .dly <- subset(.df, n==.n)$http.delay
##     .res <- getMonotoneSpline(.dr, .dly)
##     .y <- .res$beta[1] + .res$beta[2]*eval.monfd(.x, .res$Wfdobj)
##     da.http.delay <- rbind.fill(list(da.http.delay, data.frame(n=rep(.n, .l), dr=.x, interpolated=.y)))
## }
## da.http.delay <- merge(remove_missing(da.http.delay[c('n', 'dr', 'mean', 'ci.width')]),
##                        remove_missing(da.http.delay[c('n', 'dr', 'interpolated')]), all=T)  # merge to remove duplcated rows with NAs

## ## generate plots
## da.plots <- list()  # debug
## .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
## p.da.http <- ggplot(data=da.http.delay, aes(gropu=n, colour=factor(n), x=dr, y=interpolated)) + geom_line()
## p.da.http <- p.da.http + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean))
## p.da.http <- p.da.http + geom_errorbar(.limits, width=0.1)
## print(p.da.http)
## dev.off()

## ## save plots to a pdf file
## pdf(file=paste(.da.wd, .da.pdf, sep="/"), width=10, height=10)
## ## p.http <- ggplot(da.http.delay, aes(y=mean, x=dr))
## ## p.http + geom_point() + geom_errorbar(limits, width=0.1) + facet_wrap(~ n, ncol = 2)
## ## arrange(p.http, p.ftp, p.video, ncol=1)
## dev.off()

## save important objects to a file for later use
.da.save <- paste(.da.wd, paste(.da.base, "RData", sep="."), sep="/")
save(.da.df, .da.dfs, .da.plots, file=.da.save)


#########################################################################
### processing data from HybridPon simulation
#########################################################################
setwd(.hp.wd)
.hp.data <- paste(.hp.wd, paste(.hp.base, "data", sep="."), sep="/")
.df <- read.csv(.hp.data, header=TRUE);
.df <- .df[order(.df$N, .df$n, .df$dr, .df$tx, .df$br, .df$rtt, .df$repetition), ] # order data frame
.hp.df <- ddply(.df, c(.(n), .(tx)), function(df) {return(GetMeansAndCiWidths(df))})

## generate plots
.hp.plots <- list()
for (.i in 1:7) {
    .df <- subset(hp.df, select = c(1, 2, (.i*2+1):((.i+1)*2)))
    names(.df)[3:4] <- c("mean", "ci.width")
    .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
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
