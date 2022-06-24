//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALENVIRONMENTCANVASVISUALIZER_H
#define __INET_PHYSICALENVIRONMENTCANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentCanvasVisualizer : public PhysicalEnvironmentVisualizerBase
{
  protected:
    class INET_API ObjectPositionComparator {
      protected:
        const RotationMatrix& viewRotation;

      public:
        ObjectPositionComparator(const RotationMatrix& viewRotation) : viewRotation(viewRotation) {}

        bool operator()(const physicalenvironment::IPhysicalObject *left, const physicalenvironment::IPhysicalObject *right) const
        {
            return viewRotation.rotateVector(left->getPosition()).z < viewRotation.rotateVector(right->getPosition()).z;
        }
    };

  protected:
    double zIndex = NaN;
    /** @name Internal state */
    //@{
    const CanvasProjection *canvasProjection;
    //@}

    /** @name Graphics */
    //@{
    cGroupFigure *objectsLayer = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void computeFacePoints(const physicalenvironment::IPhysicalObject *object, std::vector<std::vector<Coord>>& faces, const RotationMatrix& rotation) const;
};

} // namespace visualizer

} // namespace inet

#endif

