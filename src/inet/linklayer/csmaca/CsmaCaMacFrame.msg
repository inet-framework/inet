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

import inet.linklayer.common.MACAddress;
import inet.linklayer.contract.IMACFrame;

cplusplus {{
#include "inet/linklayer/contract/IMACFrame.h"
}}

namespace inet;

packet CsmaCaMacFrame
{
    @implements(IMACFrame);
    MACAddress transmitterAddress @overrideGetter @overrideSetter;
    MACAddress receiverAddress @overrideGetter @overrideSetter;
}

packet CsmaCaMacAckFrame extends CsmaCaMacFrame
{
}

packet CsmaCaMacDataFrame extends CsmaCaMacFrame
{
    int priority;  // 802.1d UP (User Priority) 
}
