#
# Copyright (C) 2012 Opensim Ltd.
# Author: Tamas Borbely
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

require(omnetpp)
require(ggplot2)

dataset <- loadDataset(c("results/Exp1*-0.sca", "results/Exp1*-0.vec"))

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

moduleToSLA <- function(module) {
  if (grepl("\\.H1\\.", module) || grepl("\\.H5\\.", module))
    "SLA1"
  else if (grepl("\\.H2\\.", module) || grepl("\\.H6\\.", module))
    "SLA2"
  else if (grepl("\\.H3\\.", module) || grepl("\\.H7\\.", module))
    "SLA3"
  else if (grepl("\\.H4\\.", module) || grepl("\\.H8\\.", module))
    "SLA4"
  else
    NA_character_
}

scalars <- transform(scalars, SLA=sapply(module, moduleToSLA))
vectors <- transform(vectors, SLA=sapply(module, moduleToSLA))

computePkLoss <- function(appType) { # 0 = voice, 1 = video
  senderModules <- paste('H[1-4].*udpApp\\[',appType,'\\]', sep='')
  receiverModules <- paste('H[5-8].*udpApp\\[',appType,'\\]', sep='')
  d <- cast(scalars, config+SLA~name, value='value',
                    subset= ((grepl(senderModules, module) & name=='sentPk:count') |
                             (grepl(receiverModules, module) & name=='rcvdPk:count')))
  transform(d, loss=(`sentPk:count`-`rcvdPk:count`)/`sentPk:count`)
}

# voice loss
voicePkLoss <- computePkLoss(0)
voicePkLoss.plot <- ggplot(voicePkLoss, aes(x=config, y=loss, fill=SLA)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp 1 Loss Average (voice)')

# video loss
videoPkLoss <- computePkLoss(1)
videoPkLoss.plot <- ggplot(videoPkLoss, aes(x=config, y=loss, fill=SLA)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp 1 Loss Average (video)')

# voice delay
voiceDelay <- subset(vectors, grepl('H[5-8].*udpApp\\[0\\]', module) & name=='endToEndDelay:vector')
voiceDelay <- transform(voiceDelay, medianDelay=sapply(resultkey, medianOfVector))
voiceDelay.plot <- ggplot(voiceDelay, aes(x=config, y=medianDelay, fill=SLA)) +
                     geom_bar(position='dodge') +
                     opts(title="Exp 1 Delay Median (voice)")

# video delay
videoDelay <- subset(vectors, grepl('H[5-8].*udpApp\\[1\\]', module) & name=='endToEndDelay:vector')
videoDelay <- transform(videoDelay, medianDelay=sapply(resultkey, medianOfVector))
videoDelay.plot <- ggplot(videoDelay, aes(x=config, y=medianDelay, fill=SLA)) +
                     geom_bar(position='dodge') +
                     opts(title="Exp 1 Delay Median (video)")

#
# Part 2: displaying R2 queue statistics
#
modulesToDscps <- function(modules) {
  sub('.*\\.(.*?)x?Queue', '\\U\\1', modules, perl=TRUE)
}

queueScalars <- subset(scalars, grepl('R2.*ppp\\[2\\].*Queue', module) & config!='Exp17')
queueScalars <- transform(queueScalars, dscp=modulesToDscps(module))
queueScalars <- subset(queueScalars, dscp!='BE')

queueVectors <- subset(vectors, grepl('R2.*ppp\\[2\\].*Queue', module) & config!='Exp17')
queueVectors <- transform(queueVectors, dscp=modulesToDscps(module))
queueVectors <- subset(queueVectors, dscp!='BE')

# R2.ppp[2] queue packet loss
queuePkLoss <- cast(queueScalars, config+dscp~name, value='value',
                     subset=(name=='rcvdPk:count' | name=='dropPk:count'))
queuePkLoss <- transform(queuePkLoss, loss=`dropPk:count`/`rcvdPk:count`)
queuePkLoss.plot <- ggplot(queuePkLoss, aes(x=config, y=loss, fill=dscp)) +
                       geom_bar(position='dodge') +
                       opts(title='Exp 1 R2 Queue Packet Loss')

# R2.ppp[2] queue delay
queueDelay <- subset(queueVectors, name=='queueingTime:vector')
queueDelay <- transform(queueDelay, delay=sapply(resultkey, medianOfVector))
queueDelay.plot <- ggplot(queueDelay, aes(x=config, y=delay, fill=dscp)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp 1 R2 Queue Delay')


# R2.ppp[2] queue length
queueLength <- subset(queueScalars,
                      name=='queueLength:timeavg',
                      c('config', 'dscp', 'value'))
queueLength.plot <- ggplot(queueLength, aes(x=config, y=value, fill=dscp)) +
                     geom_bar(position='dodge') +
                     opts(title='Exp 1 R2 - Average Queue Length')

plotAll <- function() {
  plot <- function(p) {
    if (names(dev.cur())!='RStudioGD')
      dev.new()
    print(p)
  }

  plot(voicePkLoss.plot)
  plot(videoPkLoss.plot)
  plot(voiceDelay.plot)
  plot(videoDelay.plot)
  plot(queuePkLoss.plot)
  plot(queueDelay.plot)
  plot(queueLength.plot)
}