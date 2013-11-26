/* -*- mode:c++ -*- ********************************************************
 * file:        Packet.h
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA
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
 **************************************************************************/

#ifndef BBPACKET_H
#define BBPACKET_H

#include <sstream>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "SimpleAddress.h"

/**
 * @brief Class that keeps track of the number of packets sent.
 *
 * @ingroup utils
 * @author Andreas Koepke, Karl Wessel
 */

class MIXIM_API  Packet : public cObject
{
protected:
    /** @brief number of packets generated. */
    long             nbPacketsReceived;
    long             nbPacketsReceivedNoRS;
    long             nbPacketsSent;
    /** @brief The size of each of the received and sent packet.*/
    long             packetBitLength;
    bool             sent;
    LAddress::L3Type host;

public:

    /** @brief Constructor*/
    Packet(long bitLength, long rcvd=0, long sent=0) :
	 cObject(), nbPacketsReceived(rcvd), nbPacketsReceivedNoRS(rcvd),
	 nbPacketsSent(sent), packetBitLength(bitLength), sent(true), host(0) {
    };

    double getNbPacketsReceived() const  {
        return nbPacketsReceived;
    }

    double getNbBitsReceived() const {
		return nbPacketsReceived * packetBitLength;
	}

    double getNbPacketsReceivedNoRS() const  {
        return nbPacketsReceived;
    }

    double getNbPacketsSent() const  {
            return nbPacketsSent;
	}

    double getNbBitsSent() const {
    	return nbPacketsSent * packetBitLength;
    }

    void setNbPacketsReceived(int n)  {
        nbPacketsReceived = n;
    }

    void setNbPacketsReceivedNoRS(int n)  {
        nbPacketsReceivedNoRS = n;
    }

    void setNbPacketsSent(int n)  {
            nbPacketsSent = n;
	}

    void setBitLength(long bitLength) {
    	packetBitLength = bitLength;
    }

    long getBitLength() const {
    	return packetBitLength;
    }

    void setHost(const LAddress::L3Type& h) {
    	host = h;
    }

    const LAddress::L3Type& getHost() const {
    	return host;
    }

    bool isSent() const {
    	return sent;
    }

    void setPacketSent(bool isSent) {
    	sent = isSent;
    }

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " Number of packets generated is "<< 0;
        return ost.str();
    }
};


#endif
