/*
 * VoIPReceiver.cc
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */

#include "SimpleVoIPReceiver.h"

#include "SimpleVoIPPacket_m.h"

Define_Module(SimpleVoIPReceiver);

void SimpleVoIPReceiver::TalkspurtInfo::startTalkspurt(SimpleVoIPPacket *pk)
{
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

SimpleVoIPReceiver::~SimpleVoIPReceiver()
{
    while (!playoutQueue.empty())
    {
        delete playoutQueue.front();
        playoutQueue.pop_front();
    }
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

    packetLossRateSignal = registerSignal("VoIPPacketLossRate");
    packetDelaySignal = registerSignal("VoIPPacketDelay");
    playoutDelaySignal = registerSignal("VoIPPlayoutDelay");
    playoutLossRateSignal = registerSignal("VoIPPlayoutLossRate");
    mosSignal = registerSignal("VoIPMosSignal");
    taildropLossRateSignal = registerSignal("VoIPTaildropLossRate");
}

void SimpleVoIPReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        throw cRuntimeError("Unaccepted self message: '%s'", msg->getName());
    }

    SimpleVoIPPacket* packet = dynamic_cast<SimpleVoIPPacket*>(msg);
    if (packet==NULL) {
        EV << "VoIPReceiver: Unknown incoming message: " << msg->getClassName() << endl;
        delete msg;
        return;
    }

    if (currentTalkspurt.packets.empty())
    {   // first packet
        currentTalkspurt.startTalkspurt(packet);
    }
    else if (currentTalkspurt.talkspurtID == packet->getTalkspurtID())
    {   // talkspurt continued
        if (!currentTalkspurt.checkPacket(packet))
            throw cRuntimeError("Talkspurt parameters not equals");
        currentTalkspurt.addPacket(packet);
    }
    else
    {   // old talkspurt finished, new talkspurt started
        evaluateTalkspurt(false);
        currentTalkspurt.startTalkspurt(packet);
    }

    //emit(mPacketLossSignal,1.0);

    EV << "PACKET ARRIVED: TALKSPURT " << packet->getTalkspurtID() << " PACKET " << packet->getPacketID() << "\n\n";

    simtime_t delay = packet->getArrivalTime() - packet->getVoipTimestamp();
    emit(packetDelaySignal, delay);

    delete msg;
}

// FIXME: this should rather be called evaluateTalkspurt, because all it does is that it gathers some statistics
void SimpleVoIPReceiver::evaluateTalkspurt(bool finish)
{
    ASSERT(!currentTalkspurt.packets.empty());

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
    //FIXME Translate:  a duplikalt packetek elfednek egy-egy elveszett packetet a fenti channelLoss szamitasban, ezt korrigaljuk lejjebb a duplikalt packet detektalasnal.

    double packetLossRate = ((double)channelLoss/(double)talkspurtNumPackets);
    emit(packetLossRateSignal, packetLossRate);

    //VETTORE PER GESTIRE DUPLICATI     //FIXME Translate!!!
    // FIXME: what is actually arrived here?
    bool* isArrived = new bool[talkspurtNumPackets];
    for (unsigned int y = 0; y < talkspurtNumPackets; y++)
    {
        isArrived[y] = false;
    }

    simtime_t last_jitter = 0.0;
    simtime_t max_jitter = -1000.0;

    // FIXME: what is the idea here? what does it compute? from what data does it compute? write something about the algorithm
    for ( PacketsVector::iterator packet = currentTalkspurt.packets.begin(); packet != currentTalkspurt.packets.end(); ++packet)
    {
        packet->playoutTime = (firstPlayoutTime + ((int)packet->packetID - (int)firstPacketId) * currentTalkspurt.voiceDuration);

        // FIXME: is this really a jitter? positive means the packet is too late
        last_jitter = packet->arrivalTime - packet->playoutTime;
        max_jitter = std::max(max_jitter, last_jitter);

        EV << "MISURATO JITTER PACCHETTO: " << last_jitter << " TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";     //FIXME Translate!!!

        //GESTIONE IN CASO DI DUPLICATI     //FIXME Translate!!!
        if (isArrived[packet->packetID])
        {
            ++channelLoss; // a duplikalt packetek elfednek egy-egy elveszett packetet a fenti channelLoss szamitasban, ezt korrigaljuk itt.
            EV << "PACCHETTO DUPLICATO: TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";     //FIXME Translate!!!
        }
        else if (last_jitter > 0.0)
        {
            ++playoutLoss;
            EV << "PACCHETTO IN RITARDO ELIMINATO: TALK " << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID << "\n\n";     //FIXME Translate!!!
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
                    //EV << "RIPRODOTTO ED ESTRATTO DAL BUFFER: TALK " << mPlayoutQueue.front()->getTalkspurtID() << " PACKET " << mPlayoutQueue.front()->packetID << "\n";     //FIXME Translate!!!
                    qi = playoutQueue.erase(qi);
                }
                else
                    ++qi;
            }

            if (playoutQueue.size() < bufferSpace)
            {
                EV << "PACCHETTO CAMPIONABILE INSERITO NEL BUFFER: TALK "
                        << currentTalkspurt.talkspurtID << " PACKET " << packet->packetID
                        << " ISTANTE DI ARRIVO " << packet->arrivalTime
                        << " ISTANTE DI CAMPIONAMENTO " << packet->playoutTime << "\n\n";     //FIXME Translate!!!
                //GESTIONE DUPLICATI     //FIXME Translate!!!
                isArrived[packet->packetID] = true;

                playoutQueue.push_back(&(*packet));
            }
            else
            {   // buffer full
                ++tailDropLoss;
                EV << "BUFFER PIENO PACCHETTO SCARTATO: TALK " << currentTalkspurt.talkspurtID << " PACKET "
                        << packet->packetID << " ISTANTE DI ARRIVO " << packet->arrivalTime << "\n\n";     //FIXME Translate!!!
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

    // add calculated mos to fingerprint
    cHasher *hasher = simulation.getHasher();
    if (hasher)
        hasher->add(mos);

    double tailDropRate = ((double)tailDropLoss/(double)talkspurtNumPackets);
    emit(taildropLossRateSignal, tailDropRate);

    EV << "CALCULATED MOS: eModel( " << playoutDelay << " , " << tailDropLoss << "+" << playoutLoss << "+" << channelLoss << " ) = " << mos << "\n\n";

    EV << "PLAYOUT DELAY ADAPTATION \n" << "OLD PLAYOUT DELAY: " << playoutDelay << "\nMAX JITTER MEASURED: " << max_jitter << "\n\n";

    playoutDelay += max_jitter;
    if (playoutDelay < 0.0)
        playoutDelay = 0.0;
    EV << "NEW PLAYOUT DELAY: " << playoutDelay << "\n\n";

    delete [] isArrived;
    playoutQueue.clear();
    currentTalkspurt.packets.clear();
}

// e-model described in ITU-T G.107 standard
double SimpleVoIPReceiver::eModel(double delay, double lossRate)
{
    static const double alpha3 = 177.3; //ms
    double delayms = 1000.0 * delay;

    // FIXME: useless comment
    // Compute the Id parameter
    int u = (delayms - alpha3) > 0 ? 1 : 0;
    double id = 0.024 * delayms + 0.11 * (delayms - alpha3) * u;

    // Packet loss p in %
    double p = lossRate * 100;
    // Compute the Ie,eff parameter
    double ie_eff = emodel_Ie + (95 - emodel_Ie) * p / (p + emodel_Bpl);

    // FIXME: useless comment
    // Compute the R factor
    double Rfactor = emodel_Ro - id - ie_eff + emodel_A;

    // FIXME: useless comment
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
    if (!currentTalkspurt.packets.empty())
        evaluateTalkspurt(true);
}

