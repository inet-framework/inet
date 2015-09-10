//
// Copyright (C) 2014 OpenSim Ltd.
// Author: Benjamin Seregi
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

#include "inet/routing/aodv/AODVRouteData.h"

namespace inet {

std::ostream& operator<<(std::ostream& out, const AODVRouteData *data)
{
    out << " isActive = " << data->isActive()
        << ", hasValidDestNum = " << data->hasValidDestNum()
        << ", destNum = " << data->getDestSeqNum()
        << ", lifetime = " << data->getLifeTime();

    const std::set<L3Address>& preList = data->getPrecursorList();

    if (!preList.empty()) {
        out << ", precursor list: ";
        std::set<L3Address>::const_iterator iter = preList.begin();
        out << *iter;
        for (++iter; iter != preList.end(); ++iter)
            out << "; " << *iter;
    }
    return out;
};

} // namespace inet

