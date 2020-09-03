//
// Copyright (C) 2013 Opensim Ltd.
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/routing/dymo/DymoRouteData.h"

namespace inet {

namespace dymo {

DymoRouteData::DymoRouteData()
{
    isBroken = false;
    sequenceNumber = 0;
    lastUsed = 0;
    expirationTime = 0;
    metricType = HOP_COUNT;
}

std::string DymoRouteData::str() const
{
    std::ostringstream out;
    out << "isBroken = " << getBroken()
        << ", sequenceNumber = " << getSequenceNumber()
        << ", lastUsed = " << getLastUsed()
        << ", expirationTime = " << getExpirationTime()
        << ", metricType = " << getMetricType();
    return out.str();
};

} // namespace dymo
} // namespace inet

