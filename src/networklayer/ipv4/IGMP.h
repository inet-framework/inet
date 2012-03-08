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

#ifndef __INET_IGMP_H
#define __INET_IGMP_H

#include "INETDefs.h"
#include "INotifiable.h"
#include "IPv4Address.h"
#include "IGMPMessage_m.h"
#include "InterfaceEntry.h"

#include <set>

class IInterfaceTable;
class IRoutingTable;
class NotificationBoard;

enum IGMPRouterState
{
    IGMP_RS_INITIAL,
    IGMP_RS_QUERIER,
    IGMP_RS_NON_QUERIER,
};

enum IGMPRouterGroupState
{
	IGMP_RGS_NO_MEMBERS_PRESENT,
	IGMP_RGS_MEMBERS_PRESENT,
	IGMP_RGS_V1_MEMBERS_PRESENT,
	IGMP_RGS_CHECKING_MEMBERSHIP,
};

enum IGMPHostGroupState
{
	IGMP_HGS_NON_MEMBER,
	IGMP_HGS_DELAYING_MEMBER,
	IGMP_HGS_IDLE_MEMBER,
};

class INET_API IPv4InterfaceTimer : public cMessage
{
public:
	InterfaceEntry* ie;
	simtime_t nextExpiration;

public:
	IPv4InterfaceTimer(const char* name, InterfaceEntry* ie)
		: cMessage(name, 0)
		, ie(ie)
	{}

	virtual ~IPv4InterfaceTimer() {}
};

class IPv4InterfaceGroupTimer;

class INET_API IGMP : public cSimpleModule, protected INotifiable
{
        friend class IPv4InterfaceGroupTimer;
protected:
        struct HostGroupData
        {
            IGMP *owner;
            IPv4Address groupAddr;
            IGMPHostGroupState state;
            bool flag;                // true when we were the last host to send a report for this group
            IPv4InterfaceGroupTimer *timer;

            HostGroupData(IGMP *owner, const IPv4Address &group);
            virtual ~HostGroupData();
        };
        typedef std::map<IPv4Address, HostGroupData*> GroupToHostDataMap;

        struct RouterGroupData
        {
            IGMP *owner;
            IPv4Address groupAddr;
            IGMPRouterGroupState state;
            IPv4InterfaceGroupTimer *timer;
            IPv4InterfaceGroupTimer *rexmtTimer;
            //IPv4InterfaceGroupTimer *v1HostTimer;

            RouterGroupData(IGMP *owner, const IPv4Address &group);
            virtual ~RouterGroupData();
        };
        typedef std::map<IPv4Address, RouterGroupData*> GroupToRouterDataMap;

        struct HostInterfaceData
        {
            IGMP *owner;
            GroupToHostDataMap groups;

            HostInterfaceData(IGMP *owner);
            virtual ~HostInterfaceData();
        };
        typedef std::map<int, HostInterfaceData*> InterfaceToHostDataMap;

        struct RouterInterfaceData
        {
            IGMP *owner;
            GroupToRouterDataMap groups;
            IGMPRouterState igmpRouterState;
            IPv4InterfaceTimer *igmpQueryTimer;

            RouterInterfaceData(IGMP *owner);
            virtual ~RouterInterfaceData();
        };
        typedef std::map<int, RouterInterfaceData*> InterfaceToRouterDataMap;

protected:
	IRoutingTable *rt;     // cached pointer
	IInterfaceTable *ift;  // cached pointer
	NotificationBoard *nb; // cached pointer

	bool enabled;
	bool externalRouter;
	int robustness;                          // RFC 2236: Section 8.1
	double queryInterval;                    // RFC 2236: Section 8.2
	double queryResponseInterval;            // RFC 2236: Section 8.3
	double groupMembershipInterval;          // RFC 2236: Section 8.4
	double otherQuerierPresentInterval;      // RFC 2236: Section 8.5
	double startupQueryInterval;             // RFC 2236: Section 8.6
	double startupQueryCount;                // RFC 2236: Section 8.7
	double lastMemberQueryInterval;          // RFC 2236: Section 8.8
	double lastMemberQueryCount;             // RFC 2236: Section 8.9
	double unsolicitedReportInterval;        // RFC 2236: Section 8.10
	//double version1RouterPresentInterval;  // RFC 2236: Section 8.11

	// state variables per interface
	InterfaceToHostDataMap hostData;
	InterfaceToRouterDataMap routerData;

	// group counters
	int numGroups;
	int numHostGroups;
	int numRouterGroups;

	// message counters
	int numQueriesSent;
	int numQueriesRecv;
	int numGeneralQueriesSent;
	int numGeneralQueriesRecv;
	int numGroupSpecificQueriesSent;
	int numGroupSpecificQueriesRecv;
	int numReportsSent;
	int numReportsRecv;
	int numLeavesSent;
	int numLeavesRecv;

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);

public:
	virtual ~IGMP();

	void configureInterface(InterfaceEntry *ie);

private:
	HostGroupData *createHostGroupData(InterfaceEntry *ie, const IPv4Address &group);
	RouterGroupData *createRouterGroupData(InterfaceEntry *ie, const IPv4Address &group);
    HostInterfaceData *getHostInterfaceData(InterfaceEntry *ie);
	RouterInterfaceData *getRouterInterfaceData(InterfaceEntry *ie);
    HostGroupData *getHostGroupData(InterfaceEntry *ie, const IPv4Address &group);
    RouterGroupData *getRouterGroupData(InterfaceEntry *ie, const IPv4Address &group);
    void deleteHostInterfaceData(int interfaceId);
    void deleteRouterInterfaceData(int interfaceId);
    void deleteHostGroupData(InterfaceEntry *ie, const IPv4Address &group);
    void deleteRouterGroupData(InterfaceEntry *ie, const IPv4Address &group);

    void multicastGroupJoined(InterfaceEntry *ie, const IPv4Address& groupAddr);
    void multicastGroupLeft(InterfaceEntry *ie, const IPv4Address& groupAddr);

	void startTimer(IPv4InterfaceTimer *timer, double interval);
	void startHostTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime);

	void sendQuery(InterfaceEntry *ie, const IPv4Address& groupAddr, double maxRespTime);
	void sendReport(InterfaceEntry *ie, HostGroupData* group);
	void sendLeave(InterfaceEntry *ie, HostGroupData* group);
	void sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest);

	void processQueryTimer(IPv4InterfaceTimer *msg);
	void processGroupTimer(IPv4InterfaceGroupTimer *msg);

	void processIgmpMessage(IGMPMessage *msg);
	void processQuery(InterfaceEntry *ie, const IPv4Address& sender, IGMPMessage *msg);
	void processGroupQuery(InterfaceEntry *ie, HostGroupData* group, int maxRespTime);
	//void processV1Report(InterfaceEntry *ie, IGMPMessage *msg);
	void processV2Report(InterfaceEntry *ie, IGMPMessage *msg);
	void processLeave(InterfaceEntry *ie, IGMPMessage *msg);
};

class INET_API IPv4InterfaceGroupTimer : public IPv4InterfaceTimer
{
public:
    IGMP::HostGroupData* hostGroup;
    IGMP::RouterGroupData* routerGroup;

public:
    IPv4InterfaceGroupTimer(const char* name, InterfaceEntry* ie, IGMP::HostGroupData* group)
        : IPv4InterfaceTimer(name, ie)
        , hostGroup(group), routerGroup(NULL)
    {}

    IPv4InterfaceGroupTimer(const char* name, InterfaceEntry* ie, IGMP::RouterGroupData* group)
        : IPv4InterfaceTimer(name, ie)
        , hostGroup(NULL), routerGroup(group)
    {}

    virtual ~IPv4InterfaceGroupTimer() {}
};



#endif
