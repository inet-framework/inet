#include <sstream>

#include "IPv4MulticastData.h"
#include "IGMP.h"

IPv4MulticastData::IPv4MulticastData()
{
	igmpRouterState = IGMP_RS_INITIAL;
	igmpQueryTimer = NULL;
}

IPv4MulticastData::~IPv4MulticastData()
{
	destroy(NULL);
}

std::string IPv4MulticastData::info() const
{
	std::stringstream out;
	out << "IPv4Multicast:{";
	if (!multicastGroups.empty())
	{
		out << " mcastgrps:";
		for (MulticastGroupMap::const_iterator it = multicastGroups.begin(); it != multicastGroups.end(); ++it)
		{
		    if (it != multicastGroups.begin())
		        out << ",";
			out << it->first;
		}
	}
	out << "}";
	return out.str();
}

std::string IPv4MulticastData::detailedInfo() const
{
	std::stringstream out;
	out << "Groups:";
	for (MulticastGroupMap::const_iterator it = multicastGroups.begin(); it != multicastGroups.end(); ++it)
		out << " " << it->first;
	out << "\n";
	return out.str();
}

bool IPv4MulticastData::isMemberOfMulticastGroup(const IPv4Address& multicastAddress) const
{
    MulticastGroupMap::const_iterator it = multicastGroups.find(multicastAddress);
	return (it != multicastGroups.end() && it->second->host.refCount > 0);
}

IPv4InterfaceGroupData* IPv4MulticastData::createGroup(const IPv4Address& groupAddress, bool isHost)
{
	IPv4InterfaceGroupData *group = new IPv4InterfaceGroupData();
	group->groupAddr = groupAddress;
	group->host.state = IGMP_HGS_NON_MEMBER;
	group->host.flag = false;
	group->host.timer = NULL;
	group->host.refCount = isHost ? 1 : 0;
	group->router.state = IGMP_RGS_NO_MEMBERS_PRESENT;
	group->router.timer = NULL;
	group->router.rexmtTimer = NULL;
	//group->router.v1HostTimer = NULL;
	multicastGroups.insert(std::make_pair(groupAddress, group));
	changed1();
	return group;
}

void IPv4MulticastData::deleteGroup(const IPv4Address& groupAddr, cSimpleModule* owner)
{
    MulticastGroupMap::iterator it = multicastGroups.find(groupAddr);
	if (it == multicastGroups.end()) {
		return;
	}

	IPv4InterfaceGroupData *group = it->second;
	multicastGroups.erase(it);
	changed1();

	deleteGroup(group, owner);
}

void IPv4MulticastData::deleteGroup(IPv4InterfaceGroupData* group, cSimpleModule* owner)
{
	if (group->host.timer) {
		if (owner)
			owner->cancelEvent(group->host.timer);
		delete group->host.timer;
	}

	if (group->router.timer) {
		if (owner)
			owner->cancelEvent(group->router.timer);
		delete group->router.timer;
	}

	if (group->router.rexmtTimer) {
		if (owner)
			owner->cancelEvent(group->router.rexmtTimer);
		delete group->router.rexmtTimer;
	}

	//if (group->router.v1HostTimer) {
	//	delete group->router.v1HostTimer;
	//}

	delete group;
}

void IPv4MulticastData::destroy(cSimpleModule* owner)
{
	for (MulticastGroupMap::iterator it = multicastGroups.begin(); it != multicastGroups.end(); ++it)
		deleteGroup(it->second, owner);
	multicastGroups.clear();

	if (igmpQueryTimer) {
		if (!owner)
			throw std::exception();
		
		owner->cancelAndDelete(igmpQueryTimer);
		igmpQueryTimer = NULL;
	}
}
