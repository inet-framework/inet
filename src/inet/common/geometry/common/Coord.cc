//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/Coord.h"

#include <assert.h>

namespace inet {

const Coord Coord::NIL = Coord(NaN, NaN, NaN);
const Coord Coord::ZERO = Coord(0.0, 0.0, 0.0);
const Coord Coord::ONE = Coord(1.0, 1.0, 1.0);
const Coord Coord::X_AXIS = Coord(1.0, 0.0, 0.0);
const Coord Coord::Y_AXIS = Coord(0.0, 1.0, 0.0);
const Coord Coord::Z_AXIS = Coord(0.0, 0.0, 1.0);

/**
 * On a torus the end and the begin of the axes are connected so you
 * get a circle. On a circle the distance between two points can't be greater
 * than half of the circumference.
 * If the normal distance between two points on one axis is bigger than
 * half of the size there must be a "shorter way" over the border on this axis
 */
static double dist(double coord1, double coord2, double size)
{
    double difference = fabs(coord1 - coord2);
    if (difference == 0)
        // NOTE: event if size is zero
        return 0;
    else {
        assert(size != 0);
        double dist = math::modulo(difference, size);
        return math::minnan(dist, size - dist);
    }
}

double Coord::sqrTorusDist(const Coord& b, const Coord& size) const
{
    double xDist = dist(x, b.x, size.x);
    double yDist = dist(y, b.y, size.y);
    double zDist = dist(z, b.z, size.z);
    return xDist * xDist + yDist * yDist + zDist * zDist;
}

} // namespace inet

