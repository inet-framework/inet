#
# Copyright (C) 2012 Opensim Ltd.
# Author: Tamas Borbely
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

require(omnetpp)
require(ggplot2)

dataset <- loadDataset(c("results/Exp5*-*.sca", "results/Exp5*-*.vec"))

configs <- with(subset(dataset$runattrs, attrname=="configname"),
                data.frame(runid=runid,
                           config=as.character(attrvalue),
                           stringsAsFactors=FALSE))
scalars <- merge(dataset$scalars, configs, by="runid")
vectors <- merge(dataset$vectors, configs, by="runid")

medianOfVector <- function(vectorkey) {
  d <- loadVectors(dataset, vectorkey)
  median(d$vectordata$y)
}

averageReplications <- function(data, formula, value) {
  as.data.frame(rename(cast(data, formula, mean, value=value), c('(all)' = value)))
}

moduleToHost <- function(module) {
  if (grepl("\\.H1\\.", module) || grepl("\\.H5\\.", module))
    "Host 1"
  else if (grepl("\\.H2\\.", module) || grepl("\\.H6\\.", module))
    "Host 2"
  else if (grepl("\\.H3\\.", module) || grepl("\\.H7\\.", module))
    "Host 3"
  else if (grepl("\\.H4\\.", module) || grepl("\\.H8\\.", module))
    "Host 4"
  else
    NA_character_
}

moduleToApp <- function(module) {
  if (grepl("\\.H.\\.udpApp\\[0\\]", module))
    "voice"
  else if (grepl("\\.H.\\.udpApp\\[1\\]", module))
    "video"
  else
    NA_character_
}

scalars <- transform(scalars, host=sapply(module, moduleToHost), app=sapply(module, moduleToApp))
vectors <- transform(vectors, host=sapply(module, moduleToHost), app=sapply(module, moduleToApp))

# loss average
pkLoss <- cast(scalars, runid+config+app+host~name, value='value',
                    subset= ((grepl('H[1-4].*udpApp', module) & name=='sentPk:count') |
                             (grepl('H[5-8].*udpApp', module) & name=='rcvdPk:count')))
pkLoss <- transform(pkLoss, loss=(`sentPk:count`-`rcvdPk:count`)/`sentPk:count`, configapp=paste(config,app))
pkLoss <- averageReplications(pkLoss, configapp+host~., value='loss')
pkLoss.plot <- ggplot(pkLoss, aes(x=configapp, y=loss, fill=host)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp 5 Loss Average (voice and video)')


# delay 50th percentile
endToEndDelay <- subset(vectors, grepl('H[5-8].*udpApp\\[.\\]', module) & name=='endToEndDelay:vector')
endToEndDelay <- transform(endToEndDelay, delay=sapply(resultkey, medianOfVector), configapp=paste(config,app))
endToEndDelay <- averageReplications(endToEndDelay, configapp+host~., value='delay')
endToEndDelay.plot <- ggplot(endToEndDelay, aes(x=configapp, y=delay, fill=host)) +
                        geom_bar(position='dodge') +
                        opts(title='Exp 5 Median Delay (voice and video)')

#
# R2 queue statistics
#
modulesToDscps <- function(modules) {
  sub('.*\\.(.*?)x?Queue', '\\U\\1', modules, perl=TRUE)
}

queueScalars <- subset(scalars, grepl('R2.*ppp\\[2\\].*(af1x|af2x|ef)Queue', module) & config=='Exp51')
queueScalars <- transform(queueScalars, dscp=modulesToDscps(module))

queueVectors <- subset(vectors, grepl('R2.*ppp\\[2\\].*(af1x|af2x|ef)Queue', module) & config=='Exp51')
queueVectors <- transform(queueVectors, dscp=modulesToDscps(module))

# R2.ppp[2] queue packet loss
queuePkLoss <- cast(queueScalars, runid+config+dscp~name, value='value',
                     subset=(name=='rcvdPk:count' | name=='dropPk:count'))
queuePkLoss <- transform(queuePkLoss, loss=`dropPk:count`/`rcvdPk:count`)
queuePkLoss <- averageReplications(queuePkLoss, config+dscp~., value='loss')
queuePkLoss.plot <- ggplot(queuePkLoss, aes(x=dscp, y=loss, fill=config)) +
                       geom_bar(position='dodge') +
                       opts(title='Exp5.1 R2 Queue - Packet Loss')

# R2.ppp[2] queue delay
queueDelay <- subset(queueVectors, name=='queueingTime:vector')
queueDelay <- transform(queueDelay, delay=sapply(resultkey, medianOfVector))
queueDelay <- averageReplications(queueDelay, config+dscp~., value='delay')
queueDelay.plot <- ggplot(queueDelay, aes(x=dscp, y=delay, fill=config)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp5.1 R2 Queue - Median Queueing Delay')


# R2.ppp[2] queue length
queueLength <- subset(queueScalars,
                      name=='queueLength:timeavg',
                      c('config', 'dscp', 'value'))
queueLength <- rename(averageReplications(queueLength, config+dscp~., value='value'), c('value'='queue length'))
queueLength.plot <- ggplot(queueLength, aes(x=dscp, y=`queue length`, fill=config)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp5.1 R2 Queue - Average Queue Length')

plotAll <- function() {
  plot <- function(p) {
    if (names(dev.cur())!='RStudioGD')
      dev.new()
    print(p)
  }
  plot(pkLoss.plot)
  plot(endToEndDelay.plot)
  plot(queuePkLoss.plot)
  plot(queueDelay.plot)
  plot(queueLength.plot)
}
