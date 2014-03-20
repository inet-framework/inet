#include "UWBIRRadio.h"

#include <cassert>

#include "DeciderResultUWBIR.h"
#include "DeciderUWBIREDSyncOnAddress.h"
#include "DeciderUWBIREDSync.h"
#include "UWBIRRadioFrame_m.h"
#include "UWBIRStochasticPathlossModel.h"
#include "UWBIRIEEE802154APathlossModel.h"

Define_Module(UWBIRRadio);

//t_dynamic_expression_value (PhyLayerUWBIR::*ghassemzadehNLOSFPtr) (cComponent *context, t_dynamic_expression_value argv[], int argc) = &ghassemzadehNLOSFunc;
UWBIRRadio::fptr ghassemzadehNLOSFPtr = &UWBIRRadio::ghassemzadehNLOSFunc;
Define_NED_Function(ghassemzadehNLOSFPtr, "xml ghassemzadehNLOS()");

void UWBIRRadio::initialize(int stage) {
    DetailedRadio::initialize(stage);
	if (stage == INITSTAGE_LOCAL) {
		uwbradio = dynamic_cast<RadioUWBIR*>(radio);
        prf = par("PRF");
        assert(prf == 4 || prf == 16);
        rsDecoder = par("RSDecoder").boolValue();
        initializeCounters();
	}
}

MiximRadio* UWBIRRadio::initializeRadio() {
	int    initialRadioState = par("initialRadioState"); //readPar("initalRadioState", (int) RadioUWBIR::RADIO_MODE_SYNC);
	double radioMinAtt       = readPar("radioMinAtt", 1.0);
	double radioMaxAtt       = readPar("radioMaxAtt", 0.0);

	RadioUWBIR* uwbradio = RadioUWBIR::createNewUWBIRRadio(recordStats, initialRadioState, radioMinAtt, radioMaxAtt);

	// Radio timers
	// From Sleep mode
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_SLEEP, IRadio::RADIO_MODE_RECEIVER,    (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_SLEEP, IRadio::RADIO_MODE_TRANSMITTER,    (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

	// From TX mode
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_TRANSMITTER,    RadioUWBIR::RADIO_MODE_SYNC,  (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_TRANSMITTER,    IRadio::RADIO_MODE_RECEIVER,    (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());

	// From RX mode
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_RECEIVER,    IRadio::RADIO_MODE_TRANSMITTER,    (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
	uwbradio->setSwitchTime(IRadio::RADIO_MODE_RECEIVER,    RadioUWBIR::RADIO_MODE_SYNC,  (readPar("timeRXToSYNC", readPar("timeSYNCToRX", 0.000000001))));
	uwbradio->setSwitchTime(RadioUWBIR::RADIO_MODE_SYNC,  IRadio::RADIO_MODE_TRANSMITTER,    (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());

	// From SYNC mode
	uwbradio->setSwitchTime(RadioUWBIR::RADIO_MODE_SYNC,  IRadio::RADIO_MODE_RECEIVER,    (readPar("timeSYNCToRX", readPar("timeRXToSYNC", 0.000000001))));

	return uwbradio;
}

void UWBIRRadio::initializeCounters() {
    totalRxBits = 0;
    errRxBits = 0;
    nbReceivedPacketsRS = 0;
    nbReceivedPacketsNoRS = 0;
    nbSentPackets = 0;
    nbSymbolErrors = 0;
    nbSymbolsReceived = 0;
    nbHandledRxPackets = 0;
    nbFramesDropped = 0;
    packetsBER.setName("packetsBER");
    meanPacketBER.setName("meanPacketBER");
    dataLengths.setName("dataLengths");
    sentPulses.setName("sentPulses");
    receivedPulses.setName("receivedPulses");
    erroneousSymbols.setName("nbErroneousSymbols");
    packetSuccessRate.setName("packetSuccessRate");
    packetSuccessRateNoRS.setName("packetSuccessRateNoRS");
    ber.setName("ber");
    RSErrorRate.setName("ser");
    success.setName("success");
    successNoRS.setName("successNoRS");
}

void UWBIRRadio::finish() {
    recordScalar("Erroneous bits", errRxBits);
    recordScalar("nbSymbolErrors", nbSymbolErrors);
    recordScalar("Total received bits", totalRxBits);
    recordScalar("Average BER", errRxBits / totalRxBits);
    recordScalar("nbReceivedPacketsRS", nbReceivedPacketsRS);
    recordScalar("nbFramesDropped", nbFramesDropped);
    recordScalar("nbReceivedPacketsnoRS", nbReceivedPacketsNoRS);
    if (rsDecoder) {
        recordScalar("nbReceivedPackets", nbReceivedPacketsRS);
    } else {
        recordScalar("nbReceivedPackets", nbReceivedPacketsNoRS);
    }
    recordScalar("nbSentPackets", nbSentPackets);
    DetailedRadio::finish();
}

AnalogueModel* UWBIRRadio::getAnalogueModelFromName(const std::string& name, ParameterMap& params) const {
	if (name == "UWBIRStochasticPathlossModel")
		return createAnalogueModel<UWBIRStochasticPathlossModel>(params);

	if (name == "UWBIRIEEE802154APathlossModel")
		return createAnalogueModel<UWBIRIEEE802154APathlossModel>(params);

	return DetailedRadio::getAnalogueModelFromName(name, params);
}

Decider* UWBIRRadio::getDeciderFromName(const std::string& name, ParameterMap& params) {
	if (name == "DeciderUWBIREDSyncOnAddress") {
	    protocolId = IEEE_802154_UWB;
		return createDecider<DeciderUWBIREDSyncOnAddress>(params);;
	}
	if (name == "DeciderUWBIREDSync") {
	    protocolId = IEEE_802154_UWB;
		return createDecider<DeciderUWBIREDSync>(params);
	}
	if (name=="DeciderUWBIRED") {
	    protocolId = IEEE_802154_UWB;
	    return createDecider<DeciderUWBIRED>(params);
	}

	return DetailedRadio::getDeciderFromName(name, params);
}

simtime_t UWBIRRadio::setRadioState(int rs) {
	int prevState = radio->getCurrentState();

	if(prevState == IRadio::RADIO_MODE_RECEIVER && rs != IRadio::RADIO_MODE_RECEIVER && rs != RadioUWBIR::RADIO_MODE_SYNC) {
	    decider->cancelProcessSignal();
	}

	return DetailedRadio::setRadioState(rs);
}

bool UWBIRRadio::isRadioInRX() const {
    const int iCurRS = getRadioState();

    return iCurRS == IRadio::RADIO_MODE_RECEIVER || iCurRS == RadioUWBIR::RADIO_MODE_SYNC;
}

DetailedRadioFrame* UWBIRRadio::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// create the new AirFrame
	UWBIRRadioFrame* frame = new UWBIRRadioFrame("airframe", AIR_FRAME);

	IEEE802154A::config cfg;
    if (prf == 4)
        cfg = IEEE802154A::cfg_mandatory_4M;
    else if (prf == 16)
        cfg = IEEE802154A::cfg_mandatory_16M;
    prepareData(frame, macPkt->getByteLength(), cfg);

    //set priority of AirFrames above the normal priority to ensure
	//channel consistency (before any thing else happens at a time
	//point t make sure that the channel has removed every AirFrame
	//ended at t and added every AirFrame started at t)
	frame->setSchedulingPriority(airFramePriority);
	frame->setProtocolId(myProtocolId());
	frame->setBitLength(headerLength);
	frame->setId(simulation.getUniqueNumber());
	frame->setChannel(getRadioChannel());
	frame->setCfg(cfg);

    delete macPkt->removeControlInfo();
	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;
	EV_INFO << "AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

	return frame;
}

void UWBIRRadio::sendUp(DetailedRadioFrame* frame, DeciderResult* result) {
    validatePacket(check_and_cast<UWBIRRadioFrame*>(frame), check_and_cast<DeciderResultUWBIR*>(result));
    DetailedRadio::sendUp(frame, result);
}

void UWBIRRadio::prepareData(UWBIRRadioFrame* radioFrame, int byteLength, IEEE802154A::config cfg) {
    // generate signal
    //int nbSymbols = packet->getByteLength() * 8 + 92; // to move to ieee802154a.h
    EV_DETAIL << "prepare Data for a packet with " << byteLength << " data bytes." << endl;
    IEEE802154A::setConfig(cfg);
    IEEE802154A::setPSDULength(byteLength);
    IEEE802154A::signalAndData res = IEEE802154A::generateIEEE802154AUWBSignal(simTime());
    DetailedRadioSignal* theSignal = res.first;
    radioFrame->setDuration(theSignal->getDuration());
    // copy the signal into the AirFrame
    radioFrame->setSignal(*theSignal);
    std::vector<bool>* data = res.second;
    int nbSymbols = data->size();
    // TODO: use IS_EV_DETAIL_ENABLED or similar
    if (false) {
        int nbItems = 0;
        ConstMapping* power = theSignal->getTransmissionPower();
        ConstMappingIterator* iter = power->createConstIterator();
        iter->jumpToBegin();
        while (iter->hasNext()) {
            nbItems++;
            sentPulses.recordWithTimestamp(simTime()
                    + iter->getPosition().getTime(), iter->getValue());
            iter->next();
            //simtime_t t = simTime() + iter->getPosition().getTime();
            //EV_DETAIL << "nbItemsTx=" << nbItems << ", t= " << t <<  ", value=" << iter->getValue() << "." << endl;
        }
    }

    // save bit values
    radioFrame->setBitValuesArraySize(nbSymbols);
    for (int pos = 0; pos < nbSymbols; pos = pos + 1) {
        radioFrame->setBitValues(pos, data->at(pos));
    }
    delete data;
}

bool UWBIRRadio::validatePacket(UWBIRRadioFrame* radioFrame, DeciderResultUWBIR* res) {
    nbHandledRxPackets++;
    const std::vector<bool> * decodedBits = res->getDecodedBits();
    int bitsToDecode = radioFrame->getBitValuesArraySize();
    nbSymbolsReceived = nbSymbolsReceived + ceil((double)bitsToDecode / IEEE802154A::RSSymbolLength);
    int nbBitErrors = 0;
    int pktSymbolErrors = 0;
    bool currSymbolError = false;

    for (int i = 0; i < bitsToDecode; i++) {
        // Start of a new symbol
        if (i % IEEE802154A::RSSymbolLength == 0) {
            currSymbolError = false;
        }
        // bit error
        if (decodedBits->at(i) != radioFrame->getBitValues(i)) {
            nbBitErrors++;
            EV_DETAIL << "Found an error at position " << i << "." << std::endl;
            // symbol error
            if(!currSymbolError) {
                currSymbolError = true;
                pktSymbolErrors = pktSymbolErrors + 1;
            }
        }
    }

    EV_DETAIL << "Found " << nbBitErrors << " bit errors in radio frame." << std::endl;
    double packetBER = static_cast<double>(nbBitErrors)/static_cast<double>(bitsToDecode);
    packetsBER.record(packetBER);
    meanBER.collect(packetBER);
    meanPacketBER.record(meanBER.getMean());
    erroneousSymbols.record(pktSymbolErrors);

    // ! If this condition is true then the next one will also be true
    if(nbBitErrors == 0) {
        successNoRS.record(1);
        nbReceivedPacketsNoRS++;
        packet.setNbPacketsReceivedNoRS(packet.getNbPacketsReceivedNoRS()+1);
    } else {
        successNoRS.record(0);
    }

    if (pktSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) {
        success.record(1);
        nbReceivedPacketsRS++;
        packet.setNbPacketsReceived(packet.getNbPacketsReceived()+1);
        // TODO: emit(BaseLayer::catPacketSignal, &packet);
    } else {
        success.record(0);
        nbFramesDropped++;
    }

    totalRxBits += bitsToDecode;
    errRxBits += nbBitErrors;
    nbSymbolErrors += pktSymbolErrors;
    ber.record(errRxBits/totalRxBits);
    packetSuccessRate.record( ((double) nbReceivedPacketsRS)/nbHandledRxPackets);
    packetSuccessRateNoRS.record( ((double) nbReceivedPacketsNoRS)/nbHandledRxPackets);
    RSErrorRate.record( ((double) nbSymbolErrors) / nbSymbolsReceived);

    // validate message
    bool success = false;

    success = (nbBitErrors == 0 || (rsDecoder && pktSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) );

    return success;
}
