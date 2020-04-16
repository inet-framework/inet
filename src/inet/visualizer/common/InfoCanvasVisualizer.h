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

#ifndef __INET_INFOCANVASVISUALIZER_H
#define __INET_INFOCANVASVISUALIZER_H

#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InfoCanvasVisualizer : public InfoVisualizerBase
{
  protected:
    class INET_API InfoCanvasVisualization : public InfoVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        BoxedLabelFigure *figure = nullptr;

      public:
        InfoCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int moduleId);
        virtual ~InfoCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override;
    virtual void addInfoVisualization(const InfoVisualization *infoVisualization) override;
    virtual void removeInfoVisualization(const InfoVisualization *infoVisualization) override;
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override;

  public:
    virtual ~InfoCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INFOCANVASVISUALIZER_H

