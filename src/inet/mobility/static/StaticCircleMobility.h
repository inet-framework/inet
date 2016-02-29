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

#ifndef STATICCIRCLEMOBILITY_H_
#define STATICCIRCLEMOBILITY_H_

#include "inet/mobility/static/StationaryMobility.h"

namespace inet {

class StaticCircleMobility: public StationaryMobility {
private:
    // A 2D point.
    struct Point2D {
        // The x coordinate.
        double x;

        // The y coordinate.
        double y;
    };

    // Calculate the radius of the circle with basic triangle geometry based on the distance of the hosts.
    virtual double getRadius(int numHosts, double distance);

    // Identify the center of the circle.
    virtual Point2D identifyCenter(int numHosts, double distance);

    // Calculate the position with basic triangle geometry.
    virtual Point2D getPosition(int index, int numHosts, double distance,
            Point2D center);
protected:
    /**
     * @brief Places all hosts on a circle with radius based on the distance of the hosts.
     */
    virtual void setInitialPosition() override;
};

} /* namespace inet */

#endif /* STATICCIRCLEMOBILITY_H_ */
