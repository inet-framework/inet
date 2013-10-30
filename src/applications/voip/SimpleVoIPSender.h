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

#ifndef VOIPSENDER_H_
#define VOIPSENDER_H_

#include <string.h>
#include "INETDefs.h"
#include "UDPSocket.h"
#include "AddressResolver.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

/**
 * Implements a simple VoIP source. See the NED file for more information.
 */
class SimpleVoIPSender : public InetSimpleModule, public ILifecycle
{
  private:
    UDPSocket socket;

    // source
    simtime_t talkspurtDuration;
    simtime_t silenceDuration;
    bool      isTalk;

    // FIXME: be more specific with the name of this self message
    cMessage* selfSource;   // timer for changing talkspurt/silence periods

    int talkspurtID;
    int talkspurtNumPackets;
    int packetID;
    int talkPacketSize;
    simtime_t packetizationInterval;

    // ----------------------------
    cMessage *selfSender;   // timer for sending packets

    simtime_t timestamp;
    int localPort;
    int destPort;
    Address destAddress;
    simtime_t stopTime;

    void talkspurt(simtime_t dur);
    void selectPeriodTime();
    void sendVoIPPacket();

  public:
    virtual ~SimpleVoIPSender();
    SimpleVoIPSender();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif /* VOIPSENDER_H_ */

