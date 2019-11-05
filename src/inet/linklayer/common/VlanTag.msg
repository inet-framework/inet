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

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class VlanTagBase extends TagBase
{
    int vlanId = -1;
}

//
// This request determines the vlan that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class VlanReq extends VlanTagBase
{
}

//
// This indication specifies the vlan that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class VlanInd extends VlanTagBase
{
}

