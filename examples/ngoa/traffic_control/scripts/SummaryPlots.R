#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for generating summary plots for all performance measures
### from OMNeT++ scala files for the simulation of NGN access networks
### with traffci control (i.e., shaping/policing)
###
### (C) 2012 Kyeong Soo (Joseph) Kim
###


### load add-on packages and external source files
library(ggplot2)
library(omnetpp)
#library(plyr)
library(reshape)
#library(xtable)
ifelse (Sys.getenv("OS") == "Windows_NT",
        .base.directory <- "E:/tools/omnetpp/inet-hnrl/examples/ngoa/traffic_control",
        .base.directory <- "~/inet-hnrl/examples/ngoa/traffic_control")
## source(paste(.base.directory, "scripts/calculateCI.R", sep="/"))
## source(paste(.base.directory, "scripts/getMonotoneSpline.R", sep="/"))
## source(paste(.base.directory, "scripts/ggplot2Arrange.R", sep="/"))
## source(paste(.base.directory, "scripts/groupMeansAndCIs.R", sep="/"))

### define variables
.da_nh1_nf0_nv0.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv0", sep="/")
.da_nh1_nf0_nv1.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv1", sep="/")
.da_nh1_nf0_nv0_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv0_tbf", sep="/")
## .da_dr100M_nh1_nf0_nv1.wd <- paste(.base.directory, "results/Dedicated/dr100M_nh1_nf0_nv1", sep="/")
## .da_nh1_nf0_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv1_tbf", sep="/")
.da_dr100M_nh1_nf0_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/dr100M_nh1_nf0_nv1_tbf", sep="/")
.labels.traffic <- c("FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
## .labels.measure <- c("Average Session Delay [sec]",
##                      "Average Session Throughput [Byte/sec]",
##                      "Mean Session Transfer Rate [Byte/sec]",
##                      "Average Session Delay [sec]",
##                      "Average Session Throughput [Byte/sec]",
##                      "Mean Session Transfer Rate [Byte/sec]",
##                      "Average Decodable Frame Rate (Q)")
.labels.measure <- c("Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Decodable Frame Rate (Q)")

### define functions
ci.width <- conf.int(0.95)                  # for 95% confidence interval

### customize
.old <- theme_set(theme_bw())
.pt_size <- 3.5

#################################################################################
### summary plots for dedicated architecture with HTTP traffic and no traffic
### shaping
#################################################################################
.df <- loadDataset(paste(.da_nh1_nf0_nv0.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue', subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, and mean session transfer rate
.tmp <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
             subset=grepl('.*\\.httpApp.*', module) &
             (name=='average session delay [s]' |
              name=='average session throughput [B/s]' |
              name=='mean session transfer rate [B/s]'))
.tmp <- subset(.tmp, select = c('dr', 'n', 'name', 'mean', 'ci.width'))

### convert factor columns (i.e., 'dr' and 'n') into numeric ones
.tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$dr)), as.numeric(as.character(.tmp$n))), select=c(6, 7, 3, 4, 5))
names(.tmp)[1:2]=c('dr', 'n')
.da_nh1_nf0_nv0.df <- sort_df(.tmp, vars=c('dr', 'n'))

## ## save data into text files for further processing (e.g., plotting by matplotlib)
## .avg_session_dlys <- split(.avg_session_dly, .avg_session_dly$dr) # list of data frames (per 'dr' basis)
## for (.i in 1:length(.avg_session_dlys)) {
##     .file_name <- paste("average_session_delay_dr", as.character(.avg_session_dlys[[.i]]$dr[1]), ".data", sep="")
##     write.table(.avg_session_dlys[[.i]], paste(.da_nh1_nf0_nv0.wd, .file_name, sep='/'), row.names=FALSE)
## }

.names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]')
.short_names = c('dly', 'thr', 'trf')
.da_nh1_nf0_nv0.plots <- list()
for (.i in 1:length(.names)) {
    .df <- subset(.da_nh1_nf0_nv0.df, name==.names[.i], select=c(1, 2, 4, 5))
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
    .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
    .da_nh1_nf0_nv0.plots[[.i]] <- .p

    ## save a plot as a PDF file
    .pdf <- paste("nh1_nf0_nv0-", .short_names[.i], ".pdf", sep="")
    pdf(paste(.da_nh1_nf0_nv0.wd, .pdf, sep="/"))
    .da_nh1_nf0_nv0.plots[[.i]]
    dev.off()
}

#################################################################################
### summary plots for dedicated architecture with HTTP and video traffic but no
### traffic shaping
#################################################################################
.df <- loadDataset(paste(.da_nh1_nf0_nv1.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue', subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, mean session transfer rate, and decodable frame rate
.tmp <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
             subset=(name=='average session delay [s]' |
                     name=='average session throughput [B/s]' |
                     name=='mean session transfer rate [B/s]' |
                     name=='decodable frame rate (Q)'))
.tmp <- subset(.tmp, select = c('dr', 'n', 'name', 'mean', 'ci.width'))

### convert factor columns (i.e., 'dr' and 'n') into numeric ones
.tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$dr)), as.numeric(as.character(.tmp$n))), select=c(6, 7, 3, 4, 5))
names(.tmp)[1:2]=c('dr', 'n')
.da_nh1_nf0_nv1.df <- sort_df(.tmp, vars=c('dr', 'n'))

## ## save data into text files for further processing (e.g., plotting by matplotlib)
## .avg_session_dlys <- split(.avg_session_dly, .avg_session_dly$dr) # list of data frames (per 'dr' basis)
## for (.i in 1:length(.avg_session_dlys)) {
##     .file_name <- paste("average_session_delay_dr", as.character(.avg_session_dlys[[.i]]$dr[1]), ".data", sep="")
##     write.table(.avg_session_dlys[[.i]], paste(.da_nh1_nf0_nv0.wd, .file_name, sep='/'), row.names=FALSE)
## }

.names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
.short_names = c('dly', 'thr', 'trf', 'dfr')
.da_nh1_nf0_nv1.plots <- list()
for (.i in 1:length(.names)) {
    .df <- subset(.da_nh1_nf0_nv1.df, name==.names[.i], select=c(1, 2, 4, 5))
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
    .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
    .da_nh1_nf0_nv1.plots[[.i]] <- .p

    ## save a plot as a PDF file
    .pdf <- paste("nh1_nf0_nv1-", .short_names[.i], ".pdf", sep="")
    pdf(paste(.da_nh1_nf0_nv1.wd, .pdf, sep="/"))
    .da_nh1_nf0_nv1.plots[[.i]]
    dev.off()
}

## #################################################################################
## ### summary plots for dedicated architecture with HTTP and video traffic but no
## ### traffic shaping
## #################################################################################
## .df <- loadDataset(paste(.da_dr100M_nh1_nf0_nv1.wd, "*.sca", sep='/'))
## .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue', subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
##                   .df$scalars, by='runid',
##                   all.x=TRUE)

## ## collect average session delay, average session throughput, mean session transfer rate, and decodable frame rate
## .tmp <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
##              subset=(name=='average session delay [s]' |
##                      name=='average session throughput [B/s]' |
##                      name=='mean session transfer rate [B/s]' |
##                      name=='decodable frame rate (Q)'))
## .tmp <- subset(.tmp, select = c('dr', 'n', 'name', 'mean', 'ci.width'))

## ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
## .tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$dr)), as.numeric(as.character(.tmp$n))), select=c(6, 7, 3, 4, 5))
## names(.tmp)[1:2]=c('dr', 'n')
## .da_dr100M_nh1_nf0_nv1.df <- sort_df(.tmp, vars=c('dr', 'n'))

## ## ## save data into text files for further processing (e.g., plotting by matplotlib)
## ## .avg_session_dlys <- split(.avg_session_dly, .avg_session_dly$dr) # list of data frames (per 'dr' basis)
## ## for (.i in 1:length(.avg_session_dlys)) {
## ##     .file_name <- paste("average_session_delay_dr", as.character(.avg_session_dlys[[.i]]$dr[1]), ".data", sep="")
## ##     write.table(.avg_session_dlys[[.i]], paste(.da_nh1_nf0_nv0.wd, .file_name, sep='/'), row.names=FALSE)
## ## }

## .names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
## .short_names = c('dly', 'thr', 'trf', 'dfr')
## .da_dr100M_nh1_nf0_nv1.plots <- list()
## for (.i in 1:length(.names)) {
##     .df <- subset(.da_dr100M_nh1_nf0_nv1.df, name==.names[.i], select=c(1, 2, 4, 5))
##     .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
##     .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
##     .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
##     .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
##     .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
##     .da_dr100M_nh1_nf0_nv1.plots[[.i]] <- .p

##     ## save a plot as a PDF file
##     .pdf <- paste("dr100M_nh1_nf0_nv1-", .short_names[.i], ".pdf", sep="")
##     pdf(paste(.da_dr100M_nh1_nf0_nv1.wd, .pdf, sep="/"))
##     .da_dr100M_nh1_nf0_nv1.plots[[.i]]
##     dev.off()
## }

#################################################################################
### summary plots for dedicated architecture with HTTP traffic and traffic
### shaping
###
### Notes: Due to the huge number of files to process (i.e., 4800 in total),
###        we split them into three groups accorindg to the value of 'dr' and
###        process them separately.
#################################################################################
.dr.range <- unique(.da_nh1_nf0_nv0.df$dr)
.da_nh1_nf0_nv0_tbf.df <- list()
.da_nh1_nf0_nv0_tbf.plots <- list()
for (.i in 1:length(.dr.range)) {
    .file_list <- read.table(paste(.da_nh1_nf0_nv0_tbf.wd, paste("file_list_dr", as.character(.dr.range[.i]), ".txt", sep=""), sep="/"))
    .df <- loadDataset(as.vector(.file_list$V1))
    .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                           subset=attrname %in% c('experiment', 'measurement', 'mr', 'bs', 'n')),
                      .df$scalars, by='runid',
                      all.x=TRUE)

    ## collect average session delay, average session throughput, and mean session transfer rate
    .tmp <- cast(.scalars, experiment+measurement+bs+mr+n+name ~ ., c(mean, ci.width),
                 subset=grepl('.*\\.httpApp.*', module) &
                 (name=='average session delay [s]' |
                  name=='average session throughput [B/s]' |
                  name=='mean session transfer rate [B/s]'))
    .tmp <- subset(.tmp, select = c('mr', 'bs', 'n', 'name', 'mean', 'ci.width'))

    ## convert factor columns (i.e., 'mr', 'bs', and 'n') into numeric ones
    .tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$mr)), as.numeric(as.character(.tmp$bs)), as.numeric(as.character(.tmp$n))), select=c(7, 8, 9, 4, 5, 6))
    names(.tmp)[1:3]=c('mr', 'bs', 'n')
    .da_nh1_nf0_nv0_tbf.df[[.i]] <- sort_df(.tmp, vars=c('mr', 'bs', 'n'))

    .mr.range <- unique(.da_nh1_nf0_nv0_tbf.df[[.i]]$mr)
    .da_nh1_nf0_nv0_tbf.plots[[.i]] <- list()
    for (.j in 1:length(.mr.range)) {
        for (.k in 1:length(.names)) {
            .df <- subset(.da_nh1_nf0_nv0_tbf.df[[.i]], mr==.mr.range[.j] & name==.names[.k], select=c(2, 3, 5, 6))
            .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
            .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
            .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
            .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
            .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
            .da_nh1_nf0_nv0_tbf.plots[[.i]][[(.j-1)*length(.names)+.k]] <- .p
        }
    }
}

#################################################################################
### summary plots for dedicated architecture with HTTP and video traffic and
### traffic shaping
#################################################################################
.df <- loadDataset(paste(.da_dr100M_nh1_nf0_nv1_tbf.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                       subset=attrname %in% c('experiment', 'measurement', 'mr', 'bs', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, mean session transfer rate, and decodable frame rate
.tmp <- cast(.scalars, experiment+measurement+bs+mr+n+name ~ ., c(mean, ci.width),
             subset=(name=='average session delay [s]' |
                     name=='average session throughput [B/s]' |
                     name=='mean session transfer rate [B/s]' |
                     name=='decodable frame rate (Q)'))
.tmp <- subset(.tmp, select = c('mr', 'bs', 'n', 'name', 'mean', 'ci.width'))

### convert factor columns (i.e., 'mr', 'bs', and 'n') into numeric ones
.tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$mr)), as.numeric(as.character(.tmp$bs)), as.numeric(as.character(.tmp$n))), select=c(7, 8, 9, 4, 5, 6))
names(.tmp)[1:3]=c('mr', 'bs', 'n')
.da_dr100M_nh1_nf0_nv1_tbf.df <- sort_df(.tmp, vars=c('mr', 'bs', 'n'))

.names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
.short_names = c('dly', 'thr', 'trf', 'dfr')
.mr.range <- unique(.da_dr100M_nh1_nf0_nv1_tbf.df$mr)
.da_dr100M_nh1_nf0_nv1_tbf.plots <- list()
for (.i in 1:length(.mr.range)) {
    for (.j in 1:length(.names)) {
        .df <- subset(.da_dr100M_nh1_nf0_nv1_tbf.df, mr==.mr.range[.i] & name==.names[.j], select=c(2, 3, 5, 6))
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.j])
        .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
        .da_dr100M_nh1_nf0_nv1_tbf.plots[[(.i-1)*length(.names)+.j]] <- .p

        ## save a plot as a PDF file
        .pdf <- paste("dr100M_nh1_nf0_nv1_tbf_mr", as.character(.mr.range[.i]), "-", .short_names[.j], ".pdf", sep="")
        pdf(paste(.da_dr100M_nh1_nf0_nv1_tbf.wd, .pdf, sep="/"))
        .da_dr100M_nh1_nf0_nv1_tbf.plots[[(.i-1)*length(.names)+.j]]
        dev.off()
    }
}
