#
# Copyright (C) 2012 Opensim Ltd.
# Author: Tamas Borbely
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

require(omnetpp)
require(ggplot2)

dataset <- loadDataset(c("results/Exp2*-*.sca", "results/Exp2*-*.vec"))

configs <- with(subset(dataset$runattrs, attrname=="configname"),
                data.frame(runid=runid,
                           config=as.character(attrvalue),
                           stringsAsFactors=FALSE))

iaTime <- cast(dataset$scalars, runid~name, value='value', subset=name=='iaTime')
iaTime <- transform(iaTime, load=4000/iaTime)
runattrs <- merge(configs, iaTime)

scalars <- merge(dataset$scalars, runattrs, by="runid")
vectors <- merge(dataset$vectors, runattrs, by="runid")


moduleToSLA <- function(module) {
  if (grepl("\\.H1\\.", module) || grepl("\\.H5\\.", module))
    "SLA1"
  else if (grepl("\\.H2\\.", module) || grepl("\\.H6\\.", module))
    "SLA2"
  else if (grepl("\\.H3\\.", module) || grepl("\\.H7\\.", module))
    "SLA3"
  else
    NA_character_
}

scalars <- transform(scalars, SLA=sapply(module, moduleToSLA))
vectors <- transform(vectors, SLA=sapply(module, moduleToSLA))

# voice loss
voicePkLoss = cast(scalars, config+load+SLA~name, value='value',
                    subset= ((grepl('H[1-3].*udpApp\\[0\\]', module) & name=='sentPk:count') |
                             (grepl('H[5-7].*udpApp\\[0\\]', module) & name=='rcvdPk:count')))
voicePkLoss <- transform(voicePkLoss,
                         loss=(`sentPk:count`-`rcvdPk:count`)/`sentPk:count`)

pkLossPlot <- function(data, title) {
  ggplot(data, aes(x=load, y=loss, colour=SLA)) +
    geom_line() +
    scale_colour_manual(values=c('SLA1'='green','SLA2'='yellow','SLA3'='red')) +
    facet_wrap(~config) +
    opts(title=title)
}

voicePkLoss.plot <- pkLossPlot(voicePkLoss, 'Exp 2 Increasing Traffic Load (voice)')

# video loss
videoPkLoss <- cast(scalars, config+load+SLA~name, value='value',
                    subset= ((grepl('H[1-3].*udpApp\\[1\\]', module) & name=='sentPk:count') |
                             (grepl('H[5-7].*udpApp\\[1\\]', module) & name=='rcvdPk:count')))
videoPkLoss <- transform(videoPkLoss,
                         loss=(`sentPk:count`-`rcvdPk:count`)/`sentPk:count`)


videoPkLoss.plot <- pkLossPlot(videoPkLoss, 'Exp 2 Increasing Traffic Load (video)')

# voice loss per color
voicePkLossPerColor.plot <- ggplot(subset(voicePkLoss, config!='Exp24'), aes(x=load,y=loss,colour=config)) +
                              geom_line() +
                              scale_colour_manual(values=c('Exp21'='green','Exp22'='yellow','Exp23'='red')) +
                              facet_wrap(~SLA) +
                              opts(title='Comparison of green/yellow/red traffic (voice)')

# average length of R2.ppp[2] queue
queueLength <- cast(scalars, config+load~name, value='value',
                    subset= (grepl('R2\\.ppp\\[2\\]', module) & name=='queueLength:timeavg'))
queueLength.plot <- ggplot(queueLength,
                           aes(x=load, y=`queueLength:timeavg`, colour=config)) +
                      geom_line() +
                      scale_colour_manual(values=c('Exp21'='green','Exp22'='yellow','Exp23'='red')) +
                      opts(title='Exp 2 R2 Queue - Average Queue Length') +
                      ylab('Average queue length')

plotAll <- function() {
  plot <- function(p) {
    if (names(dev.cur())!='RStudioGD')
      dev.new()
    print(p)
  }

  plot(voicePkLoss.plot)
  plot(videoPkLoss.plot)
  plot(voicePkLossPerColor.plot)
  plot(queueLength.plot)
}