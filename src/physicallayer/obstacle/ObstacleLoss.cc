//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "ObstacleLoss.h"
#include "PhysicalObject.h"

namespace inet {

namespace physicallayer {
Define_Module(ObstacleLoss);

ObstacleLoss::ObstacleLoss() :
    environment(NULL)
{
}

void ObstacleLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *environmentModule = par("environmentModule");
        environment = check_and_cast<PhysicalEnvironment *>(simulation.getModuleByPath(environmentModule));
    }
}

void ObstacleLoss::printToStream(std::ostream& stream) const
{
    stream << "obstacle loss";
}

double ObstacleLoss::computeDielectricLoss(const PhysicalObject *object, Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const
{
    const Material *material = object->getMaterial();
    const Coord& obstaclePosition = object->getPosition();
    const LineSegment lineSegment(transmissionPosition - obstaclePosition, receptionPosition - obstaclePosition);
    double lossTangent = material->getDielectricLossTangent(frequency);
    m distance = m(object->getShape()->computeIntersectionDistance(lineSegment));
    mps propagationSpeed = material->getPropagationSpeed();
    return std::exp(-atan(lossTangent) * unit(2 * M_PI * frequency * distance / propagationSpeed).get());
}

double ObstacleLoss::computeObstacleLoss(Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const
{
    double totalLoss = 1;
    const std::vector<PhysicalObject *>& objects = environment->getObjects();
    for (std::vector<PhysicalObject *>::const_iterator it = objects.begin(); it != objects.end(); it++) {
        const PhysicalObject *object = *it;
        const Coord& obstaclePosition = object->getPosition();
        const LineSegment lineSegment(transmissionPosition - obstaclePosition, receptionPosition - obstaclePosition);
        if (object->getShape()->isIntersecting(lineSegment))
            totalLoss *= computeDielectricLoss(object, frequency, transmissionPosition, receptionPosition);
    }
    return totalLoss;
}

} // namespace physicallayer
} // namespace inet

