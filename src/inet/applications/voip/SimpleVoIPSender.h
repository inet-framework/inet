//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 Opensim Ltd.
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

#ifndef __INET_SIMPLEVOIPSENDER_H
#define __INET_SIMPLEVOIPSENDER_H

#include <string.h>
#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * Implements a simple VoIP source. See the NED file for more information.
 */
class SimpleVoIPSender : public cSimpleModule, public ILifecycle
{
  private:
    UDPSocket socket;

    // parameters
    simtime_t stopTime;
    simtime_t packetizationInterval;
    int localPort, destPort;
    int talkPacketSize;
    L3Address destAddress;

    // state
    cMessage *selfSender;    // timer for sending packets
    cMessage *selfSource;    // timer for changing talkspurt/silence periods - FIXME: be more specific with the name of this self message
    simtime_t silenceDuration;
    simtime_t talkspurtDuration;
    int packetID;
    int talkspurtID;
    int talkspurtNumPackets;
    bool isTalk;

    void talkspurt(simtime_t dur);
    void selectPeriodTime();
    void sendVoIPPacket();

    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    virtual ~SimpleVoIPSender();
    SimpleVoIPSender();
};

} // namespace inet

#endif // ifndef __INET_SIMPLEVOIPSENDER_H

