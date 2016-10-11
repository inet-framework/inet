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

#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/networknode/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InfoCanvasVisualizer : public InfoVisualizerBase
{
  protected:
    double zIndex = NaN;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;
    std::vector<cGroupFigure *> figures;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInfo(int i, const char *info) const override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INFOCANVASVISUALIZER_H

