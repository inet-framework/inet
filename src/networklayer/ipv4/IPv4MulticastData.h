#pragma once

#include "INETDefs.h"

#include "InterfaceEntry.h"
#include "IPv4Address.h"

class IGMP;
class IPv4InterfaceTimer;
struct IPv4InterfaceGroupData;

enum IGMPRouterState
{
	IGMP_RS_INITIAL,
	IGMP_RS_QUERIER,
	IGMP_RS_NON_QUERIER,
};

class INET_API IPv4MulticastData : public InterfaceProtocolData
{
	friend class IGMP;
public:
	typedef std::map<IPv4Address, IPv4InterfaceGroupData*> MulticastGroupMap;

protected:
	MulticastGroupMap multicastGroups; ///< multicast groups
	IGMPRouterState igmpRouterState;
	IPv4InterfaceTimer *igmpQueryTimer;

protected:
	void changed1() {changed(NF_INTERFACE_IPv4CONFIG_CHANGED);}
	IPv4InterfaceGroupData* createGroup(const IPv4Address& groupAddress, bool isHost);
	void deleteGroup(const IPv4Address& groupAddr, cSimpleModule* owner);
	void deleteGroup(IPv4InterfaceGroupData* group, cSimpleModule* owner);
	void destroy(cSimpleModule* owner);

public:
	IPv4MulticastData();
	virtual ~IPv4MulticastData();
	virtual std::string info() const;
	virtual std::string detailedInfo() const;

	/** @name Getters */
	//@{
	IGMPRouterState getIGMPRouterState() const {return igmpRouterState;}
	IPv4InterfaceTimer *getIGMPQueryTimer() const {return igmpQueryTimer;}
	bool isMemberOfMulticastGroup(const IPv4Address& multicastAddress) const;
	const MulticastGroupMap& getMulticastGroups() const { return multicastGroups; }
	//@}

	/** @name Setters */
	//@{
	void setIGMPRouterState(IGMPRouterState s) {igmpRouterState = s; changed1();}
	void setIGMPQueryTimer(IPv4InterfaceTimer* t) {igmpQueryTimer = t; changed1();}
	//@}
};
