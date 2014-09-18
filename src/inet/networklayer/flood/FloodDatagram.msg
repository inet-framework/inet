//
// Copyright (C) 2012 Andras Varga
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

cplusplus {{
#include "inet/networklayer/common/NetworkDatagramBase_m.h"
}}

namespace inet;

class NetworkDatagramBase;

packet FloodDatagram extends NetworkDatagramBase
{
    @customize(true);

    int           ttl = 1;    // time to live field
    unsigned long seqNum = 0; // sequence number
    int           transportProtocol = -1;
}
