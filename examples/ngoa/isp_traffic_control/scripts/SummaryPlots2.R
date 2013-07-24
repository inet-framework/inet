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
library(reshape)
###
### directory settings
###
ifelse (Sys.getenv("OS") == "Windows_NT",
        .root.directory <- "E:/tools/omnetpp/inet-hnrl",
        .root.directory <- "~/inet-hnrl")
.base.directory <- paste(.root.directory, 'examples/ngoa/isp_traffic_control', sep='/')
.script.directory <- paste(.root.directory, 'etc/inet-hnrl', sep='/')
###
### source scripts
###
source(paste(.script.directory, 'calculateFairnessIndexes.R', sep='/'))
source(paste(.script.directory, 'collectMeasures.R', sep='/'))
source(paste(.script.directory, 'collectMeasuresAndFIs.R', sep='/'))
source(paste(.script.directory, 'confidenceInterval.R', sep='/'))
source('./Configurations.R')
###
### define new functions
###
## 95% confidence interval
ci.width <- confidenceInterval(0.95)
## pause before and after processing
Pause <- function () {
    cat("Hit <enter> to continue...")
    readline()
    invisible()
}
###
### define variables
###
.csfq-tbm.wd <- paste(.base.directory, "results/csfq-tbm", sep="/")
.drr-tbm.wd <- paste(.base.directory, "results/drr-tbm", sep="/")
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
               'bits/sec rcvd',
               'overall packet loss rate of per-VLAN queues',
               'overall packet shaped rate of per-VLAN queues')
.measure.type<- c("ftp", "ftp", "ftp", "ftp", "ftp", "ftp",
                  "http", "http", "http", "http", "http", "http",
                  "video",
                  "packet", "packet", "packet", "packet", "packet",
                  "queue", "queue")
.measure.abbrv <- c('dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf',
                    'dly', '90p-dly', '95p-dly', '99p-dly', 'thr', 'trf',
                    'dfr',
                    'dly', '90p-dly', '95p-dly', '99p-dly', 'thr',
                    'plr', 'psr')
.labels.traffic <- c("FTP", "FTP", "FTP", "FTP", "FTP", "FTP",
                     "HTTP", "HTTP", "HTTP", "HTTP", "HTTP", "HTTP",
                     "UDP Streaming Video")
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
                     'Average Packet Throughput [b/s]',
                     "DS Packet Loss Rate",
                     "DS Packet Shaped Rate"
                     )
###
### customize
###
.old <- theme_set(theme_bw())
.pt_size <- 3.5
#################################################################################
### summary plots for static configurations with UDP traffic
###
### Note: This is for the IEEE Communications Letters paper.
#################################################################################
.resp <- readline("Process data from static configurations with UDP traffic? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .static_udp.wd <- paste(.base.directory, "results/Dedicated", .config, sep="/")
    .da.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.static_udp.wd, .da.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- length(list.files(path=.static_udp.wd, pattern='*.sca$'))  # total number of (scalar) files to process
        .n_repetitions <- 10    # number of repetitions per experiment
        .n_experiments <- ceiling(.n_totalFiles/.n_repetitions)   # number of experiments
        .n_files <- ceiling(.n_totalFiles/.n_experiments)   # number of files per experiment
        .scalars_dfs <- list()  # list of OMNeT++ scalar data frames from experiments
        .dfs <- list()  # list of processed data frames from experiments
        .fileNames <- rep('', .n_files)  # vector of file names
        for (.i in 1:.n_experiments) {
            cat(paste("Processing ", as.character(.i), "th experiment ...\n", sep=""))
            for (.j in 1:.n_files) {
                .fileNames[.j] <- paste(.static_udp.wd,
                                        paste(.config, "-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'ur', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+ur+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('dr', 'ur', 'n'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+dr+ur+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('dr', 'ur', 'n'),
                                         'http')
            ## collect decodable frame rate of video traffic
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+ur+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('dr', 'ur', 'n'),
                                          'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+dr+ur+n+name ~ .",
                                           '.*\\.host\\[.*\\]\\.eth\\[0\\].*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile|bits/sec rcvd)',
                                           c('dr', 'ur', 'n'),
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
        assign(.df.name, sort_df(.df, vars=c('dr', 'ur', 'n')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.static_udp.wd, .da.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.static_udp.wd, .da.rdata, sep='/'))
        .df.name <- paste('.da_', .config, '.df', sep='')
    }   # end of if() for the processing of OMNeT++ data files
    .da.df <- get(.df.name)
    .plots <- list()
    for (.i in 1:(length(.measure.type)-2)) { # subtract 2 because there are no types for 'queue'
        .df <- subset(.da.df, name==.measure[.i] & measure.type==.measure.type[.i], select=c(1, 2, 3, 5, 6))
        is.na(.df) <- is.na(.df) # remove NaNs
        .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
        .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
        .p <- ggplot(data=.df, aes(group=dr, colour=factor(dr), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
        .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.i])
        .p <- .p + geom_point(aes(group=dr, shape=factor(dr), x=n, y=mean), size=.pt_size) + scale_shape_manual("Line Rate\n[Mb/s]", values=0:9)
        .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Line Rate\n[Mb/s]")
        #.p <- .p + stat_summary(fun.data=mean_cl_boot, geom='errorbar', width=0.1) + scale_colour_discrete("Line Rate\n[Mb/s]")
        .plots[[.i]] <- .p
        ## save each plot as a PDF file
        .p
        ggsave(paste(.static_udp.wd,
                     paste(paste(.config, .measure.type[.i], .measure.abbrv[.i], sep="-"), "pdf", sep="."),
                     sep="/"))
    }   # end of for(.i)
    ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
    pdf(paste(.static_udp.wd, paste(.config, "ftp.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
    dev.off()
    pdf(paste(.static_udp.wd, paste(.config, "http.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
    dev.off()
    pdf(paste(.static_udp.wd, paste(.config, "packet.pdf", sep="-"), sep='/'))
    grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], .plots[[18]], ncol=2)
    dev.off()
}   # end of if()
#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping
###
### Note: This is for the IEEE Transactions on Networking paper.
#################################################################################
.resp <- readline("Process data from dedicated access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .da.wd <- paste(.base.directory, "results/Dedicated", .config, sep="/")
    .da.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.da.wd, .da.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- length(list.files(path=.da.wd, pattern='*.sca$'))  # total number of (scalar) files to process
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
                                        paste(.config, "-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'ur', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+ur+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('dr', 'ur', 'mr', 'bs', 'n'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+dr+ur+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('dr', 'ur', 'mr', 'bs', 'n'),
                                         'http')
            ## collect decodable frame rate of video traffic
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+ur+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('dr', 'ur', 'mr', 'bs', 'n'),
                                          'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+dr+ur+mr+bs+n+name ~ .",
                                           '.*\\.host\\[.*\\]\\.eth\\[0\\].*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile|bits/sec rcvd)',
                                           c('dr', 'ur', 'mr', 'bs', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+ur+mr+bs+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('dr', 'ur', 'mr', 'bs', 'n'),
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
        .scalars_df.name <- paste('.da_scalars_', .config, '.df', sep='')
        .df.name <- paste('.da_', .config, '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.df, vars=c('dr', 'ur', 'mr', 'bs', 'n')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.da.wd, .da.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.da.wd, .da.rdata, sep='/'))
        .df.name <- paste('.da_', .config, '.df', sep='')
    }   # end of if() for the processing of OMNeT++ data files
    .da.df <- get(.df.name)
    .ur.range <- unique(.da.df$ur)
    .ur.unit <- ''
    .mr.range <- unique(.da.df$mr)
    for (.i in 1:length(.ur.range)) {
        for (.j in 1:length(.mr.range)) {
            if (length(subset(.da.df, ur==.ur.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                for (.k in 1:length(.measure.type)) {
                    .df <- subset(.da.df, ur==.ur.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(4, 5, 7, 8))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    #.p <- .p + stat_summary(fun.data=mean_cl_boot, geom='errorbar', width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    if (.ur.range[.i] == 1) {
                        .ur.unit <- 'G'
                    }
                    else if (.ur.range[.i] == 100) {
                        .ur.unit <- 'M'
                    }
                    else {
                        stop("Unknown value of 'ur'.")
                    }
                    ggsave(paste(.da.wd,
                                 paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                       "_mr", as.character(.mr.range[.j]), 'M',
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.da.wd,
                          paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.da.wd,
                          paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
                pdf(paste(.da.wd,
                          paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-packet.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], .plots[[18]], ncol=2)
                dev.off()
                pdf(paste(.da.wd,
                          paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-queue.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[19]], .plots[[20]])
                dev.off()
            }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
}   # end of if()
#################################################################################
### summary plots for dedicated architecture with HTTP, FTP, and video traffic
### and traffic shaping with peak rate control
###
### Note: This is for the IEEE Transactions on Networking paper.
#################################################################################
.resp <- readline("Process data from dedicated access with traffic shaping and peak rate control? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .da.wd <- paste(.base.directory, "results/Dedicated", .config, sep="/")
    .da.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.da.wd, .da.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- length(list.files(path=.da.wd, pattern='*.sca$'))  # total number of (scalar) files to process
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
                                        paste(.config, "-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'dr', 'ur', 'mr', 'bs', 'pr', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+dr+ur+mr+bs+pr+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('dr', 'ur', 'mr', 'bs', 'pr', 'n'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, and mean session transfer rate of HTTP traffic
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+dr+ur+mr+bs+pr+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('dr', 'ur', 'mr', 'bs', 'pr', 'n'),
                                         'http')
            ## collect decodable frame rate of video traffic
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+ur+mr+bs+pr+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('dr', 'ur', 'mr', 'bs', 'pr', 'n'),
                                          'video')
            ## collect average & percentile packet delays from DelayMeter module
            .tmp_packet <- collectMeasures(.scalars,
                                           "experiment+measurement+dr+ur+mr+bs+pr+n+name ~ .",
                                           '.*\\.host\\[.*\\]\\.eth\\[0\\].*',
                                           '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile|bits/sec rcvd)',
                                           c('dr', 'ur', 'mr', 'bs', 'pr', 'n'),
                                           'packet')
            ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            .tmp_queue <- collectMeasures(.scalars,
                                          "experiment+measurement+dr+ur+mr+bs+pr+n+name ~ .",
                                          '.*\\.olt.*',
                                          '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
                                          c('dr', 'ur', 'mr', 'bs', 'pr', 'n'),
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
        .scalars_df.name <- paste('.da_scalars_', .config, '.df', sep='')
        .df.name <- paste('.da_', .config, '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.df, vars=c('dr', 'ur', 'mr', 'bs', 'pr', 'n')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.da.wd, .da.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.da.wd, .da.rdata, sep='/'))
        .df.name <- paste('.da_', .config, '.df', sep='')
    }   # end of if() for the processing of OMNeT++ data files
    .da.df <- get(.df.name)
    ## .ur.range <- unique(.da.df$ur)
    ## .ur.unit <- ''
    ## .mr.range <- unique(.da.df$mr)
    ## for (.i in 1:length(.ur.range)) {
    ##     for (.j in 1:length(.mr.range)) {
    ##         if (length(subset(.da.df, ur==.ur.range[.i] & mr==.mr.range[.j])$mean) > 0) {
    ##             .plots <- list()
    ##             for (.k in 1:length(.measure.type)) {
    ##                 .df <- subset(.da.df, ur==.ur.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(4, 5, 7, 8))
    ##                 is.na(.df) <- is.na(.df) # remove NaNs
    ##                 .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
    ##                 .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
    ##                 .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=n, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
    ##                 .p <- .p + xlab("Number of Users per ONU (n)") + ylab(.labels.measure[.k])
    ##                 .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=n, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
    ##                 .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
    ##                 .plots[[.k]] <- .p
    ##                 ## save each plot as a PDF file
    ##                 .p
    ##                 if (.ur.range[.i] == 1) {
    ##                     .ur.unit <- 'G'
    ##                 }
    ##                 else if (.ur.range[.i] == 100) {
    ##                     .ur.unit <- 'M'
    ##                 }
    ##                 else {
    ##                     stop("Unknown value of 'ur'.")
    ##                 }
    ##                 ggsave(paste(.da.wd,
    ##                              paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
    ##                                    "_mr", as.character(.mr.range[.j]), 'M',
    ##                                    "-", .measure.type[.k],
    ##                                    "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
    ##             }   # end of for(.k)
    ##             ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
    ##             pdf(paste(.da.wd,
    ##                       paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
    ##                             "_mr", as.character(.mr.range[.j]), 'M',
    ##                             "-ftp.pdf", sep=""), sep='/'))
    ##             grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
    ##             dev.off()
    ##             pdf(paste(.da.wd,
    ##                       paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
    ##                             "_mr", as.character(.mr.range[.j]), 'M',
    ##                             "-http.pdf", sep=""), sep='/'))
    ##             grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
    ##             dev.off()
    ##             pdf(paste(.da.wd,
    ##                       paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
    ##                             "_mr", as.character(.mr.range[.j]), 'M',
    ##                             "-packet.pdf", sep=""), sep='/'))
    ##             grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], .plots[[18]], ncol=2)
    ##             dev.off()
    ##             pdf(paste(.da.wd,
    ##                       paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
    ##                             "_mr", as.character(.mr.range[.j]), 'M',
    ##                             "-queue.pdf", sep=""), sep='/'))
    ##             grid.arrange(.plots[[19]], .plots[[20]])
    ##             dev.off()
    ##         }   # end of if()
    ##     }   # end of for(.j)
    ## }   # end of for(.i)
}   # end of if()
#################################################################################
### summary plots for shared architecture with HTTP, FTP, and video traffic
### and traffic shaping
###
### Note: This is for the IEEE Transactions on Networking paper.
#################################################################################
.resp <- readline("Process data from shared access with traffic shaping? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .sa_tbf.wd <- paste(.base.directory, "results/Shared", .config, sep="/")
    .sa_tbf.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- length(list.files(path=.sa_tbf.wd, pattern='*.sca$'))  # total number of (scalar) files to process
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
                                        paste(.config, "-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'module', 'N', 'dr', 'ur', 'mr', 'bs', 'n')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic, and their fairness indexes
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+N+dr+ur+mr+bs+n+name ~ .",
                                        '.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                        'ftp')
            .tmp_ftp_fi <- calculateFairnessIndexes(.scalars,
                                                    "N+dr+ur+mr+bs+n+name ~ .",
                                                    '.*\\.ftpApp.*',
                                                    '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                    c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                                    'ftp')
            ## collect average & percentile session delays, average session throughput, mean session transfer rate of HTTP traffic, and their fairness indexes
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+N+dr+ur+mr+bs+n+name ~ .",
                                         '.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                         'http')
            .tmp_http_fi <- calculateFairnessIndexes(.scalars,
                                                     "N+dr+ur+mr+bs+n+name ~ .",
                                                     '.*\\.httpApp.*',
                                                     '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                                     c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                                     'http')
            ## collect decodable frame rate of video traffic and its fairness index
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+N+dr+ur+mr+bs+n+name ~ .",
                                          '.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                          'video')
            .tmp_video_fi <- calculateFairnessIndexes(.scalars,
                                                      "N+dr+ur+mr+bs+n+name ~ .",
                                                      '.*\\.videoApp.*',
                                                      'decodable frame rate',
                                                      c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
                                                      'video')
            ## ## collect average & percentile packet delays from DelayMeter module
            ## .tmp_packet <- collectMeasures(.scalars,
            ##                                "experiment+measurement+N+dr+ur+mr+bs+n+name ~ .",
            ##                                '.*\\.delayMeter.*',
            ##                                '(average packet delay|90th-packetDelay:percentile|95th-packetDelay:percentile|99th-packetDelay:percentile)',
            ##                                c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
            ##                                'packet')
            ## ## collect number of packets received, dropped and shaped by per-VLAN queues at OLT
            ## .tmp_queue <- collectMeasures(.scalars,
            ##                               "experiment+measurement+N+dr+ur+mr+bs+n+name ~ .",
            ##                               '.*\\.olt.*',
            ##                               '(overall packet loss rate of per-VLAN queues|overall packet shaped rate of per-VLAN queues)',
            ##                               c('N', 'dr', 'ur', 'mr', 'bs', 'n'),
            ##                               'queue')
            ## combine data frames into one
            ## .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video, .tmp_packet, .tmp_queue)
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
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
        assign(.df.name, sort_df(.tmp.df, vars=c('N', 'dr', 'ur', 'mr', 'bs', 'n')))
        assign(.fi_df.name, sort_df(.tmp_fi.df, vars=c('N', 'dr', 'ur', 'mr', 'bs', 'n')))
        save(list=c(.scalars_df.name, .df.name, .fi_df.name), file=paste(.sa_tbf.wd, .sa_tbf.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.sa_tbf.wd, .sa_tbf.rdata, sep='/'))
        .df.name <- paste('.sa_tbf_', .config, '.df', sep='')
        .fi_df.name <- paste('.sa_tbf_fi_', .config, '.df', sep='')
    }   # end of if() for the processing of OMNeT++ data files
    .sa_tbf.df <- get(.df.name)
    .sa_tbf_fi.df <- get(.fi_df.name)
    ## ##
    ## ## prefilerting for proper data sets
    ## ##
    ## .max_n = 4  # select it for S_1,6
    ## .sa_tbf.df <- subset(.sa_tbf.df, (mr==10 & bs==100 & n==.max_n) | (mr!=10 | bs!=100))
    ## .sa_tbf_fi.df <- subset(.sa_tbf_fi.df, (mr==10 & bs==100 & n==.max_n) | (mr!=10 | bs!=100))
    ## ##
    ## ## end of prefiltering
    ## ##
    .ur.range <- unique(.sa_tbf.df$ur)
    .mr.range <- unique(.sa_tbf.df$mr)
    for (.i in 1:length(.ur.range)) {
        .mr.idx <- 0
        for (.j in 1:length(.mr.range)) {
            ## process performance measures
            if (length(subset(.sa_tbf.df, ur==.ur.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                ## for (.k in 1:length(.measure.type)) {
                for (.k in 1:(length(.measure.type)-7)) { # subtract 7 because we do not include 'packet' and 'queue' in the processing
                    .df <- subset(.sa_tbf.df, ur==.ur.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(1, 5, 8, 9))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    ## .p <- ggplot(data=.df, aes(group=bs, colour=factor(bs), x=N, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- ggplot(data=.df, aes(group=bs, linetype=factor(bs), x=N, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab("Number of Subscribers (N)") + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=N, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    #.p <- .p + stat_summary(fun.data=mean_cl_boot, geom='errorbar', width=0.1) + scale_linetype_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    if (.ur.range[.i] == 1) {
                        .ur.unit <- 'G'
                    }
                    else if (.ur.range[.i] == 100) {
                        .ur.unit <- 'M'
                    }
                    else {
                        stop("Unknown value of 'ur'.")
                    }
                    ggsave(paste(.sa_tbf.wd,
                                 paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                       "_mr", as.character(.mr.range[.j]), 'M',
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.sa_tbf.wd,
                          paste(.config,
                                "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
                ## pdf(paste(.sa_tbf.wd,
                ##           paste(.config,
                ##                 "_ur", as.character(.ur.range[.i]), .ur.unit,
                ##                 "_mr", as.character(.mr.range[.j]), 'M',
                ##                 "-packet.pdf", sep=""), sep='/'))
                ## grid.arrange(.plots[[14]], .plots[[15]], .plots[[16]], .plots[[17]], ncol=2)
                ## dev.off()
                ## pdf(paste(.sa_tbf.wd,
                ##           paste(.config,
                ##                 "_ur", as.character(.ur.range[.i]), .ur.unit,
                ##                 "_mr", as.character(.mr.range[.j]), 'M',
                ##                 "-queue.pdf", sep=""), sep='/'))
                ## grid.arrange(.plots[[18]], .plots[[19]])
                ## dev.off()
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
#################################################################################
### summary plots for shared architecture with unbalanced configurations for
### HTTP, FTP, and video traffic and traffic shaping
###
### Note: This is for the IEEE Transactions on Networking paper.
#################################################################################
.resp <- readline("Process data from shared access with unbalanced traffic configuration? (hit y or n) ")
if (.resp == 'y') {
    .config <- readline("Type OMNeT++ configuration name: ")
    .su_tbf.wd <- paste(.base.directory, "results/SharedUnbalanced", .config, sep="/")
    .su_tbf.rdata <- paste(.config, 'RData', sep=".")
    if (file.exists(paste(.su_tbf.wd, .su_tbf.rdata, sep='/')) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .n_totalFiles <- length(list.files(path=.su_tbf.wd, pattern='*.sca$'))  # total number of (scalar) files to process
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
                .fileNames[.j] <- paste(.su_tbf.wd,
                                        paste(.config, "-", as.character((.i-1)*.n_files+.j-1), ".sca", sep=""),
                                        sep='/')
            }
            .df <- loadDataset(.fileNames)
            .scalars <- merge(cast(.df$runattrs, runid ~ attrname, value='attrvalue',
                                   subset=attrname %in% c('experiment', 'measurement', 'module', 'N1', 'N2', 'dr', 'ur', 'mr', 'bs', 'n1', 'n2', 'nf')),
                              .df$scalars, by='runid',
                              all.x=TRUE)
            .scalars_dfs[[.i]] <- .scalars
            ## collect average session delay, average session throughput, and mean session transfer rate of FTP traffic, and their fairness indexes
            .tmp_ftp <- collectMeasures(.scalars,
                                        "experiment+measurement+N1+N2+dr+ur+mr+bs+n1+n2+nf+name ~ .",
                                        '.*\\.host1.*\\.ftpApp.*',
                                        '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                        c('N1', 'N2', 'dr', 'ur', 'mr', 'bs', 'n1', 'n2', 'nf'),
                                        'ftp')
            ## collect average & percentile session delays, average session throughput, mean session transfer rate of HTTP traffic, and their fairness indexes
            .tmp_http <- collectMeasures(.scalars,
                                         "experiment+measurement+N1+N2+dr+ur+mr+bs+n1+n2+nf+name ~ .",
                                         '.*\\.host1.*\\.httpApp.*',
                                         '(average session delay|average session throughput|mean session transfer rate|90th-sessionDelay:percentile|95th-sessionDelay:percentile|99th-sessionDelay:percentile)',
                                         c('N1', 'N2', 'dr', 'ur', 'mr', 'bs', 'n1', 'n2', 'nf'),
                                         'http')
            ## collect decodable frame rate of video traffic and its fairness index
            .tmp_video <- collectMeasures(.scalars,
                                          "experiment+measurement+N1+N2+dr+ur+mr+bs+n1+n2+nf+name ~ .",
                                          '.*\\.host1.*\\.videoApp.*',
                                          'decodable frame rate',
                                          c('N1', 'N2', 'dr', 'ur', 'mr', 'bs', 'n1', 'n2', 'nf'),
                                          'video')
            .dfs[[.i]] <- rbind(.tmp_ftp, .tmp_http, .tmp_video)
        }   # end of for(.i)
        ## combine the data frames from experiments into one
        .scalars_df <- .scalars_dfs[[1]]
        .tmp.df <- .dfs[[1]]
        for (.i in 2:.n_experiments) {
            .scalars_df <- rbind(.scalars_df, .scalars_dfs[[.i]])
            .tmp.df <- rbind(.tmp.df, .dfs[[.i]])
        }
        ## save data frames for later use
        .scalars_df.name <- paste('.su_tbf_scalars_', .config, '.df', sep='')
        .df.name <- paste('.su_tbf_', .config, '.df', sep='')
        assign(.scalars_df.name, .scalars_df)
        assign(.df.name, sort_df(.tmp.df, vars=c('N1', 'N2', 'dr', 'ur', 'mr', 'bs', 'n1', 'n2', 'nf')))
        save(list=c(.scalars_df.name, .df.name), file=paste(.su_tbf.wd, .su_tbf.rdata, sep="/"))
    }
    else {
        ## Otherwise, load objects from the saved file
        load(paste(.su_tbf.wd, .su_tbf.rdata, sep='/'))
        .df.name <- paste('.su_tbf_', .config, '.df', sep='')
    }   # end of if() for the processing of OMNeT++ data files
    .su_tbf.df <- get(.df.name)
    .ur.range <- unique(.su_tbf.df$ur)
    .mr.range <- unique(.su_tbf.df$mr)
    for (.i in 1:length(.ur.range)) {
        .mr.idx <- 0
        for (.j in 1:length(.mr.range)) {
            ## process performance measures
            if (length(subset(.su_tbf.df, ur==.ur.range[.i] & mr==.mr.range[.j])$mean) > 0) {
                .plots <- list()
                ## for (.k in 1:length(.measure.type)) {
                for (.k in 1:(length(.measure.type)-7)) { # subtract 7 because we do not include 'packet' and 'queue' in the processing
                    .df <- subset(.su_tbf.df, ur==.ur.range[.i] & mr==.mr.range[.j] & name==.measure[.k] & measure.type==.measure.type[.k], select=c(6, 9, 11, 12))
                    is.na(.df) <- is.na(.df) # remove NaNs
                    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
                    .limits <- aes(ymin = mean - ci.width, ymax = mean +ci.width)
                    .p <- ggplot(data=.df, aes(group=bs, linetype=factor(bs), x=nf, y=mean)) + geom_line() + scale_y_continuous(limits=c(0, 1.1*max(.df$mean+.df$ci.width)))
                    .p <- .p + xlab(expression(paste("Number of FTP streams (", italic(n[f]), ")"))) + ylab(.labels.measure[.k])
                    .p <- .p + geom_point(aes(group=bs, shape=factor(bs), x=nf, y=mean), size=.pt_size) + scale_shape_manual("Burst Size\n[MB]", values=0:9)
                    .p <- .p + geom_errorbar(.limits, width=0.1) + scale_colour_discrete("Burst Size\n[MB]")
                    #.p <- .p + stat_summary(fun.data=mean_cl_boot, geom='errorbar', width=0.1) + scale_linetype_discrete("Burst Size\n[MB]")
                    .plots[[.k]] <- .p
                    ## save each plot as a PDF file
                    .p
                    if (.ur.range[.i] == 1) {
                        .ur.unit <- 'G'
                    }
                    else if (.ur.range[.i] == 100) {
                        .ur.unit <- 'M'
                    }
                    else {
                        stop("Unknown value of 'ur'.")
                    }
                    ggsave(paste(.su_tbf.wd,
                                 paste(.config, "_ur", as.character(.ur.range[.i]), .ur.unit,
                                       "_mr", as.character(.mr.range[.j]), 'M',
                                       "-", .measure.type[.k],
                                       "-", .measure.abbrv[.k], ".pdf", sep=""), sep="/"))
                }   # end of for(.k)
                ## save combined plots into a PDF file per measure.type (e.g., ftp, http)
                pdf(paste(.su_tbf.wd,
                          paste(.config,
                                "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-ftp.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[1]], .plots[[2]], .plots[[3]], .plots[[4]], .plots[[5]], .plots[[6]])
                dev.off()
                pdf(paste(.su_tbf.wd,
                          paste(.config,
                                "_ur", as.character(.ur.range[.i]), .ur.unit,
                                "_mr", as.character(.mr.range[.j]), 'M',
                                "-http.pdf", sep=""), sep='/'))
                grid.arrange(.plots[[7]], .plots[[8]], .plots[[9]], .plots[[10]], .plots[[11]], .plots[[12]], ncol=2)
                dev.off()
            }   # end of if()
        }   # end of for(.j)
    }   # end of for(.i)
}   # end of if()
