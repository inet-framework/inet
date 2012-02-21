class noncobject IPv4Address;

cplusplus {{
#include "INETDefs.h"
#include "IPv4Address.h"
}}

enum IGMPType
{
	IGMP_MEMBERSHIP_QUERY = 0x11;
	IGMPV1_MEMBERSHIP_REPORT = 0x12;
	IGMPV2_MEMBERSHIP_REPORT = 0x16;
	IGMPV2_LEAVE_GROUP = 0x17;
}

packet IGMPMessage
{
	int type enum(IGMPType);
	int maxRespTime;
	short checksum;
	IPv4Address groupAddress;
}
