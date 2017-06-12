//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_PANELFIGURE_H
#define __INET_PANELFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

// NOTE: this cPanelFigure is a copy of the one that will be present in the OMNeT++ 5.2 release
#if OMNETPP_BUILDNUM < 1011

/**
 * @brief Sets up an axis-aligned, unscaled coordinate system for children, canceling the
 * effect of any transformation (scaling, rotation, etc.) inherited from ancestor figures.
 *
 * This allows pixel-based positioning of children, and makes them immune to zooming.
 * The anchorPoint in the coordinate system is mapped to the position of the panel figure.
 * Setting a transformation on the panel figure itself allows rotation, scaling,
 * and skewing of the coordinate system. The anchorPoint is affected by this transformation.
 *
 * The panel figure itself has no visual appearance.
 *
 * @ingroup Canvas
 */
class SIM_API cPanelFigure : public cFigure
{
    private:
        Point position;
        Point anchorPoint;
    protected:
        virtual const char **getAllowedPropertyKeys() const override;
        virtual void parse(cProperty *property) override;
    private:
        void copy(const cPanelFigure& other);
    public:
        /** @name Constructors, destructor, assignment. */
        //@{
        explicit cPanelFigure(const char *name=nullptr) : cFigure(name) {}
        cPanelFigure(const cPanelFigure& other) : cFigure(other) {copy(other);}
        cPanelFigure& operator=(const cPanelFigure& other);
        //@}

        /** @name Redefined cObject and cFigure member functions. */
        //@{
        virtual cPanelFigure *dup() const override  {return new cPanelFigure(*this);}
        virtual std::string str() const override;
        virtual const char *getRendererClassName() const override {return "GroupFigureRenderer";} // just so it will have a QGraphicsLayer (avoids a crash)
        virtual void updateParentTransform(Transform& transform) override;
        virtual void move(double dx, double dy) override { moveLocal(dx, dy); }
        virtual void moveLocal(double dx, double dy) override {position.x += dx; position.y += dy; fireTransformChange();}
        //@}

        /** @name Figure attributes */
        //@{
        virtual const Point& getPosition() const  {return position;}
        virtual void setPosition(const Point& position)  {this->position = position; fireTransformChange();}

        /**
         * By default, the (0,0) point in cPanelFigure's coordinate system will be mapped
         * to the given position (i.e. getPosition()) in the parent figure's coordinate system.
         * By setting an anchorPoint, one can change (0,0) to an arbitrary point. E.g. by setting
         * anchorPoint=(100,0), the (100,0) point will be mapped to the given position, i.e.
         * panel contents will appear 100 pixels to the left (given there are no transforms set).
         * The translation part of the local transform is cancelled out because the anchorPoint
         * is subject to the transformation of the panel figure the same way as the child figures.
         */
        virtual const Point& getAnchorPoint() const  {return anchorPoint;}
        virtual void setAnchorPoint(const Point& anchorPoint)  {this->anchorPoint = anchorPoint; fireTransformChange();}

        //@}
};

#endif

} // namespace inet

#endif // ifndef __INET_PANELFIGURE_H

