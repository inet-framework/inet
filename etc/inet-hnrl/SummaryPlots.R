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
        .base.directory <- "E:/tools/omnetpp/inet-hnrl/examples/ngoa/isp_traffic_shaping",
        .base.directory <- "~/inet-hnrl/examples/ngoa/isp_traffic_shaping")
source(paste(.base.directory, 'scripts/collectMeasures.R', sep='/'))
source(paste(.base.directory, 'scripts/calculateFairnessIndexes.R', sep='/'))
source(paste(.base.directory, 'scripts/collectMeasuresAndFIs.R', sep='/'))
###
### define variables
###
## dedicated access for n_h=1, n_f=5, n_v=1
.da_nh1_nf1_nv1.wd <- paste(.base.directory, "results/Dedicated/nh1_nf1_nv1", sep="/")
.da_nh1_nf1_nv1_tbf.wd <- paste(.base.directory, "results/Dedicated/nh1_nf1_nv1_tbf", sep="/")
## shared access for n_h=1, n_f=5, n_v=1
.sa_nh1_nf1_nv1_tbf.wd <- paste(.base.directory, "results/Shared/nh1_nf1_nv1_tbf", sep="/")
.sa_N10_nh1_nf1_nv1_tbf.wd <- paste(.base.directory, "results/Shared/test/N10_nh1_nf1_nv1_tbf-test", sep="/")
## for plotting
.measure <-  c('average session delay [s]',
               '90th-sessionDelay:percentile',
               '95th-sessionDelay:percentile',
               '99th-sessionDelay:percentile',
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
.measure.type<- c("ftp", "ftp", "ftp", "ftp", "ftp", "ftp", "http", "http", "http", "http", "http", "http", "video", "packet", "packet", "packet", "packet", "queue", "queue")
.measure.abbrv <- c('dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf', 'dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf', 'dfr', 'dly', '90p-dly', '95p-dly', '99p-dly', 'plr', 'psr')
.labels.traffic <- c("FTP", "FTP", "FTP", "FTP", "FTP", "FTP", "HTTP", "HTTP", "HTTP", "HTTP", "HTTP", "HTTP", "UDP Streaming Video")
.labels.measure <- c("Average Session Delay [sec]",
                     "90-Percentile Session Delay [sec]",
                     "95-Percentile Session Delay [sec]",
                     "99-Percentile Session Delay [sec]",
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
    .config <- readline("Type OMNeT++ configuration name: ")
    .da.wd <- paste(.base.directory, "results/Dedicated", .config, sep="/")
    .da.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.da.wd, .da.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.da.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- ceiling(.n_totalFiles/.n_repetitions)   # number of experiments
        .n_files <- ceiling(.n_totalFiles/.n_experiments)   # number of files per experiment
        .scalars_dfs <- list()  # list of OMNeT++ scalar data frames from experiments
        .dfs <- list()  # list of processed data frames from experiments
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.da.wd,
                                        paste(paste(.config, "-", sep=""),
                                              as.character((.i-1)*.n_files+.j-1),
                                              ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
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
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+dr+n+name ~ .",
                                           '.*\\.delayMeter.*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
                                           c('dr', 'n'),
                                           'packet')
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet)
        }   # end of for(.i)
        ## combine the data frames from experiments into one
        .scalars_df <- .scalars_dfs[[1]]
        .df <- .dfs[[1]]
        for (.i in 2:.n_experiments) {
            .scalars_df <- rbind(.scalars_df, .scalars_dfs[[.i]])
            .df <- rbind(.df, .dfs[[.i]])
        }
        ## save data frames for later use
        .scalars_df.name <- paste('.da_scalars_', .config, '.df', sep='')
        .df.name <- paste('.da_', .config, '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.df, vars=c('dr', 'n')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.da.wd, .da.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.da.wd, .da.rdata, sep='/'))
    }   # end of if() for the processing of OMNeT++ data files
    .da.df <- get(.df.name)
    .plots <- list()
    for (.i in 1:(length(.measure.type)-2)) { # subtract 2 because there are no types for 'queue'
        .df <- subset(.da.df, name==.measure[.i] & measure.type==.measure.type[.i], select=c(1, 2, 4, 5))
        is.na(.df) <- is.na(.df) # remove NaNs
        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
        .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
        .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
        .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Gb/s]", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Gb/s]")
        .plots[[.i]] <- .p
        ## save each plot as a PDF file
        .p
        ggsave(paste(.da.wd,
                     paste(paste(.config, .measure.type[.i], .measure.abbrv[.i], sep="-"), "pdf", sep="."),
                     sep="/"))
    }   # end of for(.i)
    ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
    pdf(paste(.da.wd, paste(.config, "ftp.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
    dev.off()
    pdf(paste(.da.wd, paste(.config, "http.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
    dev.off()
    pdf(paste(.da.wd, paste(.config, "packet.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], ncol=2)
    dev.off()
}   # end of if()
#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.resp <- readline("Process data from dedicated access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    if (file.exists(paste(.da_nh1_nf1_nv1_tbf.wd, 'nh1_nf1_nv1_tbf.RData', sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is 'nh1_nf1_nv1_tbf.RData' in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.da_nh1_nf1_nv1_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- ceiling(.n_totalFiles/.n_repetitions)   # number of experiments
        .n_files <- ceiling(.n_totalFiles/.n_experiments)   # number of files per experiment
        .scalars_dfs <- list()  # list of OMNeT++ scalar data frames from experiments
        .dfs <- list()  # list of processed data frames from experiments
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.da_nh1_nf1_nv1_tbf.wd, paste("nh1_nf1_nv1_tbf-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""), sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
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
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+dr+mr+bs+n+name ~ .",
                                           '.*\\.delayMeter.*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
                                           c('dr', 'mr', 'bs', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+mr+bs+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('dr', 'mr', 'bs', 'n'),
                                          'queue')
            ## combine the five data frames into one
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet, .tmp_queue)
        }   # end of for(.i)
        ## combine the data frames from experiments into one
        .scalars_df <- .scalars_dfs[[1]]
        .df <- .dfs[[1]]
        for (.i in 2:.n_experiments) {
            .scalars_df <- rbind(.scalars_df, .scalars_dfs[[.i]])
            .df <- rbind(.df, .dfs[[.i]])
        }
        ## save data frames for later use
        .scalars_df.name <- paste('.da_scalars_', 'nh1_nf1_nv1_tbf', '.df', sep='')
        .df.name <- paste('.da_', 'nh1_nf1_nv1_tbf', '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.df, vars=c('dr', 'mr', 'bs', 'n')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.da_nh1_nf1_nv1_tbf.wd, "nh1_nf1_nv1_tbf.RData", sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.da_nh1_nf1_nv1_tbf.wd, 'nh1_nf1_nv1_tbf.RData', sep='/'))
    }   # end of if() for the processing of OMNeT++ data files
    .da_nh1_nf1_nv1_tbf.df <- get(.df.name)
    .dr.range <- unique(.da_nh1_nf1_nv1_tbf.df$dr)
    .mr.range <- unique(.da_nh1_nf1_nv1_tbf.df$mr)
    for (.i in 1:length(.dr.range)) {
        for (.j in 1:length(.mr.range)) {
            if (length(subset(.da_nh1_nf1_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                for (.k in 1:length(.measure.type)) {
                    .df <- subset(.da_nh1_nf1_nv1_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(3, 4, 6, 7))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    ggsave(paste(.da_nh1_nf1_nv1_tbf.wd,
                                 paste("nh1_nf1_nv1_tbf_dr", as.character(.dr.range[.i]),
                                       "_mr", as.character(.mr.range[.j]),
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.da_nh1_nf1_nv1_tbf.wd,
                          paste("nh1_nf1_nv1_tbf_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.da_nh1_nf1_nv1_tbf.wd,
                          paste("nh1_nf1_nv1_tbf_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
                pdf(paste(.da_nh1_nf1_nv1_tbf.wd,
                          paste("nh1_nf1_nv1_tbf_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-packet.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], ncol=2)
                dev.off()
                pdf(paste(.da_nh1_nf1_nv1_tbf.wd,
                          paste("nh1_nf1_nv1_tbf_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-queue.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[18]], .plots[[19]])
                dev.off()
            }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
}   # end of if()
#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping
#################################################################################
.resp <- readline("Process data from shared access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .sa_tbf.wd <- paste(.base.directory, "results/Shared", .config, sep="/")
    .sa_tbf.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.sa_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- ceiling(.n_totalFiles/.n_repetitions)   # number of experiments
        .n_files <- ceiling(.n_totalFiles/.n_experiments)   # number of files per experiment
        .dfs <- list()  # list of data frames from experiments for performances measures
        .dfs_fi <- list()  # list of data frames from experiments for fairness indexes
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.sa_tbf.wd,
                                        paste(paste(.config, "-", sep=""),
                                              as.character((.i-1)*.n_files+.j-1),
                                              ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'module', 'N', 'dr', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic, and their fairness indexes
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('N', 'dr', 'mr', 'bs', 'n'),
                                        'ftp')
            .tmp_ftp_fi <- calculateFairnessIndexes(.scalars,
                                                    "N+dr+mr+bs+n+name ~ .",
                                                    '.*\\.ftpApp.*',
                                                    '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                    c('N', 'dr', 'mr', 'bs', 'n'),
                                                    'ftp')
            ## collect average & percentile session delays, average session throughput, mean session transfer rate of HTTP traffic, and their fairness indexes
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('N', 'dr', 'mr', 'bs', 'n'),
                                         'http')
            .tmp_http_fi <- calculateFairnessIndexes(.scalars,
                                                     "N+dr+mr+bs+n+name ~ .",
                                                     '.*\\.httpApp.*',
                                                     '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                     c('N', 'dr', 'mr', 'bs', 'n'),
                                                     'http')
            ## collect decodable frame rate of video traffic and its fairness index
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('N', 'dr', 'mr', 'bs', 'n'),
                                          'video')
            .tmp_video_fi <- calculateFairnessIndexes(.scalars,
                                                      "N+dr+mr+bs+n+name ~ .",
                                                      '.*\\.videoApp.*',
                                                      'decodable frame rate',
                                                      c('N', 'dr', 'mr', 'bs', 'n'),
                                                      'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                           '.*\\.delayMeter.*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
                                           c('N', 'dr', 'mr', 'bs', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('N', 'dr', 'mr', 'bs', 'n'),
                                          'queue')
            ## combine data frames into one
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet, .tmp_queue)
            .dfs_fi[[.i]] <- rbind(.tmp_ftp_fi, .tmp_http_fi, .tmp_video_fi)
        }
        ## combine the data frames from experiments into one
        .tmp.df <- .dfs[[1]]
        .tmp_fi.df <- .dfs_fi[[1]]
        for (.i in 2:.n_experiments) {
            .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
            .tmp_fi.df <- rbind(.tmp_fi.df, .dfs_fi[[.i]])
        }
        ## sort the resulting data frames
        .sa_tbf.df <- sort_df(.tmp.df, vars=c('N', 'dr', 'mr', 'bs', 'n'))
        .sa_tbf_fi.df <- sort_df(.tmp_fi.df, vars=c('N', 'dr', 'mr', 'bs', 'n'))
        ## save data frames for later use
        save(.sa_tbf.df, .sa_tbf_fi.df, file=paste(.sa_tbf.wd, .sa_tbf.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/'))
    }   # end of if() for the processing of OMNeT++ data files
    .dr.range <- unique(.sa_tbf.df$dr)
    .mr.range <- unique(.sa_tbf.df$mr)
    for (.i in 1:length(.dr.range)) {
        .mr.idx <- 0
        for (.j in 1:length(.mr.range)) {
            ## process performance measures
            if (length(subset(.sa_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                for (.k in 1:length(.measure.type)) {
                    .df <- subset(.sa_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(4, 5, 7, 8))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    ggsave(paste(.sa_tbf.wd,
                                 paste(.config,
                                       "_dr", as.character(.dr.range[.i]),
                                       "_mr", as.character(.mr.range[.j]),
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k],
                                       ".pdf",
                                       sep=""),
                                 sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-packet.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], ncol=2)
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-queue.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[18]], .plots[[19]])
                dev.off()
            }   # end of if()
            ## process fairness indexes
            if (length(subset(.sa_tbf_fi.df, dr==.dr.range[.i] & mr==.mr.range[.j])$fairness.index) > 0) {
                .plots <- list()
                for (.k in 1:(length(.measure.type)-6)) { # subtract 6 because we do not include 'packet' and 'queue' in the processing
                    .df <- subset(.sa_tbf_fi.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(4, 5, 7))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    ## if (length(.df$fairness.index) > length(unique(.df$bs))) {
                        .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=fairness.index)) + geom_line() + scale_y_continuous(limits=c(0.75, 1))
                    ## }
                    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(paste("Fairness Index of ", .labels.measure[.k], sep=""))
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=fairness.index), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + scale_colour_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    ggsave(paste(.sa_tbf.wd,
                                 paste(.config,
                                       "_dr", as.character(.dr.range[.i]),
                                       "_mr", as.character(.mr.range[.j]),
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k],
                                       "-fi.pdf",
                                       sep=""),
                                 sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-ftp-fi.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-http-fi.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
            }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
}   # end of if()
#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping
###
### Note: This is mainly for FOAN 2012 paper.
#################################################################################
.resp <- readline("Process data from shared access with traffic shaping (for FOAN 2012 paper)? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .sa_tbf.wd <- paste(.base.directory, "results/Shared", .config, sep="/")
    .sa_tbf.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- as.numeric(system(paste('ls -l ', paste(.sa_tbf.wd, '*.sca', sep='/'), ' | wc -l', sep=''), intern=TRUE))
                                        # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- ceiling(.n_totalFiles/.n_repetitions)   # number of experiments
        .n_files <- ceiling(.n_totalFiles/.n_experiments)   # number of files per experiment
        .scalars_dfs <- list()  # list of OMNeT++ scalar data frames from experiments
        .dfs <- list()  # list of data frames from experiments for performances measures
        .dfs_fi <- list()  # list of data frames from experiments for fairness indexes
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.sa_tbf.wd,
                                        paste(paste(.config, "-", sep=""),
                                              as.character((.i-1)*.n_files+.j-1),
                                              ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'module', 'N', 'dr', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic, and their fairness indexes
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('N', 'dr', 'mr', 'bs', 'n'),
                                        'ftp')
            .tmp_ftp_fi <- calculateFairnessIndexes(.scalars,
                                                    "N+dr+mr+bs+n+name ~ .",
                                                    '.*\\.ftpApp.*',
                                                    '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                    c('N', 'dr', 'mr', 'bs', 'n'),
                                                    'ftp')
            ## collect average & percentile session delays, average session throughput, mean session transfer rate of HTTP traffic, and their fairness indexes
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('N', 'dr', 'mr', 'bs', 'n'),
                                         'http')
            .tmp_http_fi <- calculateFairnessIndexes(.scalars,
                                                     "N+dr+mr+bs+n+name ~ .",
                                                     '.*\\.httpApp.*',
                                                     '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                     c('N', 'dr', 'mr', 'bs', 'n'),
                                                     'http')
            ## collect decodable frame rate of video traffic and its fairness index
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('N', 'dr', 'mr', 'bs', 'n'),
                                          'video')
            .tmp_video_fi <- calculateFairnessIndexes(.scalars,
                                                      "N+dr+mr+bs+n+name ~ .",
                                                      '.*\\.videoApp.*',
                                                      'decodable frame rate',
                                                      c('N', 'dr', 'mr', 'bs', 'n'),
                                                      'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                           '.*\\.delayMeter.*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
                                           c('N', 'dr', 'mr', 'bs', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+N+dr+mr+bs+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('N', 'dr', 'mr', 'bs', 'n'),
                                          'queue')
            ## combine data frames into one
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet, .tmp_queue)
            .dfs_fi[[.i]] <- rbind(.tmp_ftp_fi, .tmp_http_fi, .tmp_video_fi)
        }   # end of for(.i)
        ## combine the data frames from experiments into one
        .scalars_df <- .scalars_dfs[[1]]
        .tmp.df <- .dfs[[1]]
        .tmp_fi.df <- .dfs_fi[[1]]
        for (.i in 2:.n_experiments) {
            .scalars_df <- rbind(.scalars_df, .scalars_dfs[[.i]])
            .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
            .tmp_fi.df <- rbind(.tmp_fi.df, .dfs_fi[[.i]])
        }
        ## save data frames for later use
        .scalars_df.name <- paste('.sa_tbf_scalars_', .config, '.df', sep='')
        .df.name <- paste('.sa_tbf_', .config, '.df', sep='')
        .fi_df.name <- paste('.sa_tbf_fi_', .config, '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.tmp.df, vars=c('N', 'dr', 'mr', 'bs', 'n')))
        assign(.fi_df.name, sort_df(.tmp_fi.df, vars=c('N', 'dr', 'mr', 'bs', 'n')))
        save(list=c(.scalars_df.name, .df.name, .fi_df.name), file=paste(.sa_tbf.wd, .sa_tbf.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/'))
    }   # end of if() for the processing of OMNeT++ data files
    .sa_tbf.df <- get(.df.name)
    .sa_tbf_fi.df <- get(.fi_df.name)
    .dr.range <- unique(.sa_tbf.df$dr)
    .mr.range <- unique(.sa_tbf.df$mr)
    for (.i in 1:length(.dr.range)) {
        .mr.idx <- 0
        for (.j in 1:length(.mr.range)) {
            ## process performance measures
            if (length(subset(.sa_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                for (.k in 1:length(.measure.type)) {
                    .df <- subset(.sa_tbf.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(1, 4, 7, 8))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    ## .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=N, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- ggplot(data=.df, aes(group=bs, linetype=factor(bs), x=N, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Subscribers (N)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=N, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    ## .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_linetype_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    ggsave(paste(.sa_tbf.wd,
                                 paste(.config,
                                       "_dr", as.character(.dr.range[.i]),
                                       "_mr", as.character(.mr.range[.j]),
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k],
                                       ".pdf",
                                       sep=""),
                                 sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-packet.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], ncol=2)
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_dr", as.character(.dr.range[.i]),
                                "_mr", as.character(.mr.range[.j]),
                                "-queue.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[18]], .plots[[19]])
                dev.off()
            }   # end of if()
            ## ## process fairness indexes
            ## if (length(subset(.sa_tbf_fi.df, dr==.dr.range[.i] & mr==.mr.range[.j])$fairness.index) > 0) {
            ##     .plots <- list()
            ##     for (.k in 1:(length(.measure.type)-6)) { # subtract 6 because we do not include 'packet' and 'queue' in the processing
            ##         .df <- subset(.sa_tbf_fi.df, dr==.dr.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(4, 5, 7))
            ##         is.na(.df) <- is.na(.df) # remove NaNs
            ##         ## if (length(.df$fairness.index) > length(unique(.df$bs))) {
            ##             .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=fairness.index)) + geom_line() + scale_y_continuous(limits=c(0.75, 1))
            ##         ## }
            ##         .p <- .p + xlab("Number of Users per ONU (n)") + ylab(paste("Fairness Index of ", .labels.measure[.k], sep=""))
            ##         .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=fairness.index), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
            ##         .p <- .p + scale_colour_discrete("Burst Size\n[MB]")
            ##         .plots[[.k]] <- .p
            ##         ## save each plot as a PDF file
            ##         .p
            ##         ggsave(paste(.sa_tbf.wd,
            ##                      paste(.config,
            ##                            "_dr", as.character(.dr.range[.i]),
            ##                            "_mr", as.character(.mr.range[.j]),
            ##                            "-", .measure.type[.k],
            ##                            "-", .measure.abbrv[.k],
            ##                            "-fi.pdf",
            ##                            sep=""),
            ##                      sep="/"))
            ##     }   # end of for(.k)
            ##     ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
            ##     pdf(paste(.sa_tbf.wd,
            ##               paste(.config,
            ##                     "_dr", as.character(.dr.range[.i]),
            ##                     "_mr", as.character(.mr.range[.j]),
            ##                     "-ftp-fi.pdf", sep=""), sep='/'))
            ##     grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
            ##     dev.off()
            ##     pdf(paste(.sa_tbf.wd,
            ##               paste(.config,
            ##                     "_dr", as.character(.dr.range[.i]),
            ##                     "_mr", as.character(.mr.range[.j]),
            ##                     "-http-fi.pdf", sep=""), sep='/'))
            ##     grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
            ##     dev.off()
            ## }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
}   # end of if()
