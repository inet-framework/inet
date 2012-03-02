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
.traffic <- c("ftp", "ftp", "ftp", "http", "http", "http", "http", "http", "http", "video")
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
               'average frame delay [s]',
               '90th-frameDelay:percentile',
               '95th-frameDelay:percentile',
               '99th-frameDelay:percentile',
               'packets dropped by per-VLAN queue',
               'packets shaped by per-VLAN queue')
.measure_abbrv <- c('dly', 'thr', 'trf', 'dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf', 'dfr')
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
                     "Average DS Frame Delay [sec]",
                     "90-Percentile DS Frame Delay [sec]",
                     "95-Percentile DS Frame Delay [sec]",
                     "99-Percentile DS Frame Delay [sec]",
                     "DS Frame Loss Rate",
                     "DS Frame Shaping Rate"
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
    for (.i in 1:length(.traffic)) {
        .df <- subset(.da_nh1_nf5_nv1.df, name==.measure[.i] & traffic==.traffic[.i], select=c(1, 2, 4, 5))
        is.na(.df) <- is.na(.df) # remove NaNs
        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
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
}   # end of if()


#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.resp <- readline("Process data from dedicated access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
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
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
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
                    for (.l in 1:length(.traffic)) {
                        .df <- subset(.sa_nh1_nf5_nv1_tbf.df, N==.N.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k] & name==.measure[.l] & traffic==.traffic[.l], select=c(4, 5, 7, 8))
                        is.na(.df) <- is.na(.df) # remove NaNs
                        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.l])
                        .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                        .sa_nh1_nf5_nv1_tbf.plots[[.j]][[.mr.idx]][[.l]] <- .p
                        ## save as a PDF file
                        .p
                        ggsave(paste(.sa_nh1_nf5_nv1_tbf.wd,
                                     paste("nh1_nf5_nv1_tbf-N", as.character(.N.range[.i]),
                                           "_dr", as.character(.dr.range[.j]),
                                           "_mr", as.character(.mr.range[.k]),
                                           "-", .traffic[.l],
                                           "-", .measure_abbrv[.l], ".pdf", sep=""), sep="/"))
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
        ## combine the three data frames into one
        .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
    }
    ## combine the data frames from experiments into one
    .tmp.df <- .dfs[[1]]
    for (.i in 2:.n_experiments) {
        .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
    }
    ## sort the resulting data frame
    .sa_N10_nh1_nf5_nv1_tbf.df <- sort_df(.tmp.df, vars=c('qs', 'dr', 'mr', 'bs', 'n'))
    .qs.range <- unique(.sa_N10_nh1_nf5_nv1_tbf.df$qs)

    .mr.range <- unique(.sa_N10_nh1_nf5_nv1_tbf.df$mr)
    ## .sa_N10_nh1_nf5_nv1_tbf.plots <- list()
    for (.i in 1:length(.qs.range)) {
        for (.j in 1:length(.dr.range)) {
            ## .sa_N10_nh1_nf5_nv1_tbf.plots[[.j]] <- list()
            .mr.idx <- 0
            for (.k in 1:length(.mr.range)) {
                if (length(subset(.sa_N10_nh1_nf5_nv1_tbf.df, qs==.qs.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k])$mean) > 0) {
                    .mr.idx <- .mr.idx + 1
                    ## .sa_N10_nh1_nf5_nv1_tbf.plots[[.j]][[.mr.idx]] <- list()
                    for (.l in 1:length(.traffic)) {
                        .df <- subset(.sa_N10_nh1_nf5_nv1_tbf.df, qs==.qs.range[.i] & dr==.dr.range[.j] & mr==.mr.range[.k] & name==.measure[.l] & traffic==.traffic[.l], select=c(4, 5, 7, 8))
                        is.na(.df) <- is.na(.df) # remove NaNs
                        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.l])
                        .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[Byte]", values=0:9)
                        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[Byte]")
                        ## .sa_N10_nh1_nf5_nv1_tbf.plots[[.j]][[.mr.idx]][[.l]] <- .p
                        ## save as a PDF file
                        .p
                        ggsave(paste(.sa_N10_nh1_nf5_nv1_tbf.wd,
                                     paste("N10_nh1_nf5_nv1_tbf-test-qs", as.character(.qs.range[.i]),
                                           "_dr", as.character(.dr.range[.j]),
                                           "_mr", as.character(.mr.range[.k]),
                                           "-", .traffic[.l],
                                           "-", .measure_abbrv[.l], ".pdf", sep=""), sep="/"))
                    }   # end of for(.l)
                }   # end of if()
            }   # end of for(.k)
        }   # end of for(.j)
    }   # end of for(.i)
    ## save data frame and plots for later use
    save(.sa_N10_nh1_nf5_nv1_tbf.df, .sa_N10_nh1_nf5_nv1_tbf.plots,
         file=paste(.sa_N10_nh1_nf5_nv1_tbf.wd, "N10_nh1_nf5_nv1_tbf-test.RData", sep="/"))
}   # end of if()
