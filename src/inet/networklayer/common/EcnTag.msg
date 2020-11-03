//
// Copyright (C) 2020 OpenSim Ltd.
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

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

enum IpEcnCode {
  IP_ECN_NOT_ECT = 0;
  IP_ECN_ECT_1 = 1;
  IP_ECN_ECT_0 = 2;
  IP_ECN_CE = 3;
}
//
// This is an abstract base class that should not be directly added as a tag.
//
class EcnTagBase extends TagBase
{
    int explicitCongestionNotification = -1; // explicit congestion notification
}

//
// This request determines the explicit congestion notification that should be used to transmit the packet.
//
class EcnReq extends EcnTagBase
{
}

//
// This indication specifies the explicit congestion notification that was used to receive the packet.
//
class EcnInd extends EcnTagBase
{
}
