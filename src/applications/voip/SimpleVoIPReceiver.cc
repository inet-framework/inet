//
// VoIPReceiver.cc
//
// Created on: 01/feb/2011
//    Author: Adriano
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

#include "SimpleVoIPReceiver.h"

#include "SimpleVoIPPacket_m.h"

Define_Module(SimpleVoIPReceiver);

void SimpleVoIPReceiver::TalkspurtInfo::startTalkspurt(SimpleVoIPPacket *pk)
{
    status = ACTIVE;
    talkspurtID = pk->getTalkspurtID();
    talkspurtNumPackets = pk->getTalkspurtNumPackets();
    voiceDuration = pk->getVoiceDuration();
    packets.clear();
    packets.reserve(talkspurtNumPackets * 1.1);
    addPacket(pk);
}

bool SimpleVoIPReceiver::TalkspurtInfo::checkPacket(SimpleVoIPPacket *pk)
{
    return     talkspurtID == pk->getTalkspurtID()
            && talkspurtNumPackets == pk->getTalkspurtNumPackets()
            && voiceDuration == pk->getVoiceDuration();
}

void SimpleVoIPReceiver::TalkspurtInfo::addPacket(SimpleVoIPPacket *pk)
{
    VoIPPacketInfo packet;
    packet.packetID = pk->getPacketID();
    packet.creationTime = pk->getVoipTimestamp();
    packet.arrivalTime = simTime();
    packets.push_back(packet);
}

SimpleVoIPReceiver::SimpleVoIPReceiver()
{
    selfTalkspurtFinished = NULL;
}

SimpleVoIPReceiver::~SimpleVoIPReceiver()
{
    cancelAndDelete(selfTalkspurtFinished);
    playoutQueue.clear();
}

void SimpleVoIPReceiver::initialize(int stage)
{
    if (stage != 3)
        return;

    emodel_Ie = par("emodel_Ie");
    emodel_Bpl = par("emodel_Bpl");
    emodel_A = par("emodel_A");
    emodel_Ro = par("emodel_Ro");

    bufferSpace = par("bufferSpace");
    playoutDelay = par("playoutDelay");

    int port = par("localPort");
    EV << "VoIPReceiver::initialize - binding to port: local:" << port << endl;
    if (port != -1) {
            socket.setOutputGate(gate("udpOut"));
            socket.bind(port);
    }

    currentTalkspurt.talkspurtID = -1;
    selfTalkspurtFinished = new cMessage("selfTalkspurtFinished");

    packetLossRateSignal = registerSignal("VoIPPacketLossRate");
    packetDelaySignal = registerSignal("VoIPPacketDelay");
    playoutDelaySignal = registerSignal("VoIPPlayoutDelay");
    playoutLossRateSignal = registerSignal("VoIPPlayoutLossRate");
    mosSignal = registerSignal("VoIPMosSignal");
    taildropLossRateSignal = registerSignal("VoIPTaildropLossRate");
}

void SimpleVoIPReceiver::startTalkspurt(SimpleVoIPPacket* packet)
{
    currentTalkspurt.startTalkspurt(packet);
    //TODO Should be use spare time at the end of talkspurt?
    simtime_t endTime = simTime() + playoutDelay + (currentTalkspurt.talkspurtNumPackets - packet->getPacketID()) * currentTalkspurt.voiceDuration;
    scheduleAt(endTime, selfTalkspurtFinished);
}

void SimpleVoIPReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    { // selfTalkspurtFinished
        evaluateTalkspurt(false);
        return;
    }

    SimpleVoIPPacket* packet = dynamic_cast<SimpleVoIPPacket*>(msg);
    if (packet==NULL) {
        EV << "VoIPReceiver: Unknown incoming message: " << msg->getClassName() << endl;
        delete msg;
        return;
    }

    if (currentTalkspurt.status == TalkspurtInfo::EMPTY)
    {   // first talkspurt
        startTalkspurt(packet);
    }
    else if (packet->getTalkspurtID() > currentTalkspurt.talkspurtID)
    {   // old talkspurt finished, new talkspurt started
        if (currentTalkspurt.isActive())
        {
            cancelEvent(selfTalkspurtFinished);
            evaluateTalkspurt(false);
        }
        startTalkspurt(packet);
    }
    else if (currentTalkspurt.talkspurtID == packet->getTalkspurtID() && currentTalkspurt.status == TalkspurtInfo::ACTIVE)
    {   // talkspurt continued
        if (!currentTalkspurt.checkPacket(packet))
            throw cRuntimeError("Talkspurt parameters not equals");
        currentTalkspurt.addPacket(packet);
    }
    else
    {
        // packet from older talkspurt, ignore
        EV << "PACKET LATE ARRIVED: TALKSPURT " << packet->getTalkspurtID() << " PACKET " << packet->getPacketID() << ", IGNORED\n\n";
        delete msg;
        return;
    }

    EV << "PACKET ARRIVED: TALKSPURT " << packet->getTalkspurtID() << " PACKET " << packet->getPacketID() << "\n\n";

    simtime_t delay = packet->getArrivalTime() - packet->getVoipTimestamp();
    emit(packetDelaySignal, delay);

    delete msg;
}

void SimpleVoIPReceiver::evaluateTalkspurt(bool finish)
{
    ASSERT(!currentTalkspurt.packets.empty());
    ASSERT(currentTalkspurt.isActive());

    VoIPPacketInfo firstPacket = currentTalkspurt.packets.front();

    simtime_t    firstPlayoutTime = firstPacket.arrivalTime + playoutDelay;
    simtime_t    mouthToEarDelay = firstPlayoutTime - firstPacket.creationTime;
    unsigned int firstPacketId = firstPacket.packetID;
    unsigned int talkspurtNumPackets = currentTalkspurt.talkspurtNumPackets;
    unsigned int playoutLoss = 0;
    unsigned int tailDropLoss = 0;
    unsigned int channelLoss;

    if (finish)
    {
        PacketsVector::iterator it;
        unsigned int maxId = 0;
        for ( it = currentTalkspurt.packets.begin(); it != currentTalkspurt.packets.end(); it++)
            maxId = std::max(maxId, (*it).packetID);
        channelLoss = maxId + 1 - currentTalkspurt.packets.size();
    }
    else
        channelLoss = currentTalkspurt.talkspurtNumPackets - currentTalkspurt.packets.size();
    //FIXME Translate: a duplikalt packetek elfednek egy-egy elveszett packetet a fenti channelLoss szamitasban, ezt korrigaljuk lejjebb a duplikalt packet detektalasnal.

    double packetLossRate = ((double)channelLoss/(double)talkspurtNumPackets);
    emit(packetLossRateSignal, packetLossRate);

    //VECTOR TO MANAGE DUPLICATED PACKETS
    bool* isArrived = new bool[talkspurtNumPackets];
    for (unsigned int y = 0; y < talkspurtNumPackets; y++)
    {
        isArrived[y] = false;
    }

    simtime_t last_lateness = -playoutDelay;     // arrival time - playout time
    simtime_t max_lateness = -playoutDelay;

    // FIXME: what is the idea here? what does it compute? from what data does it compute? write something about the algorithm
    for ( PacketsVector::iterator packet = currentTalkspurt.packets.begin(); packet != currentTalkspurt.packets.end(); ++packet)
    {
        packet->playoutTime = (firstPlayoutTime + ((int)packet->packetID - (int)firstPacketId) * currentTalkspurt.voiceDuration);

        // FIXME: is this really a jitter? positive means the packet is too late
        last_lateness = packet->arrivalTime - packet->playoutTime;
        if (max_lateness < last_lateness)
            max_lateness = last_lateness;

        EV << "MEASURED PACKET JITTER: " << last_lateness << " TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";     //FIXME Jitter???

        //MANAGEMENT OF DUPLICATED PACKETS
        if (isArrived[packet->packetID])
        {
            ++channelLoss; // a duplikalt packetek elfednek egy-egy elveszett packetet a fenti channelLoss szamitasban, ezt korrigaljuk itt. //FIXME Translate
            EV << "DUPLICATED PACKET: TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";
        }
        else if (last_lateness > 0.0)
        {
            ++playoutLoss;
            EV << "LATE PACKAGE REMOVED: TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";
        }
        else
        {
            // FIXME: is this the place where we actually play the packets?
            // remove packets from queue
            PacketsList::iterator qi=playoutQueue.begin();
            while (qi != playoutQueue.end())
            {
                if ((*qi)->playoutTime < packet->arrivalTime)
                {
                    // EV << "REPRODUCED AND EXTRACT FROM BUFFER: TALK " << currentTalkspurt.talkspurtID << " PACKET " << (*qi)->packetID << "\n";
                    qi = playoutQueue.erase(qi);
                }
                else
                    ++qi;
            }

            if (playoutQueue.size() < bufferSpace)
            {
                EV << "PACKET INSERTED TO PLAYOUT BUFFER: TALK "
                        << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID
                        << ", MOMENT OF ARRIVAL " << packet->arrivalTime
                        << "s, MOMENT OF PLAYOUT " << packet->playoutTime << "s\n\n";
                //MANAGEMENT OF DUPLICATED PACKETS
                isArrived[packet->packetID] = true;

                playoutQueue.push_back(&(*packet));
            }
            else
            {   // buffer full
                ++tailDropLoss;
                EV << "BUFFER FULL, PACKET DISCARDED: TALK " << currentTalkspurt.talkspurtID << " PACKET "
                        << packet->packetID << " MOMENT OF ARRIVAL " << packet->arrivalTime << "s\n\n";     //FIXME Translate!!!
            }
        }
    }

    double proportionalLossRate = (double)(tailDropLoss+playoutLoss+channelLoss) / (double)talkspurtNumPackets;
    EV << "proportionalLossRate " << proportionalLossRate << "(tailDropLoss=" << tailDropLoss << " - playoutLoss="
            << playoutLoss << " - channelLoss=" << channelLoss << ")\n\n";

    double mos = eModel(SIMTIME_DBL(mouthToEarDelay), proportionalLossRate);

    emit(playoutDelaySignal, playoutDelay);
    double lossRate = ((double)playoutLoss/(double)talkspurtNumPackets);
    emit(playoutLossRateSignal, lossRate);
    emit(mosSignal, mos);

    // add calculated mos value to fingerprint
    cHasher *hasher = simulation.getHasher();
    if (hasher)
        hasher->add(mos);

    double tailDropRate = ((double)tailDropLoss/(double)talkspurtNumPackets);
    emit(taildropLossRateSignal, tailDropRate);

    EV << "CALCULATED MOS: eModel( " << playoutDelay << " , " << tailDropLoss << "+" << playoutLoss << "+" << channelLoss << " ) = " << mos << "\n\n";

    EV << "PLAYOUT DELAY ADAPTATION \n" << "OLD PLAYOUT DELAY: " << playoutDelay << "\nMAX JITTER MEASURED: " << max_lateness << "\n\n"; //FIXME JITTER???

    playoutDelay += max_lateness;
    if (playoutDelay < 0.0)
        playoutDelay = 0.0;
    EV << "NEW PLAYOUT DELAY: " << playoutDelay << "\n\n";

    delete [] isArrived;
    playoutQueue.clear();
    currentTalkspurt.finishTalkspurt();
}

// The E Model was originally developed within ETSI as a transmission planning tool,
// described in ETSI technical report ETR 250, and then standardized by the ITU as G.107.
// The objective of the model was to determine a quality rating that incorporated the
// "mouth to ear" characteristics of a speech path.
double SimpleVoIPReceiver::eModel(double delay, double lossRate)
{
    static const double alpha3 = 177.3; //ms
    double delayms = 1000.0 * delay;

    // Compute the Id parameter
    int u = (delayms - alpha3) > 0 ? 1 : 0;
    double id = 0.024 * delayms + 0.11 * (delayms - alpha3) * u;

    // p: Packet loss rate in %
    double p = lossRate * 100;
    // Compute the Ie,eff parameter
    double ie_eff = emodel_Ie + (95 - emodel_Ie) * p / (p + emodel_Bpl);

    // Compute the R factor
    double Rfactor = emodel_Ro - id - ie_eff + emodel_A;

    // Compute the MOS value
    double mos = 0.0;

    if (Rfactor < 0.0) {
        mos = 1.0;
    }
    else if (Rfactor > 100.0) {
        mos = 4.5;
    }
    else {
        mos = 1.0 + 0.035 * Rfactor + 7.0 * 1E-6 * Rfactor * (Rfactor - 60.0) * (100.0 - Rfactor);
    }

    mos = (mos < 1.0) ? 1.0 : mos;

    return mos;
}

void SimpleVoIPReceiver::finish()
{
    // evaluate last talkspurt
    cancelEvent(selfTalkspurtFinished);
    if (currentTalkspurt.isActive())
        evaluateTalkspurt(true);
}

