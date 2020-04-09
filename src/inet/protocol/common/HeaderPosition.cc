//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/protocol/common/HeaderPosition.h"

namespace inet {

HeaderPosition parseHeaderPosition(const char *string)
{
    if (!strcmp(string, "") || !strcmp(string, "none"))
        return HP_NONE;
    else if (!strcmp(string, "front"))
        return  HP_FRONT;
    else if (!strcmp(string, "back"))
        return HP_BACK;
    else
        throw cRuntimeError("Unknown header position value");
}

} // namespace inet

