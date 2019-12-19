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

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/physicallayer/obstacleloss/DielectricObstacleLoss.h"

namespace inet {

namespace physicallayer {

using namespace inet::physicalenvironment;

Define_Module(DielectricObstacleLoss);

DielectricObstacleLoss::DielectricObstacleLoss() :
    medium(nullptr),
    physicalEnvironment(nullptr),
    intersectionComputationCount(0),
    intersectionCount(0)
{
}

void DielectricObstacleLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        enableDielectricLoss = par("enableDielectricLoss");
        enableReflectionLoss = par("enableReflectionLoss");
        medium = check_and_cast<IRadioMedium *>(getParentModule());
        physicalEnvironment = getModuleFromPar<IPhysicalEnvironment>(par("physicalEnvironmentModule"), this);
    }
}

void DielectricObstacleLoss::finish()
{
    EV_INFO << "Obstacle loss intersection computation count: " << intersectionComputationCount << endl;
    EV_INFO << "Obstacle loss intersection count: " << intersectionCount << endl;
    recordScalar("Obstacle loss intersection computation count", intersectionComputationCount);
    recordScalar("Obstacle loss intersection count", intersectionCount);
}

std::ostream& DielectricObstacleLoss::printToStream(std::ostream& stream, int level) const
{
    return stream << "DielectricObstacleLoss";
}

double DielectricObstacleLoss::computeDielectricLoss(const IMaterial *material, Hz frequency, m distance) const
{
    // NOTE: based on http://en.wikipedia.org/wiki/Dielectric_loss
    double lossTangent = material->getDielectricLossTangent(frequency);
    mps propagationSpeed = material->getPropagationSpeed();
    double factor = std::exp(-atan(lossTangent) * unit(2 * M_PI * frequency * distance / propagationSpeed).get());
    ASSERT(0 <= factor && factor <= 1);
    return factor;
}

double DielectricObstacleLoss::computeReflectionLoss(const IMaterial *incidentMaterial, const IMaterial *refractiveMaterial, double angle) const
{
    // NOTE: based on http://en.wikipedia.org/wiki/Fresnel_equations
    double n1 = incidentMaterial->getRefractiveIndex();
    double n2 = refractiveMaterial->getRefractiveIndex();
    double st = sin(angle);
    double ct = cos(angle);
    double n1ct = n1 * ct;
    double n2ct = n2 * ct;
    double k = sqrt(1 - pow(n1 / n2 * st, 2));
    double n1k = n1 * k;
    double n2k = n2 * k;
    double rs = pow((n1ct - n2k) / (n1ct + n2k), 2);
    double rp = pow((n1k - n2ct) / (n1k + n2ct), 2);
    double r = (rs + rp) / 2;
    double transmittance = 1 - r;
    ASSERT(0 <= transmittance && transmittance <= 1);
    return transmittance;
}

double DielectricObstacleLoss::computeObjectLoss(const IPhysicalObject *object, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    double totalLoss = 1;
    const ShapeBase *shape = object->getShape();
    const Coord& position = object->getPosition();
    const Quaternion& orientation = object->getOrientation();
    RotationMatrix rotation(orientation.toEulerAngles());
    const LineSegment lineSegment(rotation.rotateVectorInverse(transmissionPosition - position), rotation.rotateVectorInverse(receptionPosition - position));
    Coord intersection1, intersection2, normal1, normal2;
    intersectionComputationCount++;
    bool hasIntersections = shape->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2);
    if (hasIntersections && (intersection1 != intersection2))
    {
        intersectionCount++;
        const IMaterial *material = object->getMaterial();
        if (enableDielectricLoss) {
            double intersectionDistance = intersection2.distance(intersection1);
            totalLoss *= computeDielectricLoss(material, frequency, m(intersectionDistance));
        }
        if (enableReflectionLoss && !normal1.isUnspecified()) {
            double angle1 = (intersection1 - intersection2).angle(normal1);
            if (!std::isnan(angle1))
                totalLoss *= computeReflectionLoss(medium->getMaterial(), material, angle1);
        }
        // TODO: this returns NaN because n1 > n2
//        if (enableReflectionLoss && !normal2.isUnspecified()) {
//            double angle2 = (intersection2 - intersection1).angle(normal2);
//            if (!std::isnan(angle2))
//                totalLoss *= computeReflectionLoss(material, medium->getMaterial(), angle2);
//        }
        ObstaclePenetratedEvent event(object, intersection1, intersection2, normal1, normal2, totalLoss);
        const_cast<DielectricObstacleLoss *>(this)->emit(obstaclePenetratedSignal, &event);
    }
    return totalLoss;
}

double DielectricObstacleLoss::computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    double totalLoss = 1;
    TotalObstacleLossComputation obstacleLossVisitor(this, frequency, transmissionPosition, receptionPosition);
    physicalEnvironment->visitObjects(&obstacleLossVisitor, LineSegment(transmissionPosition, receptionPosition));
    totalLoss = obstacleLossVisitor.getTotalLoss();
    return totalLoss;
}

DielectricObstacleLoss::TotalObstacleLossComputation::TotalObstacleLossComputation(const DielectricObstacleLoss *obstacleLoss, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) :
    totalLoss(1),
    obstacleLoss(obstacleLoss),
    frequency(frequency),
    transmissionPosition(transmissionPosition),
    receptionPosition(receptionPosition)
{
}

void DielectricObstacleLoss::TotalObstacleLossComputation::visit(const cObject *object) const
{
    totalLoss *= obstacleLoss->computeObjectLoss(check_and_cast<const IPhysicalObject *>(object), frequency, transmissionPosition, receptionPosition);
}

} // namespace physicallayer

} // namespace inet

