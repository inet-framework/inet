#include "IGMP.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv4MulticastData.h"
#include "NotificationBoard.h"

#include <algorithm>

Define_Module(IGMP);

// RFC 2236, Section 6: Host State Diagram
//                           ________________
//                          |                |
//                          |                |
//                          |                |
//                          |                |
//                --------->|   Non-Member   |<---------
//               |          |                |          |
//               |          |                |          |
//               |          |                |          |
//               |          |________________|          |
//               |                   |                  |
//               | leave group       | join group       | leave group
//               | (stop timer,      |(send report,     | (send leave
//               |  send leave if    | set flag,        |  if flag set)
//               |  flag set)        | start timer)     |
//       ________|________           |          ________|________
//      |                 |<---------          |                 |
//      |                 |                    |                 |
//      |                 |<-------------------|                 |
//      |                 |   query received   |                 |
//      | Delaying Member |    (start timer)   |   Idle Member   |
// ---->|                 |------------------->|                 |
//|     |                 |   report received  |                 |
//|     |                 |    (stop timer,    |                 |
//|     |                 |     clear flag)    |                 |
//|     |_________________|------------------->|_________________|
//| query received    |        timer expired
//| (reset timer if   |        (send report,
//|  Max Resp Time    |         set flag)
//|  < current timer) |
// -------------------
//
// RFC 2236, Section 7: Router State Diagram
//                                     --------------------------------
//                             _______|________  gen. query timer      |
// ---------                  |                |        expired        |
//| Initial |---------------->|                | (send general query,  |
// ---------  (send gen. q.,  |                |  set gen. q. timer)   |
//       set initial gen. q.  |                |<----------------------
//             timer)         |    Querier     |
//                            |                |
//                       -----|                |<---
//                      |     |                |    |
//                      |     |________________|    |
//query received from a |                           | other querier
//router with a lower   |                           | present timer
//IP address            |                           | expired
//(set other querier    |      ________________     | (send general
// present timer)       |     |                |    |  query,set gen.
//                      |     |                |    |  q. timer)
//                      |     |                |    |
//                       ---->|      Non       |----
//                            |    Querier     |
//                            |                |
//                            |                |
//                       ---->|                |----
//                      |     |________________|    |
//                      | query received from a     |
//                      | router with a lower IP    |
//                      | address                   |
//                      | (set other querier        |
//                      |  present timer)           |
//                       ---------------------------
//
//                              ________________
// ----------------------------|                |<-----------------------
//|                            |                |timer expired           |
//|               timer expired|                |(notify routing -,      |
//|          (notify routing -)|   No Members   |clear rxmt tmr)         |
//|                    ------->|    Present     |<-------                |
//|                   |        |                |       |                |
//|v1 report rec'd    |        |                |       |  ------------  |
//|(notify routing +, |        |________________|       | | rexmt timer| |
//| start timer,      |                    |            | |  expired   | |
//| start v1 host     |  v2 report received|            | | (send g-s  | |
//|  timer)           |  (notify routing +,|            | |  query,    | |
//|                   |        start timer)|            | |  st rxmt   | |
//|         __________|______              |       _____|_|______  tmr)| |
//|        |                 |<------------       |              |     | |
//|        |                 |                    |              |<----- |
//|        |                 | v2 report received |              |       |
//|        |                 | (start timer)      |              |       |
//|        | Members Present |<-------------------|    Checking  |       |
//|  ----->|                 | leave received     |   Membership |       |
//| |      |                 | (start timer*,     |              |       |
//| |      |                 |  start rexmt timer,|              |       |
//| |      |                 |  send g-s query)   |              |       |
//| |  --->|                 |------------------->|              |       |
//| | |    |_________________|                    |______________|       |
//| | |v2 report rec'd |  |                          |                   |
//| | |(start timer)   |  |v1 report rec'd           |v1 report rec'd    |
//| |  ----------------   |(start timer,             |(start timer,      |
//| |v1 host              | start v1 host timer)     | start v1 host     |
//| |tmr    ______________V__                        | timer)            |
//| |exp'd |                 |<----------------------                    |
//|  ------|                 |                                           |
//|        |    Version 1    |timer expired                              |
//|        | Members Present |(notify routing -)                         |
// ------->|                 |-------------------------------------------
//         |                 |<--------------------
// ------->|_________________| v1 report rec'd     |
//| v2 report rec'd |   |   (start timer,          |
//| (start timer)   |   |    start v1 host timer)  |
// -----------------     --------------------------

void IGMP::initialize(int stage)
{
	cSimpleModule::initialize(stage);

	if (stage == 0)
	{
		ift = InterfaceTableAccess().get();
		rt = RoutingTableAccess().get();
		nb = NotificationBoardAccess().get();
		
		nb->subscribe(this, NF_INTERFACE_DELETED);

		enabled = par("enabled");
		externalRouter = par("externalRouter");
		robustness = par("robustnessVariable");
		queryInterval = par("queryInterval");
		queryResponseInterval = par("queryResponseInterval");
		groupMembershipInterval = par("groupMembershipInterval");
		otherQuerierPresentInterval = par("otherQuerierPresentInterval");
		startupQueryInterval = par("startupQueryInterval");
		startupQueryCount = par("startupQueryCount");
		lastMemberQueryInterval = par("lastMemberQueryInterval");
		lastMemberQueryCount = par("lastMemberQueryCount");
		unsolicitedReportInterval = par("unsolicitedReportInterval");
		//version1RouterPresentInterval = par("version1RouterPresentInterval");

		numGroups = 0; 
		numHostGroups = 0; 
		numRouterGroups = 0; 

		numQueriesSent = 0; 
		numQueriesRecv = 0; 
		numGeneralQueriesSent = 0; 
		numGeneralQueriesRecv = 0; 
		numGroupSpecificQueriesSent = 0; 
		numGroupSpecificQueriesRecv = 0; 
		numReportsSent = 0; 
		numReportsRecv = 0; 
		numLeavesSent = 0; 
		numLeavesRecv = 0; 

		WATCH(numGroups);
		WATCH(numHostGroups);
		WATCH(numRouterGroups);

		WATCH(numQueriesSent);
		WATCH(numQueriesRecv);
		WATCH(numGeneralQueriesSent);
		WATCH(numGeneralQueriesRecv);
		WATCH(numGroupSpecificQueriesSent);
		WATCH(numGroupSpecificQueriesRecv);
		WATCH(numReportsSent);
		WATCH(numReportsRecv);
		WATCH(numLeavesSent);
		WATCH(numLeavesRecv);
	}
}

void IGMP::finish()
{
	for (std::set<IPv4MulticastData *>::iterator it = allMulticastData.begin(); it != allMulticastData.end(); ++it)
		deleteMulticastData(*it);
	allMulticastData.clear();
}

IGMP::~IGMP()
{
}

void IGMP::deleteMulticastData(IPv4MulticastData *data)
{
	data->destroy(this);
	delete data;
}

void IGMP::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();

	if (category == NF_INTERFACE_DELETED) {
		InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(details);
		IPv4MulticastData *data = ie->ipv4MulticastData();
		allMulticastData.erase(data);
		deleteMulticastData(data);
		ie->setIPv4MulticastData(NULL);
	}
}

void IGMP::configureInterface(InterfaceEntry *ie)
{
	Enter_Method("configureInterface");

	ASSERT(ie->ipv4MulticastData() == NULL);

	IPv4MulticastData *data = new IPv4MulticastData();
	allMulticastData.insert(data);
	ie->setIPv4MulticastData(data);

	if (!ie->isLoopback()) {
		// add "224.0.0.1" automatically for all interfaces
		data->createGroup(IPv4Address::ALL_HOSTS_MCAST, true);
		numGroups++;
		numHostGroups++;

		// add "224.0.0.2" only if Router (IPv4 forwarding enabled)
		if (rt->isIPForwardingEnabled()) {
			if (!externalRouter) {
				data->createGroup(IPv4Address::ALL_ROUTERS_MCAST, true);
				numGroups++;
				numHostGroups++;

				if (enabled) {
					// start querier on this interface
					IPv4InterfaceTimer *timer = new IPv4InterfaceTimer("IGMP query timer", ie);
					data->setIGMPQueryTimer(timer);
					data->setIGMPRouterState(IGMP_RS_QUERIER);
					sendQuery(ie, IPv4Address(), queryResponseInterval); // general query
					startTimer(timer, startupQueryInterval);
				}
			}
		}
	}
}

void IGMP::handleMessage(cMessage *msg)
{
	if (!enabled) {
		opp_error("IGMP: handleMessage> disabled");
		delete msg;
		return;
	}

	if (msg->isSelfMessage()) {
		if (dynamic_cast<IPv4InterfaceGroupTimer *>(msg)) {
			processGroupTimer((IPv4InterfaceGroupTimer *)msg);
		}
		else if (dynamic_cast<IPv4InterfaceTimer *>(msg)) {
			processQueryTimer((IPv4InterfaceTimer *)msg);
		}
		else {
			ASSERT(false);
		}
	}
	else if (!strcmp(msg->getArrivalGate()->getName(), "routerIn")) {
		send(msg, "ipOut");
	}
	else if (dynamic_cast<IGMPMessage *>(msg)) {
		processIgmpMessage((IGMPMessage *)msg);
	}
	else {
		ASSERT(false);
	}
}

void IGMP::joinMulticastGroup(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
	Enter_Method("joinMulticastGroup");

	if (!ie) {
		ie = getFirstNonLoopbackInterface();
	}

	if (!ie) {
		opp_error("IGMP: could not find suitable local interface for groups");
	}

	IPv4InterfaceGroupData* group = NULL;
	const IPv4MulticastData::MulticastGroupMap &multicastGroups = ie->ipv4MulticastData()->getMulticastGroups();
	IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.find(groupAddr);
	if (it != multicastGroups.end()) {
		group = it->second;
		if (group->host.refCount > 0) {
			group->host.refCount++;
			return;
		}
		group->host.refCount = 1;
	}

	if (!group) {
		group = ie->ipv4MulticastData()->createGroup(groupAddr, true);
		numGroups++;
	}

	numHostGroups++;

	if (enabled && 
		groupAddr != IPv4Address::ALL_ROUTERS_MCAST && 
		groupAddr != IPv4Address::ALL_HOSTS_MCAST) {
		sendReport(ie, group);
		group->host.flag = true;
		startHostTimer(ie, group, unsolicitedReportInterval);
		group->host.state = IGMP_HGS_DELAYING_MEMBER;
	}

	IPv4MulticastGroupInfo info;
	info.ie = ie;
	info.groupAddress = groupAddr;
	nb->fireChangeNotification(NF_IGMP_JOIN, &info);
}

void IGMP::leaveMulticastGroup(InterfaceEntry *ie, const IPv4Address& groupAddr)
{
	Enter_Method("leaveMulticastGroup");

	if (!ie) {
		ie = getFirstNonLoopbackInterface();
	}

	if (!ie) {
		opp_error("IGMP: could not find suitable local interface for groups");
	}

	const IPv4MulticastData::MulticastGroupMap &multicastGroups = ie->ipv4MulticastData()->getMulticastGroups();
	IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.find(groupAddr);
	if (it != multicastGroups.end()) {
	    IPv4InterfaceGroupData *group = it->second;
		if (group->host.refCount > 0) {
			group->host.refCount--;
			if (group->host.refCount == 0) {
				numHostGroups--;
				if (enabled) {
					if (group->host.state == IGMP_HGS_DELAYING_MEMBER) {
						cancelEvent(group->host.timer);
					}

					if (group->host.flag) {
						sendLeave(ie, group);
					}

					if (group->router.state == IGMP_RGS_NO_MEMBERS_PRESENT) {
						ie->ipv4MulticastData()->deleteGroup(group->groupAddr, this);
						numGroups--;
 					}
					else {
						// reset state, router is pinning this group down
						group->host.flag = false;
						group->host.state = IGMP_HGS_NON_MEMBER;
					}
				}
				else {
					ie->ipv4MulticastData()->deleteGroup(group->groupAddr, this);
					numGroups--;
				}
			}
		}
	}

	IPv4MulticastGroupInfo info;
	info.ie = ie;
	info.groupAddress = groupAddr;
	nb->fireChangeNotification(NF_IGMP_LEAVE, &info);
}

InterfaceEntry* IGMP::getFirstNonLoopbackInterface()
{
	for (int i = 0; i < ift->getNumInterfaces(); i++) {
		InterfaceEntry* ie = ift->getInterface(i);
		if (!ie->isLoopback()) {
			return ie;
		}
	}
	return NULL;
}

void IGMP::startTimer(IPv4InterfaceTimer *timer, double interval)
{
	ASSERT(timer);
	cancelEvent(timer);
	timer->nextExpiration = simTime() + interval;
	scheduleAt(timer->nextExpiration, timer);
}

void IGMP::startHostTimer(InterfaceEntry *ie, IPv4InterfaceGroupData* group, double maxRespTime)
{
	if (!group->host.timer) {
		group->host.timer = new IPv4InterfaceGroupTimer("IGMP group timer", ie, group);
	}

	double delay = uniform(0.0, maxRespTime);
	EV << "setting host timer for " << ie->getName() << " and group " << group->groupAddr.str() << " to " << delay << "\n";
	startTimer(group->host.timer, delay);
}

void IGMP::sendQuery(InterfaceEntry *ie, const IPv4Address& groupAddr, double maxRespTime)
{
	ASSERT(groupAddr != IPv4Address::ALL_HOSTS_MCAST);
	ASSERT(groupAddr != IPv4Address::ALL_ROUTERS_MCAST);

	if (ie->ipv4MulticastData()->getIGMPRouterState() == IGMP_RS_QUERIER) {
		IGMPMessage *msg = new IGMPMessage("IGMP query");
		msg->setType(IGMP_MEMBERSHIP_QUERY);
		msg->setGroupAddress(groupAddr);
		msg->setMaxRespTime((int)(maxRespTime * 10.0));
		msg->setByteLength(8);
		sendToIP(msg, ie, groupAddr.isUnspecified() ? IPv4Address::ALL_HOSTS_MCAST : groupAddr);

		numQueriesSent++;
		if (groupAddr.isUnspecified())
			numGeneralQueriesSent++;
		else
			numGroupSpecificQueriesSent++;
	}
}

void IGMP::sendReport(InterfaceEntry *ie, IPv4InterfaceGroupData* group)
{
	ASSERT(group->groupAddr != IPv4Address::ALL_HOSTS_MCAST);
	ASSERT(group->groupAddr != IPv4Address::ALL_ROUTERS_MCAST);

	if (!ie->isLoopback())
	{
		IGMPMessage *msg = new IGMPMessage("IGMP report");
		msg->setType(IGMPV2_MEMBERSHIP_REPORT);
		msg->setGroupAddress(group->groupAddr);
		msg->setByteLength(8);
		sendToIP(msg, ie, group->groupAddr);
		numReportsSent++;
	}
}

void IGMP::sendLeave(InterfaceEntry *ie, IPv4InterfaceGroupData* group)
{
	ASSERT(group->groupAddr != IPv4Address::ALL_HOSTS_MCAST);
	ASSERT(group->groupAddr != IPv4Address::ALL_ROUTERS_MCAST);

	if (!ie->isLoopback())
	{
		IGMPMessage *msg = new IGMPMessage("IGMP leave");
		msg->setType(IGMPV2_LEAVE_GROUP);
		msg->setGroupAddress(group->groupAddr);
		msg->setByteLength(8);
		sendToIP(msg, ie, IPv4Address::ALL_ROUTERS_MCAST);
		numLeavesSent++;
	}
}

void IGMP::sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest)
{
	ASSERT(!ie->isLoopback());

	IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
	controlInfo->setProtocol(IP_PROT_IGMP);
	controlInfo->setInterfaceId(ie->getInterfaceId());
	controlInfo->setTimeToLive(1);
	controlInfo->setDestAddr(dest);
	msg->setControlInfo(controlInfo);

	send(msg, "ipOut");
}

void IGMP::processIgmpMessage(IGMPMessage *msg)
{
	IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)msg->getControlInfo();
	InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());

	switch (msg->getType()) {
	case IGMP_MEMBERSHIP_QUERY:
		processQuery(ie, controlInfo->getSrcAddr(), msg);
		break;
	//case IGMPV1_MEMBERSHIP_REPORT:
	//	processV1Report(ie, msg);
	//	delete msg;
	//	break;
	case IGMPV2_MEMBERSHIP_REPORT:
		processV2Report(ie, msg);
		break;
	case IGMPV2_LEAVE_GROUP:
		processLeave(ie, msg);
		break;
	default:
		opp_error("IGMP: Unhandled message type");
		break;
	}
}

void IGMP::processQueryTimer(IPv4InterfaceTimer *msg)
{
    IGMPRouterState state = msg->ie->ipv4MulticastData()->getIGMPRouterState();
	if (state == IGMP_RS_QUERIER || state == IGMP_RS_NON_QUERIER) {
		msg->ie->ipv4MulticastData()->setIGMPRouterState(IGMP_RS_QUERIER);
		sendQuery(msg->ie, IPv4Address(), queryResponseInterval); // general query
		startTimer(msg, queryInterval);
	}
}

void IGMP::processGroupTimer(IPv4InterfaceGroupTimer *msg)
{
	if (msg == msg->group->host.timer) {
		sendReport(msg->ie, msg->group);
		msg->group->host.flag = true;
		msg->group->host.state = IGMP_HGS_IDLE_MEMBER;
	}
	else if (msg == msg->group->router.timer) {
		IPv4MulticastGroupInfo info;
		info.ie = msg->ie;
		info.groupAddress = msg->group->groupAddr;
		nb->fireChangeNotification(NF_IGMP_LEAVE, &info);
		numRouterGroups--;

		if (msg->group->router.state ==	IGMP_RGS_CHECKING_MEMBERSHIP) {
			cancelEvent(msg->group->router.rexmtTimer);
		}
		msg->group->router.state = IGMP_RGS_NO_MEMBERS_PRESENT;
		if (msg->group->host.refCount == 0) {
			msg->ie->ipv4MulticastData()->deleteGroup(msg->group->groupAddr, this);
			numGroups--;
		}
	}
	else if (msg == msg->group->router.rexmtTimer) {
		sendQuery(msg->ie, msg->group->groupAddr, lastMemberQueryInterval);
		startTimer(msg->group->router.rexmtTimer, lastMemberQueryInterval);
		msg->group->router.state = IGMP_RGS_CHECKING_MEMBERSHIP;
	}
	else {
		opp_error("IGMP: Unknown timer");
	}
}

void IGMP::processQuery(InterfaceEntry *ie, const IPv4Address& sender, IGMPMessage *msg)
{
    const IPv4MulticastData::MulticastGroupMap &multicastGroups = ie->ipv4MulticastData()->getMulticastGroups();
    IPv4MulticastData::MulticastGroupMap::const_iterator itEnd = multicastGroups.end();

	numQueriesRecv++;

	IPv4Address &groupAddr = msg->getGroupAddress();
	if (groupAddr.isUnspecified()) {
		// general query
		numGeneralQueriesRecv++;
		for (IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.begin(); it != itEnd; ++it) {
			processGroupQuery(ie, it->second, msg->getMaxRespTime());
		}
	}
	else {
		// group-specific query
		numGroupSpecificQueriesRecv++;
		IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.find(groupAddr);
		if (it != itEnd) {
			processGroupQuery(ie, it->second, msg->getMaxRespTime());
		}
	}

	if (rt->isIPForwardingEnabled()) {
		if (externalRouter) {
			send(msg, "routerOut");
			return;
		}

		if (sender < ie->ipv4Data()->getIPAddress()) {
			IPv4MulticastData *data = ie->ipv4MulticastData();
			startTimer(data->getIGMPQueryTimer(), otherQuerierPresentInterval);
			data->setIGMPRouterState(IGMP_RS_NON_QUERIER);
		}
	}

	delete msg;
}

void IGMP::processGroupQuery(InterfaceEntry *ie, IPv4InterfaceGroupData* group, int maxRespTime)
{
	double maxRespTimeSecs = (double)maxRespTime / 10.0;

	if (group->host.state == IGMP_HGS_DELAYING_MEMBER) {
		IPv4InterfaceGroupTimer *timer = group->host.timer;
		simtime_t maxAbsoluteRespTime = simTime() + maxRespTimeSecs;
		if (maxAbsoluteRespTime < timer->nextExpiration) {
			startHostTimer(ie, group, maxRespTimeSecs);
		}
	}
	else if (group->host.state == IGMP_HGS_IDLE_MEMBER) {
		startHostTimer(ie, group, maxRespTimeSecs);
		group->host.state = IGMP_HGS_DELAYING_MEMBER;
	}
	else {
		// ignored
	}
}

void IGMP::processV2Report(InterfaceEntry *ie, IGMPMessage *msg)
{
    const IPv4MulticastData::MulticastGroupMap &multicastGroups = ie->ipv4MulticastData()->getMulticastGroups();
	IPv4Address &groupAddr = msg->getGroupAddress();
	IPv4InterfaceGroupData* group = NULL;

	numReportsRecv++;

	IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.find(groupAddr);
	if (it != multicastGroups.end()) {
		group = it->second;
		if (group->host.state == IGMP_HGS_IDLE_MEMBER) {
			cancelEvent(group->host.timer);
			group->host.flag = false;
			group->host.state = IGMP_HGS_DELAYING_MEMBER;
		}
	}

	if (rt->isIPForwardingEnabled()) {
		if (externalRouter) {
			send(msg, "routerOut");
			return;
		}

		if (!group) {
			group = ie->ipv4MulticastData()->createGroup(groupAddr, false);
			numGroups++;
		}

		if (!group->router.timer) 
			group->router.timer = new IPv4InterfaceGroupTimer("IGMP leave timer", ie, group);
		if (!group->router.rexmtTimer)
			group->router.rexmtTimer = new IPv4InterfaceGroupTimer("IGMP rexmt timer", ie, group);

		if (group->router.state == IGMP_RGS_NO_MEMBERS_PRESENT) {
			IPv4MulticastGroupInfo info;
			info.ie = ie;
			info.groupAddress = group->groupAddr;
			nb->fireChangeNotification(NF_IGMP_JOIN, &info);
			numRouterGroups++;
		}

		startTimer(group->router.timer, groupMembershipInterval);
		group->router.state = IGMP_RGS_MEMBERS_PRESENT;
	}

	delete msg;
}

void IGMP::processLeave(InterfaceEntry *ie, IGMPMessage *msg)
{
	numLeavesRecv++;

	if (rt->isIPForwardingEnabled()) {
		if (externalRouter) {
			send(msg, "routerOut");
			return;
		}

		const IPv4MulticastData::MulticastGroupMap &multicastGroups = ie->ipv4MulticastData()->getMulticastGroups();
		IPv4Address &groupAddr = msg->getGroupAddress();

		IPv4MulticastData::MulticastGroupMap::const_iterator it = multicastGroups.find(groupAddr);
		if (it != multicastGroups.end()) {
		    IPv4InterfaceGroupData *group = it->second;
			if (group->router.state == IGMP_RGS_MEMBERS_PRESENT) {
				startTimer(group->router.rexmtTimer, lastMemberQueryInterval);
				if (ie->ipv4MulticastData()->getIGMPRouterState() == IGMP_RS_QUERIER) {
					startTimer(group->router.timer, lastMemberQueryInterval * lastMemberQueryCount);
				}
				else {
					double maxRespTimeSecs = (double)msg->getMaxRespTime() / 10.0;
					sendQuery(ie, groupAddr, maxRespTimeSecs * lastMemberQueryCount);
				}
				startTimer(group->router.rexmtTimer, lastMemberQueryInterval);
				group->router.state = IGMP_RGS_CHECKING_MEMBERSHIP;
			}
		}
	}

	delete msg;
}
