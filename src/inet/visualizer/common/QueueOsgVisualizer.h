//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_QUEUEOSGVISUALIZER_H
#define __INET_QUEUEOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/QueueVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API QueueOsgVisualizer : public QueueVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API QueueOsgVisualization : public QueueVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        QueueOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, queueing::IPacketQueue *queue);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override;
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override { return nullptr; }
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif

