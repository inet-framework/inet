//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODECANVASVISUALIZATION_H
#define __INET_NETWORKNODECANVASVISUALIZATION_H

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/util/Placement.h"

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

    virtual int getNumAnnotations() const { return annotations.size(); }
    virtual void addAnnotation(cFigure *figure, cFigure::Point size, Placement placement = PLACEMENT_ANY, double placementPriority = 0);
    virtual void removeAnnotation(cFigure *figure);
    virtual void removeAnnotation(int index);
    virtual void setAnnotationSize(cFigure *figure, cFigure::Point size);
    virtual void setAnnotationVisible(cFigure *figure, bool visible);
};

} // namespace visualizer

} // namespace inet

#endif

