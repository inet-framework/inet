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
.static.wd <- paste(.base.directory, "results/static", sep="/")
.dynamic.wd <- paste(.base.directory, "results/dynamic", sep="/")
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
### summary plots for static configurations
###
### Note: This is for the IEEE Communications Letters paper.
#################################################################################
.resp <- readline("Process data from static configurations? (hit y or n) ")
if (.resp == "y") {
    .config <- readline("Type OMNeT++ configuration name: ")
    .static.rdata <- paste(.config, "RData", sep=".")
    if (file.exists(paste(.static.wd, .static.rdata, sep="/")) == FALSE) {
        ## Do the processing of OMNeT++ data files unless there is a corresponding RData file in the working directory
        .df <- loadDataset(paste(.static.wd, paste(.config, "-*.sca", sep=""), sep="/"))
        .scalars <- merge(.df$scalars,
                          cast(.df$runattrs, runid ~ attrname, value="attrvalue", subset=attrname %in% c("experiment", "measurement")),
                           by="runid",
                          all.x=TRUE)
        .tmp <- subset(cast(.scalars, experiment+measurement+module+name ~ ., c(mean, ci.width)),
                       name=="avg throughput (bit/s)",
                       select=c("module", "mean", "ci.width"))
        .tmp$id <- as.numeric(sub(".*host\\[([0-9]+)\\].*", "\\1", .tmp$module)) + 1    # add numeric "id" column for sorting
        ## save data frames for later use
        .df.name <- paste(.config, ".df", sep="")
        assign(.df.name, sort_df(.tmp, vars=c("id")))
        save(list=c(.df.name), file=paste(.static.wd, .static.rdata, sep="/"))
    } else {
        ## Otherwise, load objects from the saved file
        load(paste(.static.wd, .static.rdata, sep="/"))
        .df.name <- paste(.config, ".df", sep="")
    }   # end of if() for the processing of OMNeT++ data files
    .df <- get(.df.name)
    ## .plots <- list()
    ## for (.i in 1:(length(.measure.type)-2)) { # subtract 2 because there are no types for 'queue'
    ## .df <- subset(.da.df, name==.measure[.i] & measure.type==.measure.type[.i], select=c(1, 2, 3, 5, 6))
    .df <- subset(.df, select=c('id', 'mean', 'ci.width'))
    .df$mean <- .df$mean / 1.0e6            # divide by 1.0e6 for unit conversion (i.e., b/s -> Mb/s)
    .df$ci.width <- .df$ci.width / 1.0e6    # ditto
    is.na(.df) <- is.na(.df) # remove NaNs
    .df <- .df[!is.infinite(.df$ci.width),] # remove Infs
    .limits <- aes(ymin = mean - ci.width, ymax = mean + ci.width)
    .p <- ggplot(data=.df, aes(x=id, y=mean)) +
        geom_bar(stat='identity', position=position_dodge(), color="black", size=.3) +
        geom_errorbar(.limits, size=.4, width=.4, position=position_dodge(.9)) +
        xlab("Flow Index") +
        ylab("Throughput [Mb/s]")
    ## save each plot as a PDF file
    .p
    ggsave(paste(.static.wd, paste(.config, "pdf", sep="."), sep="/"))
    dev.off()
}   # end of if()
