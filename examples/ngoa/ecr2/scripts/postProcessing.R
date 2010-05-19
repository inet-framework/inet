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
.da.wd <- "~/inet-hnrl/examples/ngoa/ecr2/results/DedicatedAccess/N1"
.hp.wd <- "~/inet-hnrl/examples/ngoa/ecr2/results/HybridPon"

## pdf file names
.da.pdf <- "dedicatedAccess.pdf"
.hp.pwd <- "hybridPon.pdf"

## utility functions
GetMeanAndCiWidth <- function(df, col) {
###
### Computes the sample mean and the confidence interval half-width.
###
### Args:
###   df:   a data frame.
###   col:  the column of the data frame whose mean and confidence interval
###         half-width are to be calculated.
### Returns:
###   The sample mean and the half-width of 95-percent confidence interval
###   df$col.
###
    return (data.frame(mean=mean(df[col], na.rm=TRUE),
                       ci.width=CalculateCI(df[col])))
}


#########################################################################
### processing data from DedicatedAccess simulation
#########################################################################
setwd(.da.wd)
.df <- read.csv("N1_br1000_rtt10.data", header=TRUE)
.df <- .df[order(.df$N, .df$n, .df$dr, .df$br, .df$rtt, .df$repetition), ] # order data frame

## debug (later use in the interactive session)
da.http.delay <- ddply(.df[,c('n', 'dr', 'repetition', 'http.delay')], c(.(n), .(dr)),
                       function(df) {return(GetMeanAndCiWidth(df, col='http.delay'))})

## get monotone spline curves
.x <- seq(1, 10, 0.1)
.l <- length(.x)
.n.range <- unique(da.http.delay$n)
for (.n in 1:10) {
    .dr <- subset(.df, n==.n)$dr
    .dly <- subset(.df, n==.n)$http.delay
    .res <- getMonotoneSpline(.dr, .dly)
    .y <- .res$beta[1] + .res$beta[2]*eval.monfd(.x, .res$Wfdobj)
    da.http.delay <- rbind.fill(list(da.http.delay, data.frame(n=rep(.n, .l), dr=.x, interpolated=.y)))
}
da.http.delay <- merge(remove_missing(da.http.delay[c('n', 'dr', 'mean', 'ci.width')]),
                       remove_missing(da.http.delay[c('n', 'dr', 'interpolated')]), all=T)  # merge to remove duplcated rows with NAs

## plot to a pdf file
pdf(file=paste(.da.wd, .da.pdf, sep="/"), width=10, height=10)
.limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
p.da.http <- ggplot(data=da.http.delay, aes(gropu=n, colour=factor(n), x=dr, y=interpolated)) + geom_line()
p.da.http <- p.da.http + geom_point(aes(group=n, colour=factor(n), x=dr, y=mean))
p.da.http <- p.da.http + geom_errorbar(.limits, width=0.1)
print(p.da.http)
dev.off()

## p.http <- ggplot(da.http.delay, aes(y=mean, x=dr))
## p.http + geom_point() + geom_errorbar(limits, width=0.1) + facet_wrap(~ n, ncol = 2)
## arrange(p.http, p.ftp, p.video, ncol=1)


#########################################################################
### processing data from HybridPon simulation
#########################################################################
setwd(.hp.wd)
.df <- read.csv("N16_dr10_br1000_rtt10.data", header=TRUE);
.df <- .df[order(.df$N, .df$n, .df$dr, .df$tx, .df$br, .df$rtt, .df$repetition), ] # order data frame

## debug (later use in the interactive session)
hp.http.delay <- ddply(.df[,c('n', 'dr', 'tx', 'repetition', 'http.delay')], c(.(tx), .(n)),
                       function(df) {return(GetMeanAndCiWidth(df, col='http.delay'))})
## hp.ftp.delay <- ddply(da[,c('n', 'dr', 'tx', 'repetition', 'ftp.delay')], c(.(N), .(n), .(dr)), mean)
## hp.video.dfr <- ddply(da[,c('n', 'dr', 'tx', 'repetition', 'video.dfr')], c(.(N), .(n), .(dr)), mean)

## plot to a pdf file
pdf(file=paste(.hp.wd, .hp.pdf, sep="/"), width=10, height=10)
.limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
p.hp.http <- ggplot(data=hp.http.delay, aes(group=tx, colour=factor(tx), x=n, y=mean)) + geom_line()
p.hp.http <- p.hp.http + geom_point(aes(group=tx, colour=factor(tx), x=n, y=mean))
p.hp.http <- p.hp.http + geom_errorbar(.limits, width=0.1)
print(p.hp.http)
dev.off()
