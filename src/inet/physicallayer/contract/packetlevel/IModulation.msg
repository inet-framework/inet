//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/physicallayer/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/contract/packetlevel/IApskModulation.h"
}}

namespace inet::physicallayer;

class IModulation
{
    @existingClass;
    @descriptor(readonly);
};

class IApskModulation
{
    @existingClass;
    @descriptor(readonly);
    int codeWordSize;
    int constellationSize;
};