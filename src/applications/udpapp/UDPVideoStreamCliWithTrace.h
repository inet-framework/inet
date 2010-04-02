//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_UDPVIDEOSTREAM_WITHTRACE_H
#define __INET_UDPVIDEOSTREAM_WITHTRACE_H

#include <omnetpp.h>
#include "UDPVideoStreamCli.h"
#include "UDPVideoStreamPacket_m.h"


///
/// @class UDPVideoStreamCliWithTrace
/// @brief Implementation of a video streaming client to work with
///        UDPVideoStreamSvrWithTrace
///
class INET_API UDPVideoStreamCliWithTrace : public UDPVideoStreamCli
{
  protected:
	// module parameters
	double startupDelay;

    // statistics
//    cOutVector eed;
	unsigned long numPktRcvd;	///< number of packets received
	unsigned long numPktLost;	///< number of packets lost

	// variables for detecting packet losses
	bool isFirstPacket;	///< indicator of whether the current packet is the first one or not
	unsigned short prevSequenceNumber;	///< sequence number of the previously received packet

  protected:
    ///@name Overridden cSimpleModule functions
	//@{
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);
	//@}

  protected:
//    virtual void requestStream();
    virtual void receiveStream(UDPVideoStreamPacket *pkt);
};


#endif



