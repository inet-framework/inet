#!/usr/bin/env Rscript


library(utils)
library(gplots)
library(ggplot2)


# argument processing
args <- commandArgs(TRUE)
infile <- args[1]

df <- read.csv(infile, header=TRUE)
pdf(file="test.pdf", width=10, height=10);
#plotmeans(df$http_delay ~ df$n);
p <- ggplot(df, aes(colour=repetition, y=http_delay, x=n))
p + geom_line(aes(group=repetition)) + geom_point()
dev.off();

