//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskSnir.h"

namespace inet {

namespace physicallayer {

UnitDiskSnir::UnitDiskSnir(const IReception *reception, const INoise *noise) :
    SnirBase(reception, noise)
{
}

std::ostream& UnitDiskSnir::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskSnir";
    return stream;
}

double UnitDiskSnir::getMin() const
{
    return NaN;
}

double UnitDiskSnir::getMax() const
{
    return NaN;
}

double UnitDiskSnir::getMean() const
{
    return NaN;
}

} // namespace physicallayer

} // namespace inet

