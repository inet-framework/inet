//
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

#ifndef __INET_IPv4MULTICASTDATA_H
#define __INET_IPv4MULTICASTDATA_H

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

#endif
