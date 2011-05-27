//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2006 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef WORLD_OBSTACLE_OBSTACLE_H
#define WORLD_OBSTACLE_OBSTACLE_H

#include <vector>
#include "Coord.h"
#include "world/annotations/AnnotationManager.h"

/**
 * stores information about an Obstacle for ObstacleControl
 */
class Obstacle {
    public:
        typedef std::vector<Coord> Coords;

        Obstacle(std::string id, double attenuationPerWall, double attenuationPerMeter);

        void setShape(Coords shape);
        const Coords& getShape() const;
        const Coord getBboxP1() const;
        const Coord getBboxP2() const;

        double calculateReceivedPower(double pSend, double carrierFrequency, const Coord& senderPos, double senderAngle, const Coord& receiverPos, double receiverAngle) const;

        AnnotationManager::Annotation* visualRepresentation;

    protected:
        std::string id;
        double attenuationPerWall; /**< in dB. Consumer Wi-Fi vs. an exterior wall will give approx. 50 dB */
        double attenuationPerMeter; /**< in dB / m. Consumer Wi-Fi vs. an interior hollow wall will give approx. 5 dB */
        Coords coords;
        Coord bboxP1;
        Coord bboxP2;
};

#endif
