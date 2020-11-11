//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

