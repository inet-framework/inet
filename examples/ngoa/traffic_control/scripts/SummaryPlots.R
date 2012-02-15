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

### define variables
## dedicated access for n_h=1, n_f=5, n_v=1
.da_nh1_nf5_nv1.wd <- paste(.base.directory, "results/Dedicated/nh1_nf5_nv1", sep="/")
.da_nh1_nf5_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf5_nv1_tbf", sep="/")
.da_dr100M_nh1_nf5_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/dr100M_nh1_nf5_nv1_tbf", sep="/")
## dedicated access for n_h=1, n_f=0, n_v=1
.da_nh1_nf0_nv1.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv1", sep="/")
#.da_nh1_nf0_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv1_tbf", sep="/")
.da_dr100M_nh1_nf0_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/dr100M_nh1_nf0_nv1_tbf", sep="/")
.da_dr1G_nh1_nf0_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/dr1G_nh1_nf0_nv1_tbf", sep="/")
## dedicated access for n_h=1, n_f=0, n_v=0
.da_nh1_nf0_nv0.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv0", sep="/")
.da_nh1_nf0_nv0_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf0_nv0_tbf", sep="/")
## shared access for n_h=1, n_f=5, n_v=1
.sa_nh1_nf5_nv1_tbf.wd <- paste(.base.directory, "results/Shared/nh1_nf5_nv1_tbf", sep="/")
## for plotting
.traffic <- c("ftp", "ftp", "ftp", "http", "http", "http", "video")
.measure <-  c('average session delay [s]',
               'average session throughput [B/s]',
               'mean session transfer rate [B/s]',
               'average session delay [s]',
               'average session throughput [B/s]',
               'mean session transfer rate [B/s]',
               'decodable frame rate (Q)')
.measure_abbrv <- c('dly', 'thr', 'trf', 'dly', 'thr', 'trf', 'dfr')
.labels.traffic <- c("FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
.labels.measure <- c("Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Decodable Frame Rate (Q)")

### define functions
ci.width <- conf.int(0.95)                  # for 95% confidence interval

### customize
.old <- theme_set(theme_bw())
.pt_size <- 3.5


#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### but no traffic shaping
#################################################################################
.df <- loadDataset(paste(.da_nh1_nf5_nv1.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                       subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
.tmp_ftp <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
                 subset=grepl('.*\\.ftpApp.*', module) &
                 (name=='average session delay [s]' |
                  name=='average session throughput [B/s]' |
                  name=='mean session transfer rate [B/s]'))
.tmp_ftp <- subset(.tmp_ftp, select = c('dr', 'n', 'name', 'mean', 'ci.width'))
.tmp_ftp <- subset(cbind(.tmp_ftp,      ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                         as.numeric(as.character(.tmp_ftp$dr)),
                         as.numeric(as.character(.tmp_ftp$n))),
                   select=c(6, 7, 3, 4, 5))
names(.tmp_ftp)[1:2]=c('dr', 'n')
.tmp_ftp <- sort_df(.tmp_ftp, vars=c('dr', 'n'))
.tmp_ftp$traffic <- rep('ftp', length(.tmp_ftp$mean))

## collect average session delay, average session throughput, and mean session transfer rate of HTTP traffic
.tmp_http <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
                  subset=grepl('.*\\.httpApp.*', module) &
                  (name=='average session delay [s]' |
                   name=='average session throughput [B/s]' |
                   name=='mean session transfer rate [B/s]'))
.tmp_http <- subset(.tmp_http, select = c('dr', 'n', 'name', 'mean', 'ci.width'))
.tmp_http <- subset(cbind(.tmp_http,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                          as.numeric(as.character(.tmp_http$dr)),
                          as.numeric(as.character(.tmp_http$n))),
                    select=c(6, 7, 3, 4, 5))
names(.tmp_http)[1:2]=c('dr', 'n')
.tmp_http <- sort_df(.tmp_http, vars=c('dr', 'n'))
.tmp_http$traffic <- rep('http', length(.tmp_http$mean))

## collect decodable frame rate of video traffic
.tmp_video <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
                   subset=grepl('.*\\.videoApp.*', module) &
                   name=='decodable frame rate (Q)')
.tmp_video <- subset(.tmp_video, select = c('dr', 'n', 'name', 'mean', 'ci.width'))
.tmp_video <- subset(cbind(.tmp_video,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                           as.numeric(as.character(.tmp_video$dr)),
                           as.numeric(as.character(.tmp_video$n))),
                     select=c(6, 7, 3, 4, 5))
names(.tmp_video)[1:2]=c('dr', 'n')
.tmp_video <- sort_df(.tmp_video, vars=c('dr', 'n'))
.tmp_video$traffic <- rep('video', length(.tmp_video$mean))

## combine the tree data frames into one and sort it
.tmp <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
.da_nh1_nf5_nv1.df <- sort_df(.tmp, vars=c('dr', 'n'))

.da_nh1_nf5_nv1.plots <- list()
for (.i in 1:length(.traffic)) {
    .df <- subset(.da_nh1_nf5_nv1.df, name==.measure[.i] & traffic==.traffic[.i], select=c(1, 2, 4, 5))
    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
    .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
    .da_nh1_nf5_nv1.plots[[.i]] <- .p

    ## save as a PDF file
    .p
    ggsave(paste(.da_nh1_nf5_nv1.wd,
                 paste("nh1_nf5_nv1-", .traffic[.i], "-", .measure_abbrv[.i], ".pdf", sep=""),
                 sep="/"))
}

## save data frame and plots for later use
save(.da_nh1_nf5_nv1.df, .da_nh1_nf5_nv1.plots,
     file=paste(.da_nh1_nf5_nv1.wd, "nh1_nf5_nv1.RData", sep="/"))


#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.df <- loadDataset(paste(.da_nh1_nf5_nv1_tbf.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                       subset=attrname %in% c('experiment', 'measurement', 'dr', 'mr', 'bs', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
.tmp_ftp <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                 subset=grepl('.*\\.ftpApp.*', module) &
                 (name=='average session delay [s]' |
                  name=='average session throughput [B/s]' |
                  name=='mean session transfer rate [B/s]'))
.tmp_ftp <- subset(.tmp_ftp, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_ftp <- subset(cbind(.tmp_ftp,      ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                         as.numeric(as.character(.tmp_ftp$dr)),
                         as.numeric(as.character(.tmp_ftp$mr)),
                         as.numeric(as.character(.tmp_ftp$bs)),
                         as.numeric(as.character(.tmp_ftp$n))),
                   select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_ftp)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_ftp <- sort_df(.tmp_ftp, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_ftp$traffic <- rep('ftp', length(.tmp_ftp$mean))

## collect average session delay, average session throughput, and mean session transfer rate of HTTP traffic
.tmp_http <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                  subset=grepl('.*\\.httpApp.*', module) &
                  (name=='average session delay [s]' |
                   name=='average session throughput [B/s]' |
                   name=='mean session transfer rate [B/s]'))
.tmp_http <- subset(.tmp_http, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_http <- subset(cbind(.tmp_http,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                          as.numeric(as.character(.tmp_http$dr)),
                          as.numeric(as.character(.tmp_http$mr)),
                          as.numeric(as.character(.tmp_http$bs)),
                          as.numeric(as.character(.tmp_http$n))),
                    select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_http)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_http <- sort_df(.tmp_http, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_http$traffic <- rep('http', length(.tmp_http$mean))

## collect decodable frame rate of video traffic
.tmp_video <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                   subset=grepl('.*\\.videoApp.*', module) &
                   name=='decodable frame rate (Q)')
.tmp_video <- subset(.tmp_video, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_video <- subset(cbind(.tmp_video,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                           as.numeric(as.character(.tmp_video$dr)),
                           as.numeric(as.character(.tmp_video$mr)),
                           as.numeric(as.character(.tmp_video$bs)),
                           as.numeric(as.character(.tmp_video$n))),
                     select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_video)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_video <- sort_df(.tmp_video, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_video$traffic <- rep('video', length(.tmp_video$mean))

## combine the tree data frames into one and sort it
.tmp <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
.da_nh1_nf5_nv1_tbf.df <- sort_df(.tmp, vars=c('dr', 'mr', 'bs', 'n'))

.dr.range <- unique(.da_nh1_nf5_nv1_tbf.df$dr)
.mr.range <- unique(.da_nh1_nf5_nv1_tbf.df$mr)
.da_nh1_nf5_nv1_tbf.plots <- list()
for (.i in 1:length(.dr.range)) {
    .da_nh1_nf5_nv1_tbf.plots[[.i]] <- list()
    .mr.idx <- 0
    for (.j in 1:length(.mr.range)) {
        if (length(subset(.da_nh1_nf5_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j])$mean) > 0) {
            .mr.idx <- .mr.idx + 1
            .da_nh1_nf5_nv1_tbf.plots[[.i]][[.mr.idx]] <- list()
            for (.k in 1:length(.traffic)) {
                .df <- subset(.da_nh1_nf5_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & traffic==.traffic[.k], select=c(3, 4, 6, 7))
                .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                .da_nh1_nf5_nv1_tbf.plots[[.i]][[.mr.idx]][[.k]] <- .p

                ## save as a PDF file
                .p
                ggsave(paste(.da_nh1_nf5_nv1_tbf.wd,
                             paste("nh1_nf5_nv1_tbf-dr", as.character(.dr.range[.i]),
                                   "_mr", as.character(.mr.range[.j]),
                                   "-", .traffic[.k],
                                   "-", .measure_abbrv[.k], ".pdf", sep=""), sep="/"))
            }   # end of for(.k)
        }   # end of if()
    }   # end of for(.j)
}   # end of for(.i)

## save data frame and plots for later use
save(.da_nh1_nf5_nv1_tbf.df, .da_nh1_nf5_nv1_tbf.plots,
     file=paste(.da_nh1_nf5_nv1_tbf.wd, "nh1_nf5_nv1_tbf.RData", sep="/"))


#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.df <- loadDataset(paste(.sa_nh1_nf5_nv1_tbf.wd, "*.sca", sep='/'))
.scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                       subset=attrname %in% c('experiment', 'measurement', 'dr', 'mr', 'bs', 'n')),
                  .df$scalars, by='runid',
                  all.x=TRUE)

## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
.tmp_ftp <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                 subset=grepl('.*\\.ftpApp.*', module) &
                 (name=='average session delay [s]' |
                  name=='average session throughput [B/s]' |
                  name=='mean session transfer rate [B/s]'))
.tmp_ftp <- subset(.tmp_ftp, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_ftp <- subset(cbind(.tmp_ftp,      ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                         as.numeric(as.character(.tmp_ftp$dr)),
                         as.numeric(as.character(.tmp_ftp$mr)),
                         as.numeric(as.character(.tmp_ftp$bs)),
                         as.numeric(as.character(.tmp_ftp$n))),
                   select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_ftp)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_ftp <- sort_df(.tmp_ftp, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_ftp$traffic <- rep('ftp', length(.tmp_ftp$mean))

## collect average session delay, average session throughput, and mean session transfer rate of HTTP traffic
.tmp_http <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                  subset=grepl('.*\\.httpApp.*', module) &
                  (name=='average session delay [s]' |
                   name=='average session throughput [B/s]' |
                   name=='mean session transfer rate [B/s]'))
.tmp_http <- subset(.tmp_http, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_http <- subset(cbind(.tmp_http,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                          as.numeric(as.character(.tmp_http$dr)),
                          as.numeric(as.character(.tmp_http$mr)),
                          as.numeric(as.character(.tmp_http$bs)),
                          as.numeric(as.character(.tmp_http$n))),
                    select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_http)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_http <- sort_df(.tmp_http, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_http$traffic <- rep('http', length(.tmp_http$mean))

## collect decodable frame rate of video traffic
.tmp_video <- cast(.scalars, experiment+measurement+dr+mr+bs+n+name ~ ., c(mean, ci.width),
                   subset=grepl('.*\\.videoApp.*', module) &
                   name=='decodable frame rate (Q)')
.tmp_video <- subset(.tmp_video, select = c('dr', 'mr', 'bs', 'n', 'name', 'mean', 'ci.width'))
.tmp_video <- subset(cbind(.tmp_video,    ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
                           as.numeric(as.character(.tmp_video$dr)),
                           as.numeric(as.character(.tmp_video$mr)),
                           as.numeric(as.character(.tmp_video$bs)),
                           as.numeric(as.character(.tmp_video$n))),
                     select=c(8, 9, 10, 11, 5, 6, 7))
names(.tmp_video)[1:4]=c('dr', 'mr', 'bs', 'n')
.tmp_video <- sort_df(.tmp_video, vars=c('dr', 'mr', 'bs', 'n'))
.tmp_video$traffic <- rep('video', length(.tmp_video$mean))

## combine the tree data frames into one and sort it
.tmp <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
.da_nh1_nf5_nv1_tbf.df <- sort_df(.tmp, vars=c('dr', 'mr', 'bs', 'n'))

.dr.range <- unique(.da_nh1_nf5_nv1_tbf.df$dr)
.mr.range <- unique(.da_nh1_nf5_nv1_tbf.df$mr)
.da_nh1_nf5_nv1_tbf.plots <- list()
for (.i in 1:length(.dr.range)) {
    .da_nh1_nf5_nv1_tbf.plots[[.i]] <- list()
    .mr.idx <- 0
    for (.j in 1:length(.mr.range)) {
        if (length(subset(.da_nh1_nf5_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j])$mean) > 0) {
            .mr.idx <- .mr.idx + 1
            .da_nh1_nf5_nv1_tbf.plots[[.i]][[.mr.idx]] <- list()
            for (.k in 1:length(.traffic)) {
                .df <- subset(.da_nh1_nf5_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & traffic==.traffic[.k], select=c(3, 4, 6, 7))
                .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                .da_nh1_nf5_nv1_tbf.plots[[.i]][[.mr.idx]][[.k]] <- .p

                ## save as a PDF file
                .p
                ggsave(paste(.da_nh1_nf5_nv1_tbf.wd,
                             paste("nh1_nf5_nv1_tbf-dr", as.character(.dr.range[.i]),
                                   "_mr", as.character(.mr.range[.j]),
                                   "-", .traffic[.k],
                                   "-", .measure_abbrv[.k], ".pdf", sep=""), sep="/"))
            }   # end of for(.k)
        }   # end of if()
    }   # end of for(.j)
}   # end of for(.i)

## save data frame and plots for later use
save(.da_nh1_nf5_nv1_tbf.df, .da_nh1_nf5_nv1_tbf.plots,
     file=paste(.da_nh1_nf5_nv1_tbf.wd, "nh1_nf5_nv1_tbf.RData", sep="/"))


## #################################################################################
## ### summary plots for dedicated architecture with HTTP and video traffic but no
## ### traffic shaping
## #################################################################################
## .df <- loadDataset(paste(.da_nh1_nf0_nv1.wd, "*.sca", sep='/'))
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
## .da_nh1_nf0_nv1.df <- sort_df(.tmp, vars=c('dr', 'n'))

## ## ## save data into text files for further processing (e.g., plotting by matplotlib)
## ## .avg_session_dlys <- split(.avg_session_dly, .avg_session_dly$dr) # list of data frames (per 'dr' basis)
## ## for (.i in 1:length(.avg_session_dlys)) {
## ##     .file_name <- paste("average_session_delay_dr", as.character(.avg_session_dlys[[.i]]$dr[1]), ".data", sep="")
## ##     write.table(.avg_session_dlys[[.i]], paste(.da_nh1_nf0_nv0.wd, .file_name, sep='/'), row.names=FALSE)
## ## }

## .names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
## .short_names = c('dly', 'thr', 'trf', 'dfr')
## .da_nh1_nf0_nv1.plots <- list()
## for (.i in 1:length(.names)) {
##     .df <- subset(.da_nh1_nf0_nv1.df, name==.names[.i], select=c(1, 2, 4, 5))
##     .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
##     .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
##     .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
##     .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
##     .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
##     .da_nh1_nf0_nv1.plots[[.i]] <- .p

##     ## save a plot as a PDF file
##     .pdf <- paste("nh1_nf0_nv1-", .short_names[.i], ".pdf", sep="")
##     pdf(paste(.da_nh1_nf0_nv1.wd, .pdf, sep="/"))
##     .da_nh1_nf0_nv1.plots[[.i]]
##     dev.off()
## }

## #################################################################################
## ### summary plots for dedicated architecture with HTTP traffic and no traffic
## ### shaping
## #################################################################################
## .df <- loadDataset(paste(.da_nh1_nf0_nv0.wd, "*.sca", sep='/'))
## .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue', subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
##                   .df$scalars, by='runid',
##                   all.x=TRUE)

## ## collect average session delay, average session throughput, and mean session transfer rate
## .tmp <- cast(.scalars, experiment+measurement+dr+n+name ~ ., c(mean, ci.width),
##              subset=grepl('.*\\.httpApp.*', module) &
##              (name=='average session delay [s]' |
##               name=='average session throughput [B/s]' |
##               name=='mean session transfer rate [B/s]'))
## .tmp <- subset(.tmp, select = c('dr', 'n', 'name', 'mean', 'ci.width'))

## ### convert factor columns (i.e., 'dr' and 'n') into numeric ones
## .tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$dr)), as.numeric(as.character(.tmp$n))), select=c(6, 7, 3, 4, 5))
## names(.tmp)[1:2]=c('dr', 'n')
## .da_nh1_nf0_nv0.df <- sort_df(.tmp, vars=c('dr', 'n'))

## ## ## save data into text files for further processing (e.g., plotting by matplotlib)
## ## .avg_session_dlys <- split(.avg_session_dly, .avg_session_dly$dr) # list of data frames (per 'dr' basis)
## ## for (.i in 1:length(.avg_session_dlys)) {
## ##     .file_name <- paste("average_session_delay_dr", as.character(.avg_session_dlys[[.i]]$dr[1]), ".data", sep="")
## ##     write.table(.avg_session_dlys[[.i]], paste(.da_nh1_nf0_nv0.wd, .file_name, sep='/'), row.names=FALSE)
## ## }

## .names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]')
## .short_names = c('dly', 'thr', 'trf')
## .da_nh1_nf0_nv0.plots <- list()
## for (.i in 1:length(.names)) {
##     .df <- subset(.da_nh1_nf0_nv0.df, name==.names[.i], select=c(1, 2, 4, 5))
##     .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
##     .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
##     .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
##     .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
##     .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
##     .da_nh1_nf0_nv0.plots[[.i]] <- .p

##     ## save a plot as a PDF file
##     .pdf <- paste("nh1_nf0_nv0-", .short_names[.i], ".pdf", sep="")
##     pdf(paste(.da_nh1_nf0_nv0.wd, .pdf, sep="/"))
##     .da_nh1_nf0_nv0.plots[[.i]]
##     dev.off()
## }

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


## #################################################################################
## ### summary plots for dedicated architecture with HTTP traffic and traffic
## ### shaping
## ###
## ### Notes: Due to the huge number of files to process (i.e., 4800 in total),
## ###        we split them into three groups accorindg to the value of 'dr' and
## ###        process them separately.
## #################################################################################
## .dr.range <- unique(.da_nh1_nf0_nv0.df$dr)
## .da_nh1_nf0_nv0_tbf.df <- list()
## .da_nh1_nf0_nv0_tbf.plots <- list()
## for (.i in 1:length(.dr.range)) {
##     .file_list <- read.table(paste(.da_nh1_nf0_nv0_tbf.wd, paste("file_list_dr", as.character(.dr.range[.i]), ".txt", sep=""), sep="/"))
##     .df <- loadDataset(as.vector(.file_list$V1))
##     .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
##                            subset=attrname %in% c('experiment', 'measurement', 'mr', 'bs', 'n')),
##                       .df$scalars, by='runid',
##                       all.x=TRUE)

##     ## collect average session delay, average session throughput, and mean session transfer rate
##     .tmp <- cast(.scalars, experiment+measurement+bs+mr+n+name ~ ., c(mean, ci.width),
##                  subset=grepl('.*\\.httpApp.*', module) &
##                  (name=='average session delay [s]' |
##                   name=='average session throughput [B/s]' |
##                   name=='mean session transfer rate [B/s]'))
##     .tmp <- subset(.tmp, select = c('mr', 'bs', 'n', 'name', 'mean', 'ci.width'))

##     ## convert factor columns (i.e., 'mr', 'bs', and 'n') into numeric ones
##     .tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$mr)), as.numeric(as.character(.tmp$bs)), as.numeric(as.character(.tmp$n))), select=c(7, 8, 9, 4, 5, 6))
##     names(.tmp)[1:3]=c('mr', 'bs', 'n')
##     .da_nh1_nf0_nv0_tbf.df[[.i]] <- sort_df(.tmp, vars=c('mr', 'bs', 'n'))

##     .mr.range <- unique(.da_nh1_nf0_nv0_tbf.df[[.i]]$mr)
##     .da_nh1_nf0_nv0_tbf.plots[[.i]] <- list()
##     for (.j in 1:length(.mr.range)) {
##         for (.k in 1:length(.names)) {
##             .df <- subset(.da_nh1_nf0_nv0_tbf.df[[.i]], mr==.mr.range[.j] & name==.names[.k], select=c(2, 3, 5, 6))
##             .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
##             .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
##             .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
##             .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
##             .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
##             .da_nh1_nf0_nv0_tbf.plots[[.i]][[(.j-1)*length(.names)+.k]] <- .p
##         }
##     }
## }

## #################################################################################
## ### summary plots for dedicated architecture with HTTP and video traffic and
## ### traffic shaping
## #################################################################################
## .df <- loadDataset(paste(.da_dr100M_nh1_nf0_nv1_tbf.wd, "*.sca", sep='/'))
## .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
##                        subset=attrname %in% c('experiment', 'measurement', 'mr', 'bs', 'n')),
##                   .df$scalars, by='runid',
##                   all.x=TRUE)

## ## collect average session delay, average session throughput, mean session transfer rate, and decodable frame rate
## .tmp <- cast(.scalars, experiment+measurement+bs+mr+n+name ~ ., c(mean, ci.width),
##              subset=(name=='average session delay [s]' |
##                      name=='average session throughput [B/s]' |
##                      name=='mean session transfer rate [B/s]' |
##                      name=='decodable frame rate (Q)'))
## .tmp <- subset(.tmp, select = c('mr', 'bs', 'n', 'name', 'mean', 'ci.width'))

## ### convert factor columns (i.e., 'mr', 'bs', and 'n') into numeric ones
## .tmp <- subset(cbind(.tmp, as.numeric(as.character(.tmp$mr)), as.numeric(as.character(.tmp$bs)), as.numeric(as.character(.tmp$n))), select=c(7, 8, 9, 4, 5, 6))
## names(.tmp)[1:3]=c('mr', 'bs', 'n')
## .da_dr100M_nh1_nf0_nv1_tbf.df <- sort_df(.tmp, vars=c('mr', 'bs', 'n'))

## .names = c('average session delay [s]', 'average session throughput [B/s]', 'mean session transfer rate [B/s]', 'decodable frame rate (Q)')
## .short_names = c('dly', 'thr', 'trf', 'dfr')
## .mr.range <- unique(.da_dr100M_nh1_nf0_nv1_tbf.df$mr)
## .da_dr100M_nh1_nf0_nv1_tbf.plots <- list()
## for (.i in 1:length(.mr.range)) {
##     for (.j in 1:length(.names)) {
##         .df <- subset(.da_dr100M_nh1_nf0_nv1_tbf.df, mr==.mr.range[.i] & name==.names[.j], select=c(2, 3, 5, 6))
##         .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
##         .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
##         .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.j])
##         .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
##         .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
##         .da_dr100M_nh1_nf0_nv1_tbf.plots[[(.i-1)*length(.names)+.j]] <- .p

##         ## save a plot as a PDF file
##         .pdf <- paste("dr100M_nh1_nf0_nv1_tbf_mr", as.character(.mr.range[.i]), "-", .short_names[.j], ".pdf", sep="")
##         pdf(paste(.da_dr100M_nh1_nf0_nv1_tbf.wd, .pdf, sep="/"))
##         .da_dr100M_nh1_nf0_nv1_tbf.plots[[(.i-1)*length(.names)+.j]]
##         dev.off()
##     }
## }
