/*
 * VoIPReceiver.cc
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */
#include "VoIPReceiver.h"

Define_Module(VoIPReceiver);

VoIPReceiver::~VoIPReceiver()
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

void VoIPReceiver::initialize(int stage)
{
	if (stage!=3)
		return;

	emodel_Ie_  = par("emodel_Ie_");
	emodel_Bpl_ = par("emodel_Bpl_");
	emodel_A_   = par("emodel_A_");
	emodel_Ro_  = par("emodel_Ro_");

	mBufferSpace   = par("dim_buffer");
	mSamplingDelta = par("sampling_time");
	mPlayoutDelay  = par("playout_delay");

	mInit = true;

	int port = par("localPort");
	EV << "VoIPReceiver::initialize - binding to port: local:" << port << endl;
	if (port!=-1) {
	        socket.setOutputGate(gate("udpOut"));
	        socket.bind(port);
	}

	mFrameLossSignal    = registerSignal("VoIPFrameLoss");
	mFrameDelaySignal   = registerSignal("VoIPFrameDelay");
	mPlayoutDelaySignal = registerSignal("VoIPPlayoutDelay");
	mPlayoutLossSignal  = registerSignal("VoIPPlayoutLoss");
	mMosSignal			= registerSignal("VoIPMosSignal");
	mTaildropLossSignal	= registerSignal("VoIPTaildropLoss");

	mTaggedSample = new TaggedSample();
	mTaggedSample->module = check_and_cast<cComponent*>(this);;
	mTaggedSample->id = this->getId();
}

void VoIPReceiver::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
		return;
	VoipPacket* pPacket = dynamic_cast<VoipPacket*>(msg);
	cPacket* pPacket2;
	if (pPacket==0){
		pPacket2 = dynamic_cast<cPacket*>(msg);
	}

	if(mInit)
	{
		mCurrentTalkspurt = pPacket->getIDtalk();
		mInit = false;
	}

	if(mCurrentTalkspurt != pPacket->getIDtalk())
	{
			playout(false);
			mCurrentTalkspurt = pPacket->getIDtalk();
	}

	//emit(mFrameLossSignal,1.0);

	std::cout<< simTime() << ")PACCHETTO ARRIVATO: TALK "<<pPacket->getIDtalk()<<" FRAME "<<pPacket->getIDframe()<<"\n\n";
	EV<<"PACCHETTO ARRIVATO: TALK "<<pPacket->getIDtalk()<<" FRAME "<<pPacket->getIDframe()<<"\n\n";

	pPacket->setArrivalTime(SIMTIME_DBL(simTime()));
	mPacketsList.push_back(pPacket);
}

void VoIPReceiver::playout(bool finish)
{
	std::cout << "PLAYOUT" << endl;
	if(mPacketsList.empty())
		return;

	VoipPacket* pPacket =  mPacketsList.front();

    double       firstPlayoutTime   = pPacket->getArrivalTime() + mPlayoutDelay;
    unsigned int firstFrameId       = pPacket->getIDframe();
    unsigned int n_frames		    = pPacket->getNframes();
    unsigned int playoutLoss 	    = 0;
    unsigned int tailDropLoss       = 0;
    unsigned int channelLoss;

    if (finish)
    {
    	PacketsList::iterator it;
    	unsigned int maxId = 0;
    	for( it = mPacketsList.begin(); it != mPacketsList.end(); it++)
    		maxId = std::max(maxId, (*it)->getIDframe());
    		channelLoss = maxId + 1 - mPacketsList.size();
    }

    else
    	channelLoss = pPacket->getNframes() - mPacketsList.size();

    double frameLossRate = ((double)channelLoss/(double)n_frames);
    emit(mFrameLossSignal, frameLossRate);

    //VETTORE PER GESTIRE DUPLICATI
    bool* isArrived = new bool[pPacket->getNframes()];
    for(unsigned int y = 0; y < pPacket->getNframes(); y++)
    {
    	isArrived[y] = false;
    }

    double       last_jitter 	    = 0.0;
    double       max_jitter 		= -1000.0;

	while( !mPacketsList.empty() /*&& pPacket->getIDtalk() == mCurrentTalkspurt*/ )
	{
		pPacket =  mPacketsList.front();

		double delay = pPacket->getArrivalTime()-SIMTIME_DBL(pPacket->getTimestamp());
		emit(mFrameDelaySignal, delay);

		pPacket->setPlayoutTime(firstPlayoutTime + (pPacket->getIDframe() - firstFrameId)  * mSamplingDelta);

		last_jitter = pPacket->getArrivalTime() - pPacket->getPlayoutTime();
		max_jitter  = std::max( max_jitter, last_jitter );

		std::cout<<"MISURATO JITTER PACCHETTO: "<<last_jitter<<" TALK "<<pPacket->getIDtalk()<<" FRAME "
				<<pPacket->getIDframe()<<"\n\n";

		EV<<"MISURATO JITTER PACCHETTO: "<<last_jitter<<" TALK "<<pPacket->getIDtalk()<<" FRAME "
				<<pPacket->getIDframe()<<"\n\n";

		//GESTIONE IN CASO DI DUPLICATI
		if(isArrived[pPacket->getIDframe()])
		{
					EV<<"PACCHETTO DUPLICATO: TALK "<<pPacket->getIDtalk()<<" FRAME "
						<<pPacket->getIDframe()<<"\n\n";
					delete pPacket;
		}
		else if( last_jitter > 0.0 )
		{
			++playoutLoss;
			std::cout<<"PACCHETTO IN RITARDO ELIMINATO: TALK "<<pPacket->getIDtalk()<<" FRAME "
					<<pPacket->getIDframe()<<"\n\n";
			EV<<"PACCHETTO IN RITARDO ELIMINATO: TALK "<<pPacket->getIDtalk()<<" FRAME "
					<<pPacket->getIDframe()<<"\n\n";
			delete pPacket;
		}
		else
		{
			while( !mPlayoutQueue.empty() && pPacket->getArrivalTime() > mPlayoutQueue.front()->getPlayoutTime() )
			{
				++mBufferSpace;
				//EV<<"RIPRODOTTO ED ESTRATTO DAL BUFFER: TALK "<<mPlayoutQueue.front()->getIDtalk()<<" FRAME "<<mPlayoutQueue.front()->getIDframe()<<"\n";
//				std::cout<<"RIPRODOTTO ED ESTRATTO DAL BUFFER: TALK "<<mPlayoutQueue.front()->getIDtalk()
//						<<" FRAME "<<mPlayoutQueue.front()->getIDframe()<<"\n";
				delete mPlayoutQueue.front();
				mPlayoutQueue.pop_front();
			}

			if(mBufferSpace > 0)
			{
				std::cout<<"PACCHETTO CAMPIONABILE INSERITO NEL BUFFER: TALK "
						<<pPacket->getIDtalk()<<" FRAME "<<pPacket->getIDframe()
						<<" ISTANTE DI ARRIVO "<<pPacket->getArrivalTime()
						<<" ISTANTE DI CAMPIONAMENTO "<<pPacket->getPlayoutTime()<<"\n\n";

				EV<<"PACCHETTO CAMPIONABILE INSERITO NEL BUFFER: TALK "
						<<pPacket->getIDtalk()<<" FRAME "<<pPacket->getIDframe()
						<<" ISTANTE DI ARRIVO "<<pPacket->getArrivalTime()
						<<" ISTANTE DI CAMPIONAMENTO "<<pPacket->getPlayoutTime()<<"\n\n";
				--mBufferSpace;
				//GESTIONE DUPLICATI
				isArrived[pPacket->getIDframe()] = true;

				mPlayoutQueue.push_back(pPacket);
			}
			else
			{
				++tailDropLoss;
				std::cout<<"BUFFER PIENO PACCHETTO SCARTATO: TALK "<<pPacket->getIDtalk()<<" FRAME "
						<<pPacket->getIDframe()<<" ISTANTE DI ARRIVO "<<pPacket->getArrivalTime()<<"\n\n";
				EV<<"BUFFER PIENO PACCHETTO SCARTATO: TALK "<<pPacket->getIDtalk()<<" FRAME "
						<<pPacket->getIDframe()<<" ISTANTE DI ARRIVO "<<pPacket->getArrivalTime()<<"\n\n";
				delete pPacket;
			}
		}

		mPacketsList.pop_front();
	}

	double proportionalLoss = ((double)tailDropLoss+(double)playoutLoss+(double)channelLoss)/(double)n_frames;
		EV<<"proportionalLoss "<<proportionalLoss<< "(tailDropLoss="<< tailDropLoss<< " - playoutLoss="<<
				playoutLoss << " - channelLoss=" << channelLoss<< ")\n\n";

	double mos = eModel(mPlayoutDelay, proportionalLoss);

	emit(mPlayoutDelaySignal, mPlayoutDelay);
	double lossRate = ((double)playoutLoss/(double)n_frames);
	emit(mPlayoutLossSignal, lossRate);
	emit(mMosSignal, mos);
	double tailDropRate = ((double)tailDropLoss/(double)n_frames);
	emit(mTaildropLossSignal, tailDropRate);

	EV<<"MOS CALCOLATO: eModel( "<<mPlayoutDelay<<" , "<<tailDropLoss<<"+"<<playoutLoss<<"+"
			<<channelLoss<<" ) = "<<mos<<"\n\n";

	EV<<"PLAYOUT DELAY ADAPTATION \n"<<"OLD PLAYOUT DELAY: "<<mPlayoutDelay<<"\nMAX JITTER MESEAURED: "
			<<max_jitter<<"\n\n";

	mPlayoutDelay += max_jitter;
	if(mPlayoutDelay < 0.0) mPlayoutDelay = 0.0;
	EV<<"NEW PLAYOUT DELAY: "<<mPlayoutDelay<<"\n\n";

	delete[] isArrived;
}

double VoIPReceiver::eModel (double delay, double loss)
{
		double delayms = 1000 * delay;

		// Compute the Id parameter
		int u = ( (delayms - 177.3) > 0 ? 1: 0 );
		double id = 0.0;
		id = 0.024 * delayms + 0.11 * (delayms - 177.3) * u;

		// Packet loss p in %
		double p = loss * 100;
		// Compute the Ie,eff parameter
		double ie_eff = emodel_Ie_ + (95 - emodel_Ie_) * p / (p + emodel_Bpl_);

		// Compute the R factor
		double Rfactor = emodel_Ro_ - id - ie_eff + emodel_A_;

		// Compute the MOS value
		double mos = 0.0;

		if (Rfactor < 0) {
			mos = 1.0;
		}
		else if (Rfactor > 100) {
			mos = 4.5;
		}
		else {
			mos = 1 + 0.035 * Rfactor + 7 * pow(10, (double)-6) * Rfactor *
			(Rfactor - 60) * (100 - Rfactor);
		}

		mos = ( mos < 1 ) ? 1 : mos;

		return mos;
}

void VoIPReceiver::finish()
{
	// last talkspurt playout
	playout(true);
}

