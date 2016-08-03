//
// Copyright (C) 2012 Opensim Ltd.
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

cplusplus {{
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
}}

namespace inet;

enum IPProtocolId;

class noncobject L3Address;

//
// Control information for sending/receiving packets over the generic network protocol.
//
class GenericNetworkProtocolControlInfo  //TODO rename!!!!
{
    @customize(true);
    short protocol @enum(IPProtocolId);  // encapsulated protocol
    short hopLimit;     // maximum hop count
}

