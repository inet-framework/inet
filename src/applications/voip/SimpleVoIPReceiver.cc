/*
 * VoIPReceiver.cc
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */

#include "SimpleVoIPReceiver.h"

#include "SimpleVoIPPacket_m.h"

Define_Module(SimpleVoIPReceiver);

SimpleVoIPReceiver::~SimpleVoIPReceiver()
{
    while (!playoutQueue.empty())
    {
        delete playoutQueue.front();
        playoutQueue.pop_front();
    }

    while (!packetsList.empty())
    {
        delete packetsList.front();
        packetsList.pop_front();
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

    packetLossRateSignal = registerSignal("VoIPPacketLossRate");
    packetDelaySignal = registerSignal("VoIPPacketDelay");
    playoutDelaySignal = registerSignal("VoIPPlayoutDelay");
    playoutLossRateSignal = registerSignal("VoIPPlayoutLossRate");
    mosSignal = registerSignal("VoIPMosSignal");
    taildropLossRateSignal = registerSignal("VoIPTaildropLossRate");

    taggedSample = new TaggedSample();
    taggedSample->module = this;
    taggedSample->id = getId();
}

void SimpleVoIPReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        throw cRuntimeError("Unaccepted self message: '%s'", msg->getName());
    }

    SimpleVoIPPacket* pPacket = dynamic_cast<SimpleVoIPPacket*>(msg);
    if (pPacket==0) {
        // FIXME: it should rather say unknown incoming message type (not VoipPacket)
        EV << "VoIPReceiver: Unaccepted incoming message: " << msg->getName() << endl;
        delete msg;
        return;
    }

    // FIXME add a check: the voiceDuration value does not change in same talkspurt

    if (currentTalkspurt != pPacket->getTalkspurtID())
    {
        evaluateTalkspurt(false);
        currentTalkspurt = pPacket->getTalkspurtID();
    }

    //emit(mPacketLossSignal,1.0);

    EV << "PACCHETTO ARRIVATO: TALK " << pPacket->getTalkspurtID() << " PACKET " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!

    // FXIME: maybe the simulation kernel got it wrong? this is a useless assert
    ASSERT(pPacket->getArrivalTime() == simTime());
    simtime_t delay = pPacket->getArrivalTime() - pPacket->getVoipTimestamp();
    emit(packetDelaySignal, delay);
    packetsList.push_back(pPacket);
}

// FIXME: this should rather be called evaluateTalkspurt, because all it does is that it gathers some statistics
void SimpleVoIPReceiver::evaluateTalkspurt(bool finish)
{
    if (packetsList.empty())
        return;

    SimpleVoIPPacket* pPacket = packetsList.front();

    simtime_t    firstPlayoutTime = pPacket->getArrivalTime() + playoutDelay;
    unsigned int firstPacketId = pPacket->getPacketID();
    unsigned int talkspurtNumPackets = pPacket->getTalkspurtNumPackets();
    unsigned int playoutLoss = 0;
    unsigned int tailDropLoss = 0;
    unsigned int channelLoss;

    if (finish)
    {
        PacketsList::iterator it;
        unsigned int maxId = 0;
        for ( it = packetsList.begin(); it != packetsList.end(); it++)
            maxId = std::max(maxId, (*it)->getPacketID());
        channelLoss = maxId + 1 - packetsList.size();
    }
    else
        channelLoss = pPacket->getTalkspurtNumPackets() - packetsList.size();

    double packetLossRate = ((double)channelLoss/(double)talkspurtNumPackets);
    emit(packetLossRateSignal, packetLossRate);

    //VETTORE PER GESTIRE DUPLICATI     //FIXME Translate!!!
    // FIXME: what is actually arrived here?
    bool* isArrived = new bool[pPacket->getTalkspurtNumPackets()];
    for (unsigned int y = 0; y < pPacket->getTalkspurtNumPackets(); y++)
    {
        isArrived[y] = false;
    }

    simtime_t last_jitter = 0.0;
    simtime_t max_jitter = -1000.0;

    // FIXME: what is the idea here? what does it compute? from what data does it compute? write something about the algorithm
    while (!packetsList.empty() /*&& pPacket->getTalkID() == mCurrentTalkspurt*/)
    {
        pPacket = packetsList.front();
        // FIXME: why do we modify a packet in the receiver?
        pPacket->setPlayoutTime(firstPlayoutTime + ((int)pPacket->getPacketID() - (int)firstPacketId)  * pPacket->getVoiceDuration());

        // FIXME: is this really a jitter? positive means the packet is too late
        last_jitter = pPacket->getArrivalTime() - pPacket->getPlayoutTime();
        max_jitter = std::max( max_jitter, last_jitter );

        EV << "MISURATO JITTER PACCHETTO: " << last_jitter << " TALK " << pPacket->getTalkspurtID() << " PACKET " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!

        //GESTIONE IN CASO DI DUPLICATI     //FIXME Translate!!!
        if (isArrived[pPacket->getPacketID()])
        {
            EV << "PACCHETTO DUPLICATO: TALK " << pPacket->getTalkspurtID() << " PACKET " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!
            delete pPacket;
        }
        else if (last_jitter > 0.0)
        {
            ++playoutLoss;
            EV << "PACCHETTO IN RITARDO ELIMINATO: TALK " << pPacket->getTalkspurtID() << " PACKET " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!
            delete pPacket;
        }
        else
        {
            // FIXME: is this the place where we actually play the packets?
            while (!playoutQueue.empty() && pPacket->getArrivalTime() > playoutQueue.front()->getPlayoutTime())
            {
                ++bufferSpace;
                //EV << "RIPRODOTTO ED ESTRATTO DAL BUFFER: TALK " << mPlayoutQueue.front()->getTalkspurtID() << " PACKET " << mPlayoutQueue.front()->getPacketID() << "\n";     //FIXME Translate!!!
                delete playoutQueue.front();
                playoutQueue.pop_front();
            }

            if (bufferSpace > 0)
            {
                EV << "PACCHETTO CAMPIONABILE INSERITO NEL BUFFER: TALK "
                        << pPacket->getTalkspurtID() << " PACKET " << pPacket->getPacketID()
                        << " ISTANTE DI ARRIVO " << pPacket->getArrivalTime()
                        << " ISTANTE DI CAMPIONAMENTO " << pPacket->getPlayoutTime() << "\n\n";     //FIXME Translate!!!
                --bufferSpace;
                //GESTIONE DUPLICATI     //FIXME Translate!!!
                isArrived[pPacket->getPacketID()] = true;

                playoutQueue.push_back(pPacket);
            }
            else
            {
                ++tailDropLoss;
                EV << "BUFFER PIENO PACCHETTO SCARTATO: TALK " << pPacket->getTalkspurtID() << " PACKET "
                        << pPacket->getPacketID() << " ISTANTE DI ARRIVO " << pPacket->getArrivalTime() << "\n\n";     //FIXME Translate!!!
                delete pPacket;
            }
        }

        packetsList.pop_front();
    }

    double proportionalLossRate = ((double)tailDropLoss+(double)playoutLoss+(double)channelLoss)/(double)talkspurtNumPackets;
    EV << "proportionalLossRate " << proportionalLossRate << "(tailDropLoss=" << tailDropLoss << " - playoutLoss="
            << playoutLoss << " - channelLoss=" << channelLoss << ")\n\n";

    double mos = eModel(SIMTIME_DBL(playoutDelay), proportionalLossRate);

    emit(playoutDelaySignal, playoutDelay);
    double lossRate = ((double)playoutLoss/(double)talkspurtNumPackets);
    emit(playoutLossRateSignal, lossRate);
    emit(mosSignal, mos);
    double tailDropRate = ((double)tailDropLoss/(double)talkspurtNumPackets);
    emit(taildropLossRateSignal, tailDropRate);

    EV << "CALCULATED MOS: eModel( " << playoutDelay << " , " << tailDropLoss << "+" << playoutLoss << "+" << channelLoss << " ) = " << mos << "\n\n";

    EV << "PLAYOUT DELAY ADAPTATION \n" << "OLD PLAYOUT DELAY: " << playoutDelay << "\nMAX JITTER MESEAURED: " << max_jitter << "\n\n";

    playoutDelay += max_jitter;
    if (playoutDelay < 0.0)
        playoutDelay = 0.0;
    EV << "NEW PLAYOUT DELAY: " << playoutDelay << "\n\n";

    delete [] isArrived;
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
    evaluateTalkspurt(true);
}

