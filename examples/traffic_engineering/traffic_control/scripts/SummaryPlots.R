#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for generating summary plots for all performance measures
### from OMNeT++ scala files for the simulation of NGN access networks
### with traffci control (i.e., shaping/policing)
###
### (C) 2012 Kyeong Soo (Joseph) Kim
###


###
### load add-on packages and external source files
###
library(ggplot2)
library(gridExtra)
library(omnetpp)
#library(plyr)
library(reshape)
#library(xtable)
ifelse (Sys.getenv("OS") == "Windows_NT",
        .base.directory <- "E:/tools/omnetpp/inet-hnrl/examples/ngoa/traffic_control",
        .base.directory <- "~/inet-hnrl/examples/ngoa/traffic_control")
source(paste(.base.directory, 'scripts/collectMeasures.R', sep='/'))
###
### define variables
###
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
.sa_N10_nh1_nf5_nv1_tbf.wd <- paste(.base.directory, "results/Shared/test/N10_nh1_nf5_nv1_tbf-test", sep="/")
## for plotting
.measure <-  c('average session delay [s]',
               'average session throughput [B/s]',
               'mean session transfer rate [B/s]',
               'average session delay [s]',
               '90th-sessionDelay:percentile',
               '95th-sessionDelay:percentile',
               '99th-sessionDelay:percentile',
               'average session throughput [B/s]',
               'mean session transfer rate [B/s]',
               'decodable frame rate (Q)',
               'average packet delay [s]',
               '90th-packetDelay:percentile',
               '95th-packetDelay:percentile',
               '99th-packetDelay:percentile',
               'overall packet loss rate of per-VLAN queues',
               'overall packet shaped rate of per-VLAN queues')
.measure.type<- c("ftp", "ftp", "ftp", "http", "http", "http", "http", "http", "http", "video", "packet", "packet", "packet", "packet", "queue", "queue")
.measure.abbrv <- c('dly', 'thr', 'trf', 'dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf', 'dfr', 'dly', '90p-dly', '95p-dly', '99p-dly', 'plr', 'psr')
.labels.traffic <- c("FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
.labels.measure <- c("Average Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Session Delay [sec]",
                     "90-Percentile Session Delay [sec]",
                     "95-Percentile Session Delay [sec]",
                     "99-Percentile Session Delay [sec]",
                     "Average Session Throughput [Byte/sec]",
                     "Mean Session Transfer Rate [Byte/sec]",
                     "Average Decodable Frame Rate (Q)",
                     "Average DS Packet Delay [sec]",
                     "90-Percentile DS Packet Delay [sec]",
                     "95-Percentile DS Packet Delay [sec]",
                     "99-Percentile DS Packet Delay [sec]",
                     "DS Packet Loss Rate",
                     "DS Packet Shaped Rate"
                     )
###
### define functions
###
## for 95% confidence interval
ci.width <- conf.int(0.95)
## for pause before and after processing
Pause <- function () {
    cat("Hit <enter> to continue...")
    readline()
    invisible()
}
###
### customize
###
.old <- theme_set(theme_bw())
.pt_size <- 3.5


#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### but no traffic shaping
#################################################################################
.resp <- readline("Process data from dedicated access without traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.da_nh1_nf5_nv1.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
    .n_repetitions <- 10    # number of repetitions per experiment
    .n_experiments <- .n_totalFiles / .n_repetitions  # number of experiments
    .n_files <- .n_totalFiles / .n_experiments  # number of files per experiment
    .dfs <- list()  # list of data frames from experiments
    .fileNames <- rep('', .n_files)  # vector of file names
    for (.i in 1:.n_experiments) {
        cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
        for (.j in 1:.n_files) {
            .fileNames[.j] <- paste(.da_nh1_nf5_nv1.wd, paste("nh1_nf5_nv1-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""), sep='/')
        }
        .df <- loadDataset(.fileNames)
        .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                               subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
                          .df$scalars, by='runid',
                          all.x=TRUE)
        ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
        .tmp_ftp <- collectMeasures(.scalars,
                                    "experiment+measurement+dr+n+name ~ .",
                                    '.*\\.ftpApp.*',
                                    '(average session delay|average session throughput|mean session transfer rate)',
                                    c('dr', 'n'),
                                    'ftp')
        ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
        .tmp_http <- collectMeasures(.scalars,
                                     "experiment+measurement+dr+n+name ~ .",
                                     '.*\\.httpApp.*',
                                     '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                     c('dr', 'n'),
                                     'http')
        ## collect decodable frame rate of video traffic
        .tmp_video <- collectMeasures(.scalars,
                                      "experiment+measurement+dr+n+name ~ .",
                                      '.*\\.videoApp.*',
                                      'decodable frame rate',
                                      c('dr', 'n'),
                                      'video')
        ## combine the three data frames into one
        .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
    }
    ## combine the data frames from experiments into one
    .tmp.df <- .dfs[[1]]
    for (.i in 2:.n_experiments) {
        .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
    }
    ## sort the resulting data frame
    .da_nh1_nf5_nv1.df <- sort_df(.tmp.df, vars=c('dr', 'n'))
    .da_nh1_nf5_nv1.plots <- list()
    for (.i in 1:length(.measure.type)) {
        .df <- subset(.da_nh1_nf5_nv1.df, name==.measure[.i] & measure.type==.measure.type[.i], select=c(1, 2, 4, 5))
        is.na(.df) <- is.na(.df) # remove NaNs
        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
        .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
        .da_nh1_nf5_nv1.plots[[.i]] <- .p
        ## save each plot as a PDF file
        .p
        ggsave(paste(.da_nh1_nf5_nv1.wd,
                     paste("nh1_nf5_nv1-", .measure.type[.i], "-", .measure.abbrv[.i], ".pdf", sep=""),
                     sep="/"))
    }
    ## save data frame and plots for later use
    save(.da_nh1_nf5_nv1.df, .da_nh1_nf5_nv1.plots,
         file=paste(.da_nh1_nf5_nv1.wd, "nh1_nf5_nv1.RData", sep="/"))
}   # end of if()


#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.resp <- readline("Process data from dedicated access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    if (file.exists(paste(.da_nh1_nf5_nv1_tbf.wd, 'nh1_nf5_nv1_tbf.RData', sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is 'nh1_nf5_nv1_tbf.RData' in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.da_nh1_nf5_nv1_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- .n_totalFiles / .n_repetitions  # number of experiments
        .n_files <- .n_totalFiles / .n_experiments  # number of files per experiment
        .dfs <- list()  # list of data frames from experiments
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.da_nh1_nf5_nv1_tbf.wd, paste("nh1_nf5_nv1_tbf-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""), sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate)',
                                        c('dr', 'mr', 'bs', 'n'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+dr+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('dr', 'mr', 'bs', 'n'),
                                         'http')
            ## collect decodable frame rate of video traffic
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('dr', 'mr', 'bs', 'n'),
                                          'video')
            ## combine the three data frames into one
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
        }
        ## combine the data frames from experiments into one
        .tmp.df <- .dfs[[1]]
        for (.i in 2:.n_experiments) {
            .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
        }
        ## sort the resulting data frame
        .da_nh1_nf5_nv1_tbf.df <- sort_df(.tmp.df, vars=c('dr', 'mr', 'bs', 'n'))
        ## save data frame for later use
        save(.da_nh1_nf5_nv1_tbf.df, file=paste(.da_nh1_nf5_nv1_tbf.wd, "nh1_nf5_nv1_tbf.RData", sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.da_nh1_nf5_nv1_tbf.wd, 'nh1_nf5_nv1_tbf.RData', sep='/'))
    }   # end of if() for the processing of OMNeT++ data files

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
                for (.k in 1:length(.measure.type)) {
                    .df <- subset(.da_nh1_nf5_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(3, 4, 6, 7))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                    .da_nh1_nf5_nv1_tbf.plots[[.i]][[.mr.idx]][[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    ggsave(paste(.da_nh1_nf5_nv1_tbf.wd,
                                 paste("nh1_nf5_nv1_tbf-dr", as.character(.dr.range[.i]),
                                       "_mr", as.character(.mr.range[.j]),
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
                }   # end of for(.k)
            }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
    ## ## save data frame and plots for later use
    ## save(.da_nh1_nf5_nv1_tbf.df, .da_nh1_nf5_nv1_tbf.plots,
    ##      file=paste(.da_nh1_nf5_nv1_tbf.wd, "nh1_nf5_nv1_tbf.RData", sep="/"))
}   # end of if()


#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.resp <- readline("Process data from shared access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.sa_nh1_nf5_nv1_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
    .n_repetitions <- 10    # number of repetitions per experiment
    .n_experiments <- .n_totalFiles / .n_repetitions  # number of experiments
    .n_files <- .n_totalFiles / .n_experiments  # number of files per experiment
    .dfs <- list()  # list of data frames from experiments
    .fileNames <- rep('', .n_files)  # vector of file names
    for (.i in 1:.n_experiments) {
        cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
        for (.j in 1:.n_files) {
            .fileNames[.j] <- paste(.sa_nh1_nf5_nv1_tbf.wd, paste("nh1_nf5_nv1_tbf-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""), sep='/')
        }
        .df <- loadDataset(.fileNames)
        .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                               subset=attrname %in% c('experiment', 'measurement', 'N', 'dr', 'mr', 'bs', 'n')),
                          .df$scalars, by='runid',
                          all.x=TRUE)
        ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
        .tmp_ftp <- collectMeasures(.scalars,
                                    "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                    '.*\\.ftpApp.*',
                                    '(average session delay|average session throughput|mean session transfer rate)',
                                    c('N', 'dr', 'mr', 'bs', 'n'),
                                    'ftp')
        ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
        .tmp_http <- collectMeasures(.scalars,
                                     "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                     '.*\\.httpApp.*',
                                     '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                     c('N', 'dr', 'mr', 'bs', 'n'),
                                     'http')
        ## collect decodable frame rate of video traffic
        .tmp_video <- collectMeasures(.scalars,
                                     "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                     '.*\\.videoApp.*',
                                     'decodable frame rate',
                                     c('N', 'dr', 'mr', 'bs', 'n'),
                                     'video')
        ## combine the three data frames into one
        .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
    }
    ## combine the data frames from experiments into one
    .tmp.df <- .dfs[[1]]
    for (.i in 2:.n_experiments) {
        .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
    }
    ## sort the resulting data frame
    .sa_nh1_nf5_nv1_tbf.df <- sort_df(.tmp.df, vars=c('N', 'dr', 'mr', 'bs', 'n'))
    .N.range <- unique(.sa_nh1_nf5_nv1_tbf.df$N)
    .dr.range <- unique(.sa_nh1_nf5_nv1_tbf.df$dr)
    .mr.range <- unique(.sa_nh1_nf5_nv1_tbf.df$mr)
    .sa_nh1_nf5_nv1_tbf.plots <- list()
    for (.i in 1:length(.N.range)) {
        for (.j in 1:length(.dr.range)) {
            .sa_nh1_nf5_nv1_tbf.plots[[.j]] <- list()
            .mr.idx <- 0
            for (.k in 1:length(.mr.range)) {
                if (length(subset(.sa_nh1_nf5_nv1_tbf.df, N==.N.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k])$mean) > 0) {
                    .mr.idx <- .mr.idx + 1
                    .sa_nh1_nf5_nv1_tbf.plots[[.j]][[.mr.idx]] <- list()
                    for (.l in 1:length(.measure.type)) {
                        .df <- subset(.sa_nh1_nf5_nv1_tbf.df, N==.N.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k] & name==.measure[.l] & measure.type==.measure.type[.l], select=c(4, 5, 7, 8))
                        is.na(.df) <- is.na(.df) # remove NaNs
                        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.l])
                        .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                        .sa_nh1_nf5_nv1_tbf.plots[[.j]][[.mr.idx]][[.l]] <- .p
                        ## save each plot as a PDF file
                        .p
                        ggsave(paste(.sa_nh1_nf5_nv1_tbf.wd,
                                     paste("nh1_nf5_nv1_tbf-N", as.character(.N.range[.i]),
                                           "_dr", as.character(.dr.range[.j]),
                                           "_mr", as.character(.mr.range[.k]),
                                           "-", .measure.type[.l],
                                           "-", .measure.abbrv[.l], ".pdf", sep=""), sep="/"))
                    }   # end of for(.l)
                }   # end of if()
            }   # end of for(.k)
        }   # end of for(.j)
    }   # end of for(.i)
    ## save data frame and plots for later use
    save(.sa_nh1_nf5_nv1_tbf.df, .sa_nh1_nf5_nv1_tbf.plots,
         file=paste(.sa_nh1_nf5_nv1_tbf.wd, "nh1_nf5_nv1_tbf.RData", sep="/"))
}   # end of if()


#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping for different sizes of TX and TBF queues
#################################################################################
.resp <- readline("Process data from shared access with traffic shaping for different queue sizes? (hit y or n) ")
if (.resp == 'y') {
    if (file.exists(paste(.sa_N10_nh1_nf5_nv1_tbf.wd, 'N10_nh1_nf5_nv1_tbf-test.RData', sep='/')) == FALSE) {
    ## Do the processing of OMNeT++ data files unless there is 'N10_nh1_nf5_nv1_tbf-test.RData' in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.sa_N10_nh1_nf5_nv1_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- .n_totalFiles / .n_repetitions  # number of experiments
        .n_files <- .n_totalFiles / .n_experiments  # number of files per experiment
        .dfs <- list()  # list of data frames from experiments
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.sa_N10_nh1_nf5_nv1_tbf.wd, paste("N10_nh1_nf5_nv1_tbf-test-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""), sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'qs', 'dr', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+qs+dr+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate)',
                                        c('qs', 'dr', 'mr', 'bs', 'n'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+qs+dr+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('qs', 'dr', 'mr', 'bs', 'n'),
                                         'http')
            ## collect decodable frame rate of video traffic
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+qs+dr+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('qs', 'dr', 'mr', 'bs', 'n'),
                                          'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+qs+dr+mr+bs+n+name ~ .",
                                           '.*\\.delayMeter.*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
                                           c('qs', 'dr', 'mr', 'bs', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+qs+dr+mr+bs+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('qs', 'dr', 'mr', 'bs', 'n'),
                                          'queue')
            ## combine the three data frames into one
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet, .tmp_queue)
        }
        ## combine the data frames from experiments into one
        .tmp.df <- .dfs[[1]]
        for (.i in 2:.n_experiments) {
            .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
        }
        ## sort the resulting data frame
        .sa_N10_nh1_nf5_nv1_tbf.df <- sort_df(.tmp.df, vars=c('qs', 'dr', 'mr', 'bs', 'n'))
        ## save data frame and plots for later use
        save(.sa_N10_nh1_nf5_nv1_tbf.df, file=paste(.sa_N10_nh1_nf5_nv1_tbf.wd, "N10_nh1_nf5_nv1_tbf-test.RData", sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.sa_N10_nh1_nf5_nv1_tbf.wd, 'N10_nh1_nf5_nv1_tbf-test.RData', sep='/'))
    }   # end of if() for the processing of OMNeT++ data files

    .qs.range <- unique(.sa_N10_nh1_nf5_nv1_tbf.df$qs)
    .dr.range <- unique(.sa_N10_nh1_nf5_nv1_tbf.df$dr)
    .mr.range <- unique(.sa_N10_nh1_nf5_nv1_tbf.df$mr)
    for (.i in 1:length(.qs.range)) {
        for (.j in 1:length(.dr.range)) {
            .mr.idx <- 0
            for (.k in 1:length(.mr.range)) {
                if (length(subset(.sa_N10_nh1_nf5_nv1_tbf.df, qs==.qs.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k])$mean) > 0) {
                    .mr.idx <- .mr.idx + 1
                    .plots <- list()
                    for (.l in 1:length(.measure.type)) {
                        .df <- subset(.sa_N10_nh1_nf5_nv1_tbf.df, qs==.qs.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k] & name==.measure[.l] & measure.type==.measure.type[.l], select=c(4, 5, 7, 8))
                        is.na(.df) <- is.na(.df) # remove NaNs
                        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.l])
                        .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                        .plots[[.l]] <- .p
                        ## save each plot as a PDF file
                        .p
                        ggsave(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                                     paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                           "_dr", as.character(.dr.range[.j]),
                                           "_mr", as.character(.mr.range[.k]),
                                           "-", .measure.type[.l],
                                           "-", .measure.abbrv[.l], ".pdf", sep=""), sep="/"))
                    }   # end of for(.l)
                    ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                    pdf(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                              paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                    "_dr", as.character(.dr.range[.j]),
                                    "_mr", as.character(.mr.range[.k]),
                                    "-ftp.pdf", sep=""), sep='/'))
                    grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]])
                    dev.off()
                    pdf(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                              paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                    "_dr", as.character(.dr.range[.j]),
                                    "_mr", as.character(.mr.range[.k]),
                                    "-http.pdf", sep=""), sep='/'))
                    grid.arrange(.plots[[4]], .plots[[5]], .plots[[6]], .plots[[7]], .plots[[8]], .plots[[9]], ncol=2)
                    dev.off()
                    pdf(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                              paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                    "_dr", as.character(.dr.range[.j]),
                                    "_mr", as.character(.mr.range[.k]),
                                    "-packet.pdf", sep=""), sep='/'))
                    grid.arrange(.plots[[11]], .plots[[12]], .plots[[13]], .plots[[14]], ncol=2)
                    dev.off()
                    pdf(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                              paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                    "_dr", as.character(.dr.range[.j]),
                                    "_mr", as.character(.mr.range[.k]),
                                    "-queue.pdf", sep=""), sep='/'))
                    grid.arrange(.plots[[15]], .plots[[16]])
                    dev.off()
                }   # end of if()
            }   # end of for(.k)
        }   # end of for(.j)
    }   # end of for(.i)
    ## save(.sa_N10_nh1_nf5_nv1_tbf.df, .sa_N10_nh1_nf5_nv1_tbf.plots,
    ##      file=paste(.sa_N10_nh1_nf5_nv1_tbf.wd, "N10_nh1_nf5_nv1_tbf-test.RData", sep="/"))
}   # end of if()
