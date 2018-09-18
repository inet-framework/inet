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

#ifndef __INET_PHYSICALENVIRONMENTCANVASVISUALIZER_H
#define __INET_PHYSICALENVIRONMENTCANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentCanvasVisualizer : public PhysicalEnvironmentVisualizerBase
{
  protected:
    class ObjectPositionComparator
    {
      protected:
        const RotationMatrix &viewRotation;

      public:
        ObjectPositionComparator(const RotationMatrix &viewRotation) : viewRotation(viewRotation) {}

        bool operator() (const physicalenvironment::IPhysicalObject *left, const physicalenvironment::IPhysicalObject *right) const
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

    virtual void computeFacePoints(const physicalenvironment::IPhysicalObject *object, std::vector<std::vector<Coord> >& faces, const RotationMatrix& rotation) const;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENTCANVASVISUALIZER_H

