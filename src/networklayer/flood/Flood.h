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

#ifndef _FLOOD_H_
#define _FLOOD_H_

#include <list>

#include "INETDefs.h"
#include "Address.h"
#include "InterfaceTableAccess.h"
#include "FloodDatagram.h"

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
class INET_API Flood : public cSimpleModule
{
protected:
    /** @brief Interface table of node*/
    IInterfaceTable *interfaceTable;

    /** @brief Network layer sequence number*/
    unsigned long seqNum;

    /** @brief cached variable of my networ address */
    ModuleIdAddress myNetwAddr;

    /** @brief Length of the header*/
    int headerLength;

    /** @brief Default time-to-live (ttl) used for this module*/
    int defaultTtl;

    /** @brief Defines whether to use plain flooding or not*/
    bool plainFlooding;

    class Bcast {
    public:
        unsigned long    seqNum;
        Address          srcAddr;
        simtime_t        delTime;
    public:
        Bcast(unsigned long n = 0, const Address& s = Address(),  simtime_t_cref d = SIMTIME_ZERO) :
            seqNum(n), srcAddr(s), delTime(d) {
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
    	: cSimpleModule()
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
    virtual int numInitStages() const { return 2; }
    virtual void initialize(int);
    virtual void finish();    

    /** @brief Called every time a message arrives*/
    virtual void handleMessage( cMessage* );

protected:

    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long nbDataPacketsForwarded;
    long nbHops;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage *);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *);

    /** @brief we have no self messages */
    virtual void handleSelfMsg(cMessage* /*msg*/) {};

    /** @brief Sends a message to the lower layer
     */
    void sendDown(cMessage *msg);

    /** @brief Sends a message to the upper layer
     */
    void sendUp(cMessage *msg);
    
    /** @brief Checks whether a message was already broadcasted*/
    bool notBroadcasted(FloodDatagram *);

    cMessage* decapsMsg(FloodDatagram *);

    FloodDatagram * encapsMsg(cPacket*);

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
    virtual cObject* setDownControlInfo(cMessage *const pMsg, const MACAddress& pDestAddr);
    /**
     * @brief Attaches a "control info" (NetwToUpper) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pSrcAddr  The MAC address of the message receiver.
     */
    virtual cObject* setUpControlInfo(cMessage *const pMsg, const Address& pSrcAddr);
};

#endif
