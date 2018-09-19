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

#ifndef __INET_SCENECANVASVISUALIZER_H
#define __INET_SCENECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneCanvasVisualizer : public SceneVisualizerBase
{
  protected:
    double zIndex = NaN;
    CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *axisLayer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void initializeAxis(double axisLength);
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshAxis(double axisLength);

    virtual RotationMatrix parseViewAngle(const char *viewAngle, bool& invertY);
    virtual cFigure::Point parse2D(const char *text, bool invertY = false);
    virtual void displayDescription(const char *descriptionFigurePath);

};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_SCENECANVASVISUALIZER_H

