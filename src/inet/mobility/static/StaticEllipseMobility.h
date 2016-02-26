//
// Copyright (C) 2016 Kai Kientopf <kai.kientopf@uni-muenster.de>
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef STATICELLIPSEMOBILITY_H_
#define STATICELLIPSEMOBILITY_H_

#include "inet/mobility/static/StationaryMobility.h"

namespace inet {

/**
 * @brief Places all hosts on a ellipse based on an given area over constraintAreaMin and constraintAreaMax.
 *
 * The construction is based on Steiner's generation of a conic.
 *
 * @attention The number of hosts must be a multiple of 4.
 * @attention constraintAreaMax.x and constraintAreaMax.y should't be infinity.
 * @note If constraintAreaMin.x or constraintAreaMin.y is infinity 0 is assumed.
 */
class StaticEllipseMobility : public StationaryMobility
{
private:
    // A 2D point.
    struct Point2D
    {
        // The x coordinate.
        double x;

        // The y coordinate.
        double y;
    };

    // A 2D line defined by two points.
    struct Line2D
    {
        // The first point.
        Point2D a;

        // The second point.
        Point2D b;
    };

    // Calculate the intersection point of two lines.
    virtual Point2D getIntersection(struct Line2D p, struct Line2D q);

    // Calculate a ellipse which fits in the given area.
    // The construction is based on Steiner's generation of a conic.
    // An area should be defined over the Parameters constraintAreaMax.x and constraintAreaMax.y unequal infinity.
    // The number of hosts must be a multiple of 4.
    virtual void setAreaPosition(int numHosts);

protected:
    /**
     * @brief Initialises the position according to the mobility model.
     */
    virtual void setInitialPosition() override;
};

} // namespace inet

#endif /* STATICELLIPSEMOBILITY_H_ */
