//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_QUEUECANVASVISUALIZER_H
#define __INET_QUEUECANVASVISUALIZER_H

#include "inet/common/figures/QueueFigure.h"
#include "inet/visualizer/base/QueueVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API QueueCanvasVisualizer : public QueueVisualizerBase
{
  protected:
    class INET_API QueueCanvasVisualization : public QueueVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        QueueFigure *figure = nullptr;

      public:
        QueueCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, QueueFigure *figure, queueing::IPacketQueue *queue);
        virtual ~QueueCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override;
    virtual void addQueueVisualization(const QueueVisualization *queueVisualization) override;
    virtual void removeQueueVisualization(const QueueVisualization *queueVisualization) override;
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override;

  public:
    virtual ~QueueCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_QUEUECANVASVISUALIZER_H

