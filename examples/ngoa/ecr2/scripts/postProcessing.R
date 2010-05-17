#!/usr/bin/Rscript --vanilla --default-packages=utils
#
# R script for post processing of data extracted from OMNeT++ scala files
# for the simulation of candidate NGOA architectures
#
# (C) 2010 Kyeong Soo (Joseph) Kim
#


# load add-on packages and external source files
library(plyr);
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/calculateCI.R")
source("~/inet-hnrl/examples/ngoa/ecr2/scripts/ggplot2Arrange.R")


#########################################################################
# processing data from DedicatedAccess simulation
#########################################################################

setwd("~/inet-hnrl/examples/ngoa/ecr2/results/DedicatedAccess/N1")
df <- read.csv("N1_br1000_rtt10.data", header=TRUE)
da <- df[order(df$N, df$n, df$dr, df$br, df$rtt, df$repetition), ]
da.http.delay <- ddply(da[,c('n', 'dr', 'repetition', 'http.delay')], c(.(n), .(dr)),
    function(df) data.frame(mean=mean(df$http.delay, na.rm=T), ci.width=CalculateCI(df$http.delay)))
da.ftp.delay <- ddply(da[,c('n', 'dr', 'repetition', 'ftp.delay')], c(.(n), .(dr)),
    function(df) data.frame(mean=mean(df$ftp.delay, na.rm=T), ci.width=CalculateCI(df$ftp.delay)))
da.video.dfr <- ddply(da[,c('n', 'dr', 'repetition', 'video.dfr')], c(.(n), .(dr)),
    function(df) data.frame(mean=mean(df$video.dfr, na.rm=T), ci.width=CalculateCI(df$video.dfr)))

# plot xxx to a pdf file
pdf(file="http_delay.pdf", width=10, height=10)
#p.http <- ggplot(da.http.delay, aes(y=mean, x=dr))
#p.http + geom_point() + geom_errorbar(limits, width=0.1) + facet_wrap(~ n, ncol = 2)
p.http <- ggplot(da.http.delay, aes(group=n, colour=factor(n), y=mean, x=dr)) 
p.http + geom_point(aes(group=n)) + geom_errorbar(limits, width=0.1) +
    scale_y_continuous("HTTP Average Session Delay [s]") + scale_colour_grey("Number of Hosts per ONU")
#p.ftp <- ggplot(da.ftp.delay, aes(y=mean, x=dr))
#p.ftp + geom_point() + geom_errorbar(limits, width=0.1) + facet_wrap(~ n, ncol = 2)
p.ftp <- ggplot(da.ftp.delay, aes(group=n, colour=factor(n), y=mean, x=dr))
p.ftp + geom_point(aes(group=n)) + geom_errorbar(limits, width=0.1)
#p.video <- ggplot(da.video.dfr, aes(y=mean, x=dr))
#p.video + geom_point() + geom_errorbar(limits, width=0.1) + facet_wrap(~ n, ncol = 2)
p.video <- ggplot(da.video.dfr, aes(group=n, colour=factor(n), y=mean, x=dr))
p.video + geom_point(aes(group=n)) + geom_errorbar(limits, width=0.1)
arrange(p.http, p.ftp, p.video, ncol=1)
dev.off();


#########################################################################
# processing data from HybridPon simulation
#########################################################################

#setwd("~/inet-hnrl/examples/ngoa/ecr2/results/HybridPon")
#df <- read.csv("N16_dr10_br1000_rtt10.data", header=TRUE);
#hp <- df[order(df$N, df$n, df$dr, df$tx, df$br, df$rtt, df$repetition], ]
#hp.http.delay <- ddply(hp[,c('n', 'dr', 'tx', 'repetition', 'http.delay')], c(.(n), .(dr)), function(df) data.frame(mean=mean(df$http.delay), ci.width=CalculateCI(df$http.delay)))
## hp.ftp.delay <- ddply(da[,c('n', 'dr', 'tx', 'repetition', 'ftp.delay')], c(.(N), .(n), .(dr)), mean)
## hp.video.dfr <- ddply(da[,c('n', 'dr', 'tx', 'repetition', 'video.dfr')], c(.(N), .(n), .(dr)), mean)
