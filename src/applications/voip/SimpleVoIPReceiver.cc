/*
 * VoIPReceiver.cc
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */
#include "SimpleVoIPReceiver.h"

Define_Module(SimpleVoIPReceiver);

SimpleVoIPReceiver::~SimpleVoIPReceiver()
{
    while ( !mPlayoutQueue.empty() )
    {
        delete mPlayoutQueue.front();
        mPlayoutQueue.pop_front();
    }

    while ( !mPacketsList.empty() )
    {
        delete mPacketsList.front();
        mPacketsList.pop_front();
    }
}

void SimpleVoIPReceiver::initialize(int stage)
{
    if (stage!=3)
        return;

    emodel_Ie_ = par("emodel_Ie_");
    emodel_Bpl_ = par("emodel_Bpl_");
    emodel_A_ = par("emodel_A_");
    emodel_Ro_ = par("emodel_Ro_");

    mBufferSpace = par("dim_buffer");
    mPlayoutDelay = par("playoutDelay");

    mInit = true;

    int port = par("localPort");
    EV << "VoIPReceiver::initialize - binding to port: local:" << port << endl;
    if (port!=-1) {
            socket.setOutputGate(gate("udpOut"));
            socket.bind(port);
    }

    mFrameLossRateSignal = registerSignal("VoIPFrameLossRate");
    mFrameDelaySignal = registerSignal("VoIPFrameDelay");
    mPlayoutDelaySignal = registerSignal("VoIPPlayoutDelay");
    mPlayoutLossRateSignal = registerSignal("VoIPPlayoutLossRate");
    mMosSignal = registerSignal("VoIPMosSignal");
    mTaildropLossRateSignal = registerSignal("VoIPTaildropLossRate");

    mTaggedSample = new TaggedSample();
    mTaggedSample->module = this;
    mTaggedSample->id = getId();
}

void SimpleVoIPReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        throw cRuntimeError("Unaccepted self message: '%s'", msg->getName());
        // FIXME: there are no continuable exceptions in C++ (as opposed to in common lisp), so this is dead code
        delete msg;
        return;
    }
    SimpleVoIPPacket* pPacket = dynamic_cast<SimpleVoIPPacket*>(msg);
    if (pPacket==0) {
        // FIXME: it should rather say unknown incoming message type (not VoipPacket)
        EV << "VoIPReceiver: Unaccepted incoming message: " << msg->getName() << endl;
        delete msg;
        return;
    }

    // FIXME add a check: the voiceDuration value does not change in same talkspurt

    // FIXME: what does this mInit mean? does it mean initialized? if so, how could it be false here? and more importantly why do we set it back to false? this is confusing
    if (mInit)
    {
        mCurrentTalkspurt = pPacket->getTalkspurtID();
        // FIXME: so now we are uninitialized?
        mInit = false;
    }

    if (mCurrentTalkspurt != pPacket->getTalkspurtID())
    {
            playout(false);
            mCurrentTalkspurt = pPacket->getTalkspurtID();
    }

    //emit(mFrameLossSignal,1.0);

    EV << "PACCHETTO ARRIVATO: TALK " << pPacket->getTalkspurtID() << " FRAME " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!

    // FXIME: maybe the simulation kernel got it wrong? this is a useless assert
    ASSERT(pPacket->getArrivalTime() == simTime());
    simtime_t delay = pPacket->getArrivalTime() - pPacket->getVoipTimestamp();
    emit(mFrameDelaySignal, delay);
    mPacketsList.push_back(pPacket);
}

// FIXME: this should rather be called evaluateTalkspurt, because all it does is that it gathers some statistics
void SimpleVoIPReceiver::playout(bool finish)
{
    if (mPacketsList.empty())
        return;

    SimpleVoIPPacket* pPacket = mPacketsList.front();

    simtime_t    firstPlayoutTime = pPacket->getArrivalTime() + mPlayoutDelay;
    unsigned int firstFrameId = pPacket->getPacketID();
    unsigned int n_frames = pPacket->getTalkspurtNumPackets();
    unsigned int playoutLoss = 0;
    unsigned int tailDropLoss = 0;
    unsigned int channelLoss;

    if (finish)
    {
        PacketsList::iterator it;
        unsigned int maxId = 0;
        for ( it = mPacketsList.begin(); it != mPacketsList.end(); it++)
            maxId = std::max(maxId, (*it)->getPacketID());
        channelLoss = maxId + 1 - mPacketsList.size();
    }

    else
        channelLoss = pPacket->getTalkspurtNumPackets() - mPacketsList.size();

    double frameLossRate = ((double)channelLoss/(double)n_frames);
    emit(mFrameLossRateSignal, frameLossRate);

    //VETTORE PER GESTIRE DUPLICATI     //FIXME Translate!!!
    // FIXME: what is actually arrived here?
    bool* isArrived = new bool[pPacket->getTalkspurtNumPackets()];
    for (unsigned int y = 0; y < pPacket->getTalkspurtNumPackets(); y++)
    {
        isArrived[y] = false;
    }

    simtime_t       last_jitter = 0.0;
    simtime_t       max_jitter = -1000.0;

    // FIXME: what is the idea here? what does it compute? from what data does it compute? write something about the algorithm
    while ( !mPacketsList.empty() /*&& pPacket->getTalkID() == mCurrentTalkspurt*/ )
    {
        pPacket = mPacketsList.front();
        // FIXME: why do we modify a packet in the receiver?
        pPacket->setPlayoutTime(firstPlayoutTime + ((int)pPacket->getPacketID() - (int)firstFrameId)  * pPacket->getVoiceDuration());

        // FIXME: is this really a jitter? positive means the packet is too late
        last_jitter = pPacket->getArrivalTime() - pPacket->getPlayoutTime();
        max_jitter = std::max( max_jitter, last_jitter );

        EV << "MISURATO JITTER PACCHETTO: " << last_jitter << " TALK " << pPacket->getTalkspurtID() << " FRAME " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!

        //GESTIONE IN CASO DI DUPLICATI     //FIXME Translate!!!
        if (isArrived[pPacket->getPacketID()])
        {
                    EV << "PACCHETTO DUPLICATO: TALK " << pPacket->getTalkspurtID() << " FRAME " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!
                    delete pPacket;
        }
        else if ( last_jitter > 0.0 )
        {
            ++playoutLoss;
            EV << "PACCHETTO IN RITARDO ELIMINATO: TALK " << pPacket->getTalkspurtID() << " FRAME " << pPacket->getPacketID() << "\n\n";     //FIXME Translate!!!
            delete pPacket;
        }
        else
        {
            // FIXME: is this the place where we actually play the packets?
            while ( !mPlayoutQueue.empty() && pPacket->getArrivalTime() > mPlayoutQueue.front()->getPlayoutTime() )
            {
                ++mBufferSpace;
                //EV << "RIPRODOTTO ED ESTRATTO DAL BUFFER: TALK " << mPlayoutQueue.front()->getTalkspurtID() << " FRAME " << mPlayoutQueue.front()->getPacketID() << "\n";     //FIXME Translate!!!
                delete mPlayoutQueue.front();
                mPlayoutQueue.pop_front();
            }

            if (mBufferSpace > 0)
            {
                EV << "PACCHETTO CAMPIONABILE INSERITO NEL BUFFER: TALK "
                        << pPacket->getTalkspurtID() << " FRAME " << pPacket->getPacketID()
                        << " ISTANTE DI ARRIVO " << pPacket->getArrivalTime()
                        << " ISTANTE DI CAMPIONAMENTO " << pPacket->getPlayoutTime() << "\n\n";     //FIXME Translate!!!
                --mBufferSpace;
                //GESTIONE DUPLICATI     //FIXME Translate!!!
                isArrived[pPacket->getPacketID()] = true;

                mPlayoutQueue.push_back(pPacket);
            }
            else
            {
                ++tailDropLoss;
                EV << "BUFFER PIENO PACCHETTO SCARTATO: TALK " << pPacket->getTalkspurtID() << " FRAME "
                        << pPacket->getPacketID() << " ISTANTE DI ARRIVO " << pPacket->getArrivalTime() << "\n\n";     //FIXME Translate!!!
                delete pPacket;
            }
        }

        mPacketsList.pop_front();
    }

    double proportionalLoss = ((double)tailDropLoss+(double)playoutLoss+(double)channelLoss)/(double)n_frames;
    EV << "proportionalLoss " << proportionalLoss << "(tailDropLoss=" << tailDropLoss << " - playoutLoss="
            << playoutLoss << " - channelLoss=" << channelLoss << ")\n\n";

    double mos = eModel(SIMTIME_DBL(mPlayoutDelay), proportionalLoss);

    emit(mPlayoutDelaySignal, mPlayoutDelay);
    double lossRate = ((double)playoutLoss/(double)n_frames);
    emit(mPlayoutLossRateSignal, lossRate);
    emit(mMosSignal, mos);
    double tailDropRate = ((double)tailDropLoss/(double)n_frames);
    emit(mTaildropLossRateSignal, tailDropRate);

    EV << "MOS CALCOLATO: eModel( " << mPlayoutDelay << " , " << tailDropLoss << "+" << playoutLoss << "+" << channelLoss << " ) = " << mos << "\n\n";     //FIXME Translate!!!

    EV << "PLAYOUT DELAY ADAPTATION \n" << "OLD PLAYOUT DELAY: " << mPlayoutDelay << "\nMAX JITTER MESEAURED: " << max_jitter << "\n\n";

    mPlayoutDelay += max_jitter;
    if (mPlayoutDelay < 0.0) mPlayoutDelay = 0.0;
    EV << "NEW PLAYOUT DELAY: " << mPlayoutDelay << "\n\n";

    delete [] isArrived;
}

// FIXME: a reference to a paper, article, whatever that describes the model used here would be great!
double SimpleVoIPReceiver::eModel(double delay, double loss)
{
    double delayms = 1000.0 * delay;

    // FIXME: useless comment
    // Compute the Id parameter
    int u = ( (delayms - 177.3) > 0 ? 1: 0 );
    double id = 0.024 * delayms + 0.11 * (delayms - 177.3) * u;

    // Packet loss p in %
    double p = loss * 100;
    // Compute the Ie,eff parameter
    double ie_eff = emodel_Ie_ + (95 - emodel_Ie_) * p / (p + emodel_Bpl_);

    // FIXME: useless comment
    // Compute the R factor
    double Rfactor = emodel_Ro_ - id - ie_eff + emodel_A_;

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
        mos = 1.0 + 0.035 * Rfactor + 7.0 * 1E-6 * Rfactor *
                (Rfactor - 60.0) * (100.0 - Rfactor);
    }

    mos = ( mos < 1.0 ) ? 1.0 : mos;

    return mos;
}

void SimpleVoIPReceiver::finish()
{
    // last talkspurt playout
    playout(true);
}

