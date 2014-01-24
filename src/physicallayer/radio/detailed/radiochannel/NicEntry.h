/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntry.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: Class to store information about a nic for the
 *              ConnectionManager module
 **************************************************************************/

#ifndef NICENTRY_H
#define NICENTRY_H

#include <omnetpp.h>
#include <map>

#include "INETDefs.h"
#include "Coord.h"

class DetailedRadioChannelAccess;

/**
 * @brief NicEntry is used by ConnectionManager to store the necessary
 * information for each nic
 *
 * @ingroup connectionManager
 * @author Daniel Willkomm
 * @sa ConnectionManager
 */
class INET_API NicEntry : public cObject
{
  public:
	/** @brief Type for NIC identifier. */
	typedef int     t_nicid;
	typedef t_nicid t_nicid_cref;

	/** @brief Comparator class for %NicEntry for usage in STL containers. */
	class NicEntryComparator {
	  public:
		bool operator() (const NicEntry* nic1, const NicEntry* nic2) const {
			return nic1->nicId < nic2->nicId;
		}
	};
  public:
	/** @brief Type for map from NicEntry pointer to a gate.*/
    typedef std::map<const NicEntry*, cGate*, NicEntryComparator> GateList;

    /** @brief module id of the nic for which information is stored*/
    t_nicid nicId;

    /** @brief Pointer to the NIC module */
    cModule *nicPtr;

    /** @brief Module id of the host module this nic belongs to*/
    int hostId;

    /** @brief Geographic location of the nic*/
    Coord pos;

    /** @brief Points to this nics ConnectionManagerAccess module */
    DetailedRadioChannelAccess* chAccess;

  protected:
    /** @brief Outgoing connections of this nic
     *
     * This map stores all connection for this nic to other nics
     *
     * The first entry is the module id of the nic the connection is
     * going to and the second the gate to send the msg to
     **/
    GateList outConns;

  public:
    /**
     * @brief Constructor, initializes all members
     */
    NicEntry()
      : cObject()
      , nicId(0)
      , nicPtr(NULL)
      , hostId(0)
      , pos()
      , chAccess(NULL)
      , outConns()
    { }

    NicEntry(const NicEntry& o)
      : cObject(o)
      , nicId(o.nicId)
      , nicPtr(o.nicPtr)
      , hostId(o.hostId)
      , pos(o.pos)
      , chAccess(o.chAccess)
      , outConns(o.outConns)
    { }

    void swap(NicEntry& s)
    {
    	std::swap(nicId, s.nicId);
    	std::swap(nicPtr, s.nicPtr);
    	std::swap(hostId, s.hostId);
    	std::swap(pos, s.pos);
    	std::swap(chAccess, s.chAccess);
    	std::swap(outConns, s.outConns);
    }

    NicEntry& operator=(const NicEntry& o)
    {
    	nicId     = o.nicId;
    	nicPtr    = o.nicPtr;
    	hostId    = o.hostId;
    	pos       = o.pos;
    	chAccess  = o.chAccess;
    	outConns  = o.outConns;
    	return *this;
    }
    /**
     * @brief Destructor -- needs to be there...
     */
    virtual ~NicEntry() {}

    /** @brief Connect two nics */
    virtual void connectTo(NicEntry*) = 0;

    /** @brief Disconnect two nics */
    virtual void disconnectFrom(NicEntry*) = 0;

    /** @brief return the actual gateList*/
    const GateList& getGateList() const {
    	return outConns;
    }

    /** @brief Checks if this nic is connected to the "other" nic*/
    bool isConnected(const NicEntry* other) {
        return (outConns.find(other) != outConns.end());
    }

    /**
     * Called by P2PPhyLayer. Needed to send a packet directly to a
     * certain nic without other nodes 'hearing' it. This is only useful
     * for physical layers that work with bit error probability like
     * P2PPhyLayer.
     *
     * @param to pointer to the NicEntry to which the packet is about to be sent
     */
    const cGate* getOutGateTo(const NicEntry* to) const {
    	return outConns.at(to);
    }

};

#endif
