//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 Opensim Ltd
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
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include <vector>
}}

namespace inet;

cplusplus {{
typedef std::vector<IPv4Address> IPv4AddressVector;
}}

class noncobject IPv4Address;
class noncobject IPv4AddressVector;

enum IGMPType
{
    IGMP_MEMBERSHIP_QUERY = 0x11;
    IGMPV1_MEMBERSHIP_REPORT = 0x12;
    IGMPV2_MEMBERSHIP_REPORT = 0x16;
    IGMPV2_LEAVE_GROUP = 0x17;
    IGMPV3_MEMBERSHIP_REPORT = 0x22;
}

packet IGMPMessage
{
    int type @enum(IGMPType);
}

packet IGMPQuery extends IGMPMessage
{
    IPv4Address groupAddress;
}

packet IGMPv1Query extends IGMPQuery
{
    type = IGMP_MEMBERSHIP_QUERY;
}

packet IGMPv1Report extends IGMPMessage
{
    type = IGMPV1_MEMBERSHIP_REPORT;
    IPv4Address groupAddress;
}

packet IGMPv2Query extends IGMPQuery
{
    @fieldNameSuffix("_var"); //TODO remove when OMNeT++ 4.x compatibility is no longer required
    type = IGMP_MEMBERSHIP_QUERY;
    unsigned char maxRespTime;
}

packet IGMPv2Report extends IGMPMessage
{
    type = IGMPV2_MEMBERSHIP_REPORT;
    IPv4Address groupAddress;
}

packet IGMPv2Leave extends IGMPMessage
{
    type = IGMPV2_LEAVE_GROUP;
    IPv4Address groupAddress;
}

//
// IGMPv3Query extends IGMPv2Query, because
// IGMPv2 routers must accept IGMPv3Query packets.
//
// Note: in IGMPv3Query the maxResponseTime field is
// renamed to maxResponseCode and it is interpreted
// differently than in IGMPv2.
packet IGMPv3Query extends IGMPv2Query
{
    @customize(true);
    @fieldNameSuffix("_var"); //TODO remove when OMNeT++ 4.x compatibility is no longer required
    bool suppressRouterProc;
    unsigned char robustnessVariable;
    unsigned char queryIntervalCode;
    IPv4AddressVector sourceList;
    abstract unsigned char maxRespCode;
}

enum GroupRecordType
{
    MODE_IS_INCLUDE = 1;
    MODE_IS_EXCLUDE = 2;
    CHANGE_TO_INCLUDE_MODE = 3;
    CHANGE_TO_EXCLUDE_MODE = 4;
    ALLOW_NEW_SOURCES = 5;
    BLOCK_OLD_SOURCE = 6;
}

struct GroupRecord
{
    int recordType enum(GroupRecordType);
    IPv4Address groupAddress;
    IPv4AddressVector sourceList;
};

packet IGMPv3Report extends IGMPMessage
{
    type = IGMPV3_MEMBERSHIP_REPORT;
    GroupRecord groupRecord[];
}
