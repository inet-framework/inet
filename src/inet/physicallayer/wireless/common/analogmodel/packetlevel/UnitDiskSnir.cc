//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskSnir.h"

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskNoise.h"


namespace inet {

namespace physicallayer {

UnitDiskSnir::UnitDiskSnir(const IReception *reception, const INoise *noise) :
    SnirBase(reception, noise)
{
    auto unitDiskReception = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getNewAnalogModel());
    auto power = unitDiskReception->getPower();
    auto unitDiskNoise = check_and_cast<const UnitDiskNoise *>(noise);
    auto isInterfering = unitDiskNoise->isInterfering();

    isSnirInfinite = (power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE)
                          && !isInterfering;
}

std::ostream& UnitDiskSnir::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskSnir";
    return stream;
}

} // namespace physicallayer

} // namespace inet

