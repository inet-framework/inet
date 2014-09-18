/* -*- mode:c++ -*- ********************************************************
 * file:        Flood.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/

#ifndef __INET_FLOOD_H
#define __INET_FLOOD_H

#include <list>
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/common/NetworkProtocolBase.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/flood/FloodDatagram.h"

namespace inet {

/**
 * @brief A simple flooding protocol
 *
 * This implementation uses plain flooding, i.e. it "remembers"
 * (stores) already broadcasted messages in a list and does not
 * rebroadcast them again, if it gets another copy of that message.
 *
 * The maximum number of entires for that list can be defined in the
 * .ini file (@ref bcMaxEntries) as well as the time after which an entry
 * is deleted (@ref bcDelTime).
 *
 * If you prefere a memory-less version you can comment out the
 * @verbatim #define PLAINFLOODING @endverbatim
 *
 * @ingroup netwLayer
 * @author Daniel Willkomm
 *
 * ported to Mixim 2.0 by Theodoros Kapourniotis
 **/
class INET_API Flood : public NetworkProtocolBase, public INetworkProtocol
{
  protected:
    /** @brief Network layer sequence number*/
    unsigned long seqNum;

    /** @brief cached variable of my networ address */
    L3Address myNetwAddr;

    /** @brief Length of the header*/
    int headerLength;

    /** @brief Default time-to-live (ttl) used for this module*/
    int defaultTtl;

    /** @brief Defines whether to use plain flooding or not*/
    bool plainFlooding;

    class Bcast
    {
      public:
        unsigned long seqNum;
        L3Address srcAddr;
        simtime_t delTime;

      public:
        Bcast(unsigned long n = 0, const L3Address& s = L3Address(), simtime_t_cref d = SIMTIME_ZERO) :
            seqNum(n), srcAddr(s), delTime(d)
        {
        }
    };

    typedef std::list<Bcast> cBroadcastList;

    /** @brief List of already broadcasted messages*/
    cBroadcastList bcMsgs;

    /**
     * @brief Max number of entries in the list of already broadcasted
     * messages
     **/
    unsigned int bcMaxEntries;

    /**
     * @brief Time after which an entry for an already broadcasted msg
     * can be deleted
     **/
    simtime_t bcDelTime;

  public:
    Flood()
        : NetworkProtocolBase()
        , seqNum(0)
        , defaultTtl(0)
        , plainFlooding(false)
        , bcMsgs()
        , bcMaxEntries(0)
        , bcDelTime()
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbDataPacketsForwarded(0)
        , nbHops(0)
    {}

    /** @brief Initialization of omnetpp.ini parameters*/
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual void initialize(int);

    virtual void finish();

  protected:

    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long nbDataPacketsForwarded;
    long nbHops;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket *);

    /** @brief Checks whether a message was already broadcasted*/
    bool notBroadcasted(FloodDatagram *);

    cMessage *decapsMsg(FloodDatagram *);

    FloodDatagram *encapsMsg(cPacket *);

    /**
     * @brief Attaches a "control info" (NetwToMac) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pDestAddr The MAC address of the message receiver.
     */
    virtual cObject *setDownControlInfo(cMessage *const pMsg, const MACAddress& pDestAddr);
};

} // namespace inet

#endif // ifndef __INET_FLOOD_H

