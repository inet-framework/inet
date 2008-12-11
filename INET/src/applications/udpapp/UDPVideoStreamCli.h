//
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


//
// based on the video streaming app of the similar name by Johnny Lai
//

#ifndef __INET_UDPVIDEOSTREAM_H
#define __INET_UDPVIDEOSTREAM_H

#include <omnetpp.h>
#include "UDPAppBase.h"

/**
 * A "Realtime" VideoStream client application.
 *
 * Basic video stream application. Clients connect to server and get a stream of
 * video back.
 */
class INET_API UDPVideoStreamCli : public UDPAppBase
{
  protected:
    // statistics
    cOutVector eed;

  protected:
    ///@name Overridden cSimpleModule functions
    //@{
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    //@}

  protected:
    virtual void requestStream();
    virtual void receiveStream(cPacket *msg);
};


#endif



