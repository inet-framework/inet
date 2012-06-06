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

dataset <- loadDataset("results/Exp3*-*.sca")

configs <- with(subset(dataset$runattrs, attrname=="configname"),
                data.frame(runid=runid,
                           config=as.character(attrvalue),
                           stringsAsFactors=FALSE))

iaTime <- cast(dataset$scalars, runid~name, value='value', subset=name=='iaTime')
iaTime <- transform(iaTime, load=4000/iaTime)
runattrs <- merge(configs, iaTime)

scalars <- merge(dataset$scalars, runattrs, by="runid")

moduleToSLA <- function(module) {
  if (grepl("\\.H1\\..*udpApp\\[0\\]", module) || grepl("\\.H5\\..*udpApp\\[0\\]", module))
    "SLA1 voice"
  else if (grepl("\\.H1\\..*udpApp\\[1\\]", module) || grepl("\\.H5\\..*udpApp\\[1\\]", module))
    "SLA1 video"
  else if (grepl("\\.H1\\..*udpApp\\[2\\]", module) || grepl("\\.H5\\..*udpApp\\[2\\]", module))
    "SLA1 CBR"
  else if (grepl("\\.H3\\..*udpApp\\[0\\]", module) || grepl("\\.H7\\..*udpApp\\[0\\]", module))
    "SLA3 voice"
  else if (grepl("\\.H3\\..*udpApp\\[1\\]", module) || grepl("\\.H7\\..*udpApp\\[1\\]", module))
    "SLA3 video"
  else
    NA_character_
}

scalars <- transform(scalars, SLA=sapply(module, moduleToSLA))

pkLoss = cast(scalars, runid+config+load+SLA~name, value='value',
                    subset= ((grepl('H1.*udpApp\\[[012]\\]', module) & name=='sentPk:count') |
                             (grepl('H3.*udpApp\\[[01]\\]', module) & name=='sentPk:count') |
                             (grepl('H5.*udpApp\\[[012]\\]', module) & name=='rcvdPk:count') |
                             (grepl('H7.*udpApp\\[[01]\\]', module) & name=='rcvdPk:count')))
pkLoss <- transform(pkLoss, loss=(`sentPk:count`-`rcvdPk:count`)/`sentPk:count`)
pkLoss <- cast(pkLoss, config+load+SLA~., mean, value='loss')
names(pkLoss) <- c('config','load','SLA','loss')

pkLoss.plot <- ggplot(pkLoss, aes(x=load, y=loss, colour=SLA)) +
                 geom_line(size=1) +
                 facet_wrap(~config) +
                 opts(title='Comparison of DiffServ and Best Effort')

plotAll <- function() {
  print(pkLoss.plot)
}