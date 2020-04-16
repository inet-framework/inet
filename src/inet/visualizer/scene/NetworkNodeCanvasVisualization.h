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

#ifndef __INET_NETWORKNODECANVASVISUALIZATION_H
#define __INET_NETWORKNODECANVASVISUALIZATION_H

#include "inet/common/INETDefs.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeCanvasVisualization : public NetworkNodeVisualizerBase::NetworkNodeVisualization, public cGroupFigure
{
  protected:
    class INET_API Annotation {
      public:
        cFigure *figure;
        cFigure::Rectangle bounds;
        Placement placementHint;
        double placementPriority;

      public:
        Annotation(cFigure *figure, const cFigure::Point& size, Placement placement, double placementPriority);

        static bool comparePlacementPriority(const Annotation& a1, const Annotation& a2);
    };

  protected:
    double annotationSpacing = NaN;
    double placementPenalty = NaN;

    bool isLayoutInvalid = false;
    cFigure::Rectangle submoduleBounds;
    std::vector<Annotation> annotations;
    cPanelFigure *annotationFigure = nullptr;

  protected:
    virtual void layout();

  public:
    NetworkNodeCanvasVisualization(cModule *networkNode, double annotationSpacing, double placementPenalty);

    virtual void refreshDisplay() override;

    virtual void addAnnotation(cFigure *figure, cFigure::Point size, Placement placement = PLACEMENT_ANY, double placementPriority = 0);
    virtual void removeAnnotation(cFigure *figure);
    virtual void setAnnotationSize(cFigure *figure, cFigure::Point size);
    virtual void setAnnotationVisible(cFigure *figure, bool visible);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_NETWORKNODECANVASVISUALIZATION_H

