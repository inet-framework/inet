//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/obstacleloss/IdealObstacleLoss.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

namespace physicallayer {

using namespace inet::physicalenvironment;

Define_Module(IdealObstacleLoss);

IdealObstacleLoss::IdealObstacleLoss()
{
}

void IdealObstacleLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        medium = check_and_cast<IRadioMedium *>(getParentModule());
        physicalEnvironment.reference(this, "physicalEnvironmentModule", true);
    }
}

std::ostream& IdealObstacleLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "IdealObstacleLoss";
}

bool IdealObstacleLoss::isObstacle(const IPhysicalObject *object, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    const ShapeBase *shape = object->getShape();
    const Coord& position = object->getPosition();
    const Quaternion& orientation = object->getOrientation();
    RotationMatrix rotation(orientation.toEulerAngles());
    const LineSegment lineSegment(rotation.rotateVectorInverse(transmissionPosition - position), rotation.rotateVectorInverse(receptionPosition - position));
    Coord intersection1, intersection2, normal1, normal2;
    bool hasIntersections = shape->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2);
    bool isObstacle = hasIntersections && intersection1 != intersection2;
    if (isObstacle) {
        ObstaclePenetratedEvent event(object, intersection1, intersection2, normal1, normal2, isObstacle ? 1 : 0);
        const_cast<IdealObstacleLoss *>(this)->emit(obstaclePenetratedSignal, &event);
    }
    return isObstacle;
}

double IdealObstacleLoss::computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const
{
    TotalObstacleLossComputation obstacleLossVisitor(this, transmissionPosition, receptionPosition);
    physicalEnvironment->visitObjects(&obstacleLossVisitor, LineSegment(transmissionPosition, receptionPosition));
    return obstacleLossVisitor.isObstacleFound() ? 0 : 1;
}

IdealObstacleLoss::TotalObstacleLossComputation::TotalObstacleLossComputation(const IdealObstacleLoss *obstacleLoss, const Coord& transmissionPosition, const Coord& receptionPosition) :
    obstacleLoss(obstacleLoss),
    transmissionPosition(transmissionPosition),
    receptionPosition(receptionPosition)
{
}

void IdealObstacleLoss::TotalObstacleLossComputation::visit(const cObject *object) const
{
    if (!isObstacleFound_)
        isObstacleFound_ = obstacleLoss->isObstacle(check_and_cast<const IPhysicalObject *>(object), transmissionPosition, receptionPosition);
}

} // namespace physicallayer

} // namespace inet

