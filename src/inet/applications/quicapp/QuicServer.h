//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_QUIC_QUICSERVER_H_
#define __INET_QUIC_QUICSERVER_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/quic/QuicSocket.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace omnetpp;

namespace inet {

enum IndicationCode {
    APP_START_READ_DATA,
    APP_STOP_READ_DATA
};

struct dataRequest{
    uint64_t expectedDataSize = 0; //0 = infiniter, app is ready to read all avaliable data
    uint64_t avaliableDataSize = 0;
    cMessage *timer = nullptr;
    int counter = 0;
};

class QuicServer : public ApplicationBase
{
  protected:
    QuicSocket socket;
    std::map<uint64_t, dataRequest *> dataRequestMap;
    simtime_t stopTime;
    cMessage *timerStopRead;

  protected:
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual dataRequest *findOrCreateDataRequest(uint64_t streamId);

    virtual void sendDataRequest(cMessage *msg);
    virtual void resendDataRequest(cMessage *msg);
    virtual void readDataFromQuic(cMessage *msg);

    virtual void startTimerReadyToRead(uint64_t streamId);
    virtual void stopTimersReadyToRead();
};

} //namespace

#endif
