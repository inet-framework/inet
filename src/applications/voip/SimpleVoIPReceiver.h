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

#ifndef VOIPRECEIVER_H_
#define VOIPRECEIVER_H_

#include <string.h>
#include <list>
#include "INETDefs.h"
#include "IPvXAddressResolver.h"
#include "UDPSocket.h"

class SimpleVoIPPacket;

/**
 * Implements a simple VoIP source. See the NED file for more information.
 */
class SimpleVoIPReceiver : public cSimpleModule
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

    typedef std::list<VoIPPacketInfo*> PacketsList;
    typedef std::vector<VoIPPacketInfo> PacketsVector;

    class TalkspurtInfo
    {
      public:
        enum Status {
            EMPTY,
            ACTIVE,
            FINISHED
        };
        Status  status;
        unsigned int talkspurtID;
        unsigned int talkspurtNumPackets;
        simtime_t voiceDuration;
        PacketsVector  packets;
      public:
        TalkspurtInfo() : status(EMPTY), talkspurtID(-1) {}
        void startTalkspurt(SimpleVoIPPacket *pk);
        void finishTalkspurt() { status = FINISHED; packets.clear(); }
        bool checkPacket(SimpleVoIPPacket *pk);
        void addPacket(SimpleVoIPPacket *pk);
        bool isActive() { return (status == ACTIVE); }
    };

    UDPSocket socket;

    int emodel_Ie;
    int emodel_Bpl;
    int emodel_A;
    double emodel_Ro;

    cMessage* selfTalkspurtFinished;

    TalkspurtInfo currentTalkspurt;
    unsigned int bufferSpace;
    simtime_t playoutDelay;
    simtime_t mosSpareTime; // spare time before calculating MOS (after calculated playout time of last packet)

    simsignal_t packetLossRateSignal;
    simsignal_t packetDelaySignal;
    simsignal_t playoutDelaySignal;
    simsignal_t playoutLossRateSignal;
    simsignal_t mosSignal;
    simsignal_t taildropLossRateSignal;

    double eModel(double delay, double loss);
    void evaluateTalkspurt(bool finish);
    void startTalkspurt(SimpleVoIPPacket* packet);

  public:
    SimpleVoIPReceiver();
    ~SimpleVoIPReceiver();

  protected:
    virtual int numInitStages() const {return 4;}
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    virtual void finish();
};


#endif /* VOIPRECEIVER_H_ */
