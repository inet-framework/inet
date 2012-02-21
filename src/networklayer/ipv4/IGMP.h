#pragma once

#include "INETDefs.h"
#include "INotifiable.h"
#include "IPv4Address.h"
#include "IGMPMessage_m.h"

#include <set>

class IInterfaceTable;
class IRoutingTable;
class InterfaceEntry;
class NotificationBoard;
class IPv4MulticastData;

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

class IPv4InterfaceGroupTimer;

struct IPv4InterfaceGroupData
{
	IPv4Address groupAddr;

	struct Host
	{
		IGMPHostGroupState state;
		bool flag;                // true when we were the last host to send a report for this group
		IPv4InterfaceGroupTimer *timer;
		int refCount;
	} host;

	struct Router
	{
		IGMPRouterGroupState state;
		IPv4InterfaceGroupTimer *timer;
		IPv4InterfaceGroupTimer *rexmtTimer;
		//IPv4InterfaceGroupTimer *v1HostTimer;
	} router;
};

class INET_API IPv4MulticastGroupInfo : public cPolymorphic
{
public:
	InterfaceEntry* ie;
	IPv4Address groupAddress;
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

class INET_API IPv4InterfaceGroupTimer : public IPv4InterfaceTimer
{
public:
	IPv4InterfaceGroupData* group;

public:
	IPv4InterfaceGroupTimer(const char* name, InterfaceEntry* ie, IPv4InterfaceGroupData* group)
		: IPv4InterfaceTimer(name, ie)
		, group(group)
	{}

	virtual ~IPv4InterfaceGroupTimer() {}
};

class INET_API IGMP : public cSimpleModule, protected INotifiable
{
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

	//std::set<IPv4MulticastData *, ComparePointers<IPv4MulticastData>> allMulticastData;
	std::set<IPv4MulticastData *> allMulticastData;

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
	virtual int numInitStages() const { return 2; }
	virtual void initialize(int stage);
	virtual void finish();
	virtual void handleMessage(cMessage *msg);
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);

public:
	virtual ~IGMP();

	void configureInterface(InterfaceEntry *ie);

	/**
	 * Emulation of the socket option: IP_ADD_MEMBERSHIP
	 */
	void joinMulticastGroup(InterfaceEntry *ie, const IPv4Address& groupAddr);

	/**
	 * Emulation of the socket option: IP_DROP_MEMBERSHIP
	 */
	void leaveMulticastGroup(InterfaceEntry *ie, const IPv4Address& groupAddr);

private:
	InterfaceEntry* getFirstNonLoopbackInterface();
	void deleteMulticastData(IPv4MulticastData *data);

	void startTimer(IPv4InterfaceTimer *timer, double interval);
	void startHostTimer(InterfaceEntry *ie, IPv4InterfaceGroupData* group, double maxRespTime);

	void sendQuery(InterfaceEntry *ie, const IPv4Address& groupAddr, double maxRespTime);
	void sendReport(InterfaceEntry *ie, IPv4InterfaceGroupData* group);
	void sendLeave(InterfaceEntry *ie, IPv4InterfaceGroupData* group);
	void sendToIP(IGMPMessage *msg, InterfaceEntry *ie, const IPv4Address& dest);

	void processQueryTimer(IPv4InterfaceTimer *msg);
	void processGroupTimer(IPv4InterfaceGroupTimer *msg);

	void processIgmpMessage(IGMPMessage *msg);
	void processQuery(InterfaceEntry *ie, const IPv4Address& sender, IGMPMessage *msg);
	void processGroupQuery(InterfaceEntry *ie, IPv4InterfaceGroupData* group, int maxRespTime);
	//void processV1Report(InterfaceEntry *ie, IGMPMessage *msg);
	void processV2Report(InterfaceEntry *ie, IGMPMessage *msg);
	void processLeave(InterfaceEntry *ie, IGMPMessage *msg);
};
