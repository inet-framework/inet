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

#ifndef __INET_QUEUEVISUALIZERBASE_H
#define __INET_QUEUEVISUALIZERBASE_H

#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/QueueFilter.h"

namespace inet {

namespace visualizer {

class INET_API QueueVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API QueueVisitor : public cVisitor
    {
        public:
            std::vector<queueing::IPacketQueue *> queues;

        public:
            virtual void visit(cObject *object) override;

    };

    class INET_API QueueVisualization
    {
      public:
        queueing::IPacketQueue *queue = nullptr;

      public:
        QueueVisualization(queueing::IPacketQueue *queue);
        virtual ~QueueVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayQueues = false;
    QueueFilter queueFilter;
    cFigure::Color color;
    double width;
    double height;
    double spacing;
    Placement placementHint;
    double placementPriority;
    //@}

    std::vector<const QueueVisualization *> queueVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const = 0;
    virtual void addQueueVisualization(const QueueVisualization *queueVisualization);
    virtual void addQueueVisualizations();
    virtual void removeQueueVisualization(const QueueVisualization *queueVisualization);
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const = 0;
    virtual void removeAllQueueVisualizations();
};

} // namespace visualizer

} // namespace inet

#endif

