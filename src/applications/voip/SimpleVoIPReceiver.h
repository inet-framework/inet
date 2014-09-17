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

#ifndef __INET_SIMPLEVOIPRECEIVER_H
#define __INET_SIMPLEVOIPRECEIVER_H

#include <string.h>
#include <list>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class SimpleVoIPPacket;

/**
 * Implements a simple VoIP source. See the NED file for more information.
 */
class SimpleVoIPReceiver : public cSimpleModule, public ILifecycle
{
  private:
    class VoIPPacketInfo
    {
      public:
        unsigned int packetID;
        simtime_t creationTime;
        simtime_t arrivalTime;
        simtime_t playoutTime;
    };

    typedef std::list<VoIPPacketInfo *> PacketsList;
    typedef std::vector<VoIPPacketInfo> PacketsVector;

    class TalkspurtInfo
    {
      public:
        enum Status {
            EMPTY,
            ACTIVE,
            FINISHED
        };
        Status status;
        unsigned int talkspurtID;
        unsigned int talkspurtNumPackets;
        simtime_t voiceDuration;
        PacketsVector packets;

      public:
        TalkspurtInfo() : status(EMPTY), talkspurtID(-1) {}
        void startTalkspurt(SimpleVoIPPacket *pk);
        void finishTalkspurt() { status = FINISHED; packets.clear(); }
        bool checkPacket(SimpleVoIPPacket *pk);
        void addPacket(SimpleVoIPPacket *pk);
        bool isActive() { return status == ACTIVE; }
    };

    // parameters
    double emodelRo;
    unsigned int bufferSpace;
    int emodelIe;
    int emodelBpl;
    int emodelA;
    simtime_t playoutDelay;
    simtime_t mosSpareTime;    // spare time before calculating MOS (after calculated playout time of last packet)

    // state
    UDPSocket socket;
    cMessage *selfTalkspurtFinished;
    TalkspurtInfo currentTalkspurt;

    static simsignal_t packetLossRateSignal;
    static simsignal_t packetDelaySignal;
    static simsignal_t playoutDelaySignal;
    static simsignal_t playoutLossRateSignal;
    static simsignal_t mosSignal;
    static simsignal_t taildropLossRateSignal;

    double eModel(double delay, double loss);
    void evaluateTalkspurt(bool finish);
    void startTalkspurt(SimpleVoIPPacket *packet);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    virtual void finish();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    SimpleVoIPReceiver();
    ~SimpleVoIPReceiver();
};

} // namespace inet

#endif // ifndef __INET_SIMPLEVOIPRECEIVER_H

