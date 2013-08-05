#!/usr/bin/Rscript --vanilla --default-packages="utils"
###
### R script for generating summary plots for all performance measures
### from OMNeT++ scala files for the simulation of NGN access networks
### with traffci control (i.e., shaping/policing)
###
### (C) 2013 Kyeong Soo (Joseph) Kim
###


###
### add-on packages
###
library(ggplot2)
library(gridExtra)
library(omnetpp)
library(reshape)
###
### directories
###
ifelse (Sys.getenv("OS") == "Windows_NT",
        .root.directory <- "E:/tools/omnetpp/inet-hnrl",
        .root.directory <- "~/inet-hnrl")
.base.directory <- paste(.root.directory, 'examples/ngoa/isp_traffic_control', sep='/')
.script.directory <- paste(.root.directory, 'etc/inet-hnrl', sep='/')
.mixed.wd <- paste(.base.directory, "results/mixed", sep="/")
###
### external source scripts
###
source(paste(.script.directory, 'calculateFairnessIndexes.R', sep='/'))
source(paste(.script.directory, 'collectMeasures.R', sep='/'))
source(paste(.script.directory, 'collectMeasuresAndFIs.R', sep='/'))
source(paste(.script.directory, 'confidenceInterval.R', sep='/'))
###
### functions
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
### customize
###
.old <- theme_set(theme_bw())
.pt_size <- 3.5
#################################################################################
### create data frame from simulation results mixed configurations
###
### Note: This is for the IEEE Communications Letters paper.
#################################################################################
.resp <- readline("Process data from mixed configurations? (hit y or n) ")
if (.resp == "y") {
    .config <- readline("Type OMNeT++ configuration name: ")
    .mixed.rdata <- paste(.config, "RData", sep=".")
    if (file.exists(paste(.mixed.wd, .mixed.rdata, sep="/")) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .df <- loadDataset(paste(.mixed.wd, paste(.config, "-0.vec", sep=""), sep="/"))
        .v <- loadVectors(.df, NULL)
        .vv <- .v$vectors
        .vd <- .v$vectordata
        .rk <- subset(.vv, name=="thruput (bit/sec)")$resultkey
        .df <- subset(.vd, is.element(resultkey, .rk))  # select throughput in b/s only
        .df$resultkey <- as.factor(.df$resultkey)       # for later processing (including plotting)
        ## save data frames for later use
        .df.name <- paste(.config, ".df", sep="")
        assign(.df.name, .df)
        save(list=c(.df.name), file=paste(.mixed.wd, .mixed.rdata, sep="/"))
    } else {
        ## Otherwise, load objects from the saved file
        load(paste(.mixed.wd, .mixed.rdata, sep="/"))
        .df.name <- paste(.config, ".df", sep="")
    }   # end of if() for the processing of OMNeT++ data files
    .df <- get(.df.name)
    .df$y <- .df$y / 1.0e6  # divide by 1.0e6 for unit conversion (i.e., b/s -> Mb/s)
    is.na(.df) <- is.na(.df) # remove NaNs
    .df <- .df[!is.infinite(.df$y),] # remove Infs
#################################################################################
### time series plots for groups
#################################################################################
    .vs <- split(.df, .df$resultkey) # split into separate vectors
    ## get group throughputs from flow ones
    .time <- .vs[[2]]$x                 # note that length of 1st flow time series is longer than others
    .idx <- 1:length(.time)
    .dfs <- list()
    for (.i in 0:3) {
        .thr <- .vs[[.i*4+1]]$y[.idx] + .vs[[.i*4+2]]$y[.idx] + .vs[[.i*4+3]]$y[.idx] + .vs[[.i*4+4]]$y[.idx]
        .dfs[[.i+1]] <- data.frame(group=rep(.i+1, length(.idx)), x=.time, y=.thr)
    }
    .df_line <- .dfs[[1]]
    for (.i in 2:4) {
        .df_line <- rbind(.df_line, .dfs[[.i]])
    }
    .df_line$group <- as.factor(.df_line$group)
    ## plot
    .p <- ggplot(.df_line, aes(x=x, y=y, group=group, linetype=group)) +
        geom_line(size=0.5) +
        ## geom_point() +
        scale_x_continuous("Time [s]", breaks=seq(0, 80, 10)) +
        scale_y_continuous("Throughput [Mb/s]", limits=c(0, 65), breaks=seq(0, 60, 10)) +
        scale_linetype_manual(name="Config.", values=c('solid', 'longdash', 'twodash', 'dotted'),
                        labels=c('Group 1', 'Group 2', 'Group 3', 'Group 4')) +
        theme(legend.position=c(0.9, 0.85),
              legend.key=element_blank(),
              legend.title=element_blank(),
              legend.text=element_text(size=14),
              legend.key.size=unit(1.5, "lines"),
              panel.grid.major=element_blank(),
              panel.grid.minor=element_blank())
    ## save each plot as a PDF file
    .p
    ggsave(paste(.mixed.wd, paste(paste(.config, "-dynamic", sep=""), "pdf", sep="."), sep="/"))
    dev.off()
#################################################################################
### bar charts for individual flows
#################################################################################
    ## ## .plots <- list()
    ## ## for (.i in 1:(length(.measure.type)-2)) { # subtract 2 because there are no types for 'queue'
    ## ## .df <- subset(.da.df, name==.measure[.i] & measure.type==.measure.type[.i], select=c(1, 2, 3, 5, 6))
    ## .df <- subset(.df, select=c('id', 'mean', 'ci.width'))
    ## .df$mean <- .df$mean / 1.0e6            # divide by 1.0e6 for unit conversion (i.e., b/s -> Mb/s)
    ## .df$ci.width <- .df$ci.width / 1.0e6    # ditto
    ## is.na(.df) <- is.na(.df) # remove NaNs
    ## .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
    ## .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
    ## .p <- ggplot(data=.df, aes(x=id, y=mean)) +
    ##     geom_bar(stat='identity', position=position_dodge(), color="black", size=.3) +
    ##     geom_errorbar(.limits, size=.4, width=.4, position=position_dodge(.9)) +
    ##     xlab("Flow Index") +
    ##     ylab("Throughput [Mb/s]")
    ## ## save each plot as a PDF file
    ## .p
    ## ggsave(paste(.mixed.wd, paste(paste(.config, "-static", sep=""), "pdf", sep="."), sep="/"))
    ## dev.off()
}   # end of if()
