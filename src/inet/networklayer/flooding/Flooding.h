//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/* -*- mode:c++ -*- ********************************************************
 * file:        Flooding.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/

#ifndef __INET_FLOODING_H
#define __INET_FLOODING_H

#include <list>

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/base/NetworkProtocolBase.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/flooding/FloodingHeader_m.h"

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
class INET_API Flooding : public NetworkProtocolBase, public INetworkProtocol
{
  protected:
    /** @brief Network layer sequence number*/
    unsigned long seqNum = 0;

    /** @brief cached variable of my networ address */
    L3Address myNetwAddr;

    /** @brief Length of the header*/
    int headerLength = 0;

    /** @brief Default time-to-live (ttl) used for this module*/
    int defaultTtl = 0;

    /** @brief Defines whether to use plain flooding or not*/
    bool plainFlooding = false;

    class INET_API Bcast {
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
    unsigned int bcMaxEntries = 0;

    /**
     * @brief Time after which an entry for an already broadcasted msg
     * can be deleted
     **/
    simtime_t bcDelTime;

    long nbDataPacketsReceived = 0;
    long nbDataPacketsSent = 0;
    long nbDataPacketsForwarded = 0;
    long nbHops = 0;

  public:
    Flooding() {}

    /** @brief Initialization of omnetpp.ini parameters*/
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int) override;

    virtual void finish() override;

    const Protocol& getProtocol() const override { return Protocol::flooding; }

  protected:

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Checks whether a message was already broadcasted*/
    bool notBroadcasted(const FloodingHeader *);

    void decapsulate(Packet *packet);
    void encapsulate(Packet *packet);
    void forwardPacket(Packet *packet);

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
    virtual void setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr);

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override {} // TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override {} // TODO implementation
    virtual void handleCrashOperation(LifecycleOperation *operation) override {} // TODO implementation
};

} // namespace inet

#endif

