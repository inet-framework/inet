//
// VoIPReceiver.h
//
// Created on: 01/feb/2011
//     Author: Adriano
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
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

class SimpleVoIPReceiver : public cSimpleModule
{
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
        unsigned int talkspurtID;
        unsigned int talkspurtNumPackets;
        simtime_t voiceDuration;
        PacketsVector  packets;
      public:
        void startTalkspurt(SimpleVoIPPacket *pk);
        bool checkPacket(SimpleVoIPPacket *pk);
        void addPacket(SimpleVoIPPacket *pk);
    };

    UDPSocket socket;

    ~SimpleVoIPReceiver();

    // FIXME: avoid _ characters
    int         emodel_Ie;
    int         emodel_Bpl;
    int         emodel_A;
    double      emodel_Ro;


    PacketsList  playoutQueue;
    TalkspurtInfo currentTalkspurt;
    unsigned int bufferSpace;
    simtime_t    playoutDelay;

    simsignal_t packetLossRateSignal;
    simsignal_t packetDelaySignal;
    simsignal_t playoutDelaySignal;
    simsignal_t playoutLossRateSignal;
    simsignal_t mosSignal;
    simsignal_t taildropLossRateSignal;

    virtual void finish();

  protected:
    virtual int numInitStages() const {return 4;}
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    double eModel(double delay, double loss);
    void evaluateTalkspurt(bool finish);
};


#endif /* VOIPRECEIVER_H_ */
