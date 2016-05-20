//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/linklayer/common/MACAddress.h"
}}

class noncobject MACAddress;

namespace inet::newcsma;

enum NewCsmaFrameType
{
    ST_ACK = 0x1d;
    ST_DATA = 0x20;
}

packet NewCsmaFrame
{
    byteLength = 14;
    short type enum(NewCsmaFrameType);
    bool retry;
    MACAddress receiverAddress;
}

packet NewCsmaACKFrame extends NewCsmaFrame
{
    type = ST_ACK;
}

packet NewCsmaDataFrame extends NewCsmaFrame
{
    type = ST_DATA;
    byteLength = 34;
    MACAddress transmitterAddress;
}
