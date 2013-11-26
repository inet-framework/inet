#include "PhyLayerUWBIR.h"

#include <cassert>

#include "DeciderUWBIREDSyncOnAddress.h"
#include "DeciderUWBIREDSync.h"
#include "MacToUWBIRPhyControlInfo.h"
#include "AirFrameUWBIR_m.h"
#include "BaseWorldUtility.h"
#include "UWBIRStochasticPathlossModel.h"
#include "UWBIRIEEE802154APathlossModel.h"

Define_Module(PhyLayerUWBIR);

//t_dynamic_expression_value (PhyLayerUWBIR::*ghassemzadehNLOSFPtr) (cComponent *context, t_dynamic_expression_value argv[], int argc) = &ghassemzadehNLOSFunc;
PhyLayerUWBIR::fptr ghassemzadehNLOSFPtr = &PhyLayerUWBIR::ghassemzadehNLOSFunc;
Define_NED_Function(ghassemzadehNLOSFPtr, "xml ghassemzadehNLOS()");

void PhyLayerUWBIR::initialize(int stage) {
    PhyLayerBattery::initialize(stage);
	if (stage == 0) {
		/* parameters belong to the NIC, not just phy layer
		 *
		 * if/when variable transmit power is supported, txCurrent
		 * should be specified as an xml table of available transmit
		 * power levels and corresponding txCurrent */
		syncCurrent = getNic()->par( "syncCurrent" ); // assume instantaneous transitions between rx and sync
		uwbradio    = dynamic_cast<RadioUWBIR*>(radio);
	}
}

MiximRadio* PhyLayerUWBIR::initializeRadio() const {
	int    initialRadioState = par("initialRadioState"); //readPar("initalRadioState", (int) RadioUWBIR::SYNC);
	double radioMinAtt       = readPar("radioMinAtt", 1.0);
	double radioMaxAtt       = readPar("radioMaxAtt", 0.0);

	RadioUWBIR* uwbradio = RadioUWBIR::createNewUWBIRRadio(recordStats, initialRadioState, radioMinAtt, radioMaxAtt);

	// Radio timers
	// From Sleep mode
	uwbradio->setSwitchTime(RadioUWBIR::SLEEP, RadioUWBIR::RX,    (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());
	uwbradio->setSwitchTime(RadioUWBIR::SLEEP, RadioUWBIR::TX,    (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

	// From TX mode
	uwbradio->setSwitchTime(RadioUWBIR::TX,    RadioUWBIR::SYNC,  (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
	uwbradio->setSwitchTime(RadioUWBIR::TX,    RadioUWBIR::RX,    (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());

	// From RX mode
	uwbradio->setSwitchTime(RadioUWBIR::RX,    RadioUWBIR::TX,    (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
	uwbradio->setSwitchTime(RadioUWBIR::RX,    RadioUWBIR::SYNC,  (readPar("timeRXToSYNC", readPar("timeSYNCToRX", 0.000000001))));
	uwbradio->setSwitchTime(RadioUWBIR::SYNC,  RadioUWBIR::TX,    (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());

	// From SYNC mode
	uwbradio->setSwitchTime(RadioUWBIR::SYNC,  RadioUWBIR::RX,    (readPar("timeSYNCToRX", readPar("timeRXToSYNC", 0.000000001))));

	return uwbradio;
}

AnalogueModel* PhyLayerUWBIR::getAnalogueModelFromName(const std::string& name, ParameterMap& params) const {
	if (name == "UWBIRStochasticPathlossModel")
		return createAnalogueModel<UWBIRStochasticPathlossModel>(params);

	if (name == "UWBIRIEEE802154APathlossModel")
		return createAnalogueModel<UWBIRIEEE802154APathlossModel>(params);

	return PhyLayerBattery::getAnalogueModelFromName(name, params);
}

Decider* PhyLayerUWBIR::getDeciderFromName(const std::string& name, ParameterMap& params) {
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

	return PhyLayerBattery::getDeciderFromName(name, params);
}

void PhyLayerUWBIR::setSwitchingCurrent(int from, int to) {
	int    act     = SWITCHING_ACCT;
	double current = 0;

	if (from != to && (from == RadioUWBIR::SYNC || to == RadioUWBIR::SYNC)) {
	    if (from == RadioUWBIR::SYNC) {
	        switch(to) {
                case RadioUWBIR::RX:
                case RadioUWBIR::SLEEP:
                    current = syncCurrent;
                break;
                case RadioUWBIR::TX:
                    current = rxTxCurrent;
                break;
                // ! transitions between rx and sync should be immediate
                default:
                    opp_error("Unknown radio switch! From SYNC to %d", to);
                break;
	        }
	    }
	    else {
            switch(from) {
                case RadioUWBIR::RX:
                    current = syncCurrent;
                break;
                case RadioUWBIR::SLEEP:
                    current = setupRxCurrent;
                break;
                case RadioUWBIR::TX:
                    current = txRxCurrent;
                break;
                // ! transitions between rx and sync should be immediate
                default:
                    opp_error("Unknown radio switch! From %d to SYNC", from);
                break;
            }
	    }
	}
	else {
	    PhyLayerBattery::setSwitchingCurrent(from, to);
	    return;
	}

	MiximBatteryAccess::drawCurrent(current, act);
}

void PhyLayerUWBIR::setRadioCurrent(int rs) {
	switch(rs) {
        case RadioUWBIR::SYNC:
            MiximBatteryAccess::drawCurrent(syncCurrent, SYNC_ACCT);
            break;
        case RadioUWBIR::SWITCHING:
            // do nothing here
            break;
        default:
            PhyLayerBattery::setRadioCurrent(rs);
            break;
	}
}

simtime_t PhyLayerUWBIR::setRadioState(int rs) {
	int prevState = radio->getCurrentState();

	if(prevState == RadioUWBIR::RX && rs != RadioUWBIR::RX && rs != RadioUWBIR::SYNC) {
	    decider->cancelProcessSignal();
	}

	return PhyLayerBattery::setRadioState(rs);
}

bool PhyLayerUWBIR::isRadioInRX() const {
    const int iCurRS = getRadioState();

    return iCurRS == RadioUWBIR::RX || iCurRS == RadioUWBIR::SYNC;
}

PhyLayerUWBIR::airframe_ptr_t PhyLayerUWBIR::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// ...and must always have a ControlInfo attached (contains Signal)
	cObject* ctrlInfo = macPkt->removeControlInfo();
	assert(ctrlInfo);

	// create the new AirFrame
	AirFrameUWBIR* frame = new AirFrameUWBIR("airframe", AIR_FRAME);

	// Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
	// We are now the new owner of this instance.
	Signal* s = MacToUWBIRPhyControlInfo::getSignalFromControlInfo(ctrlInfo);
	// make sure we really obtained a pointer to an instance
	assert(s);

	// set the members
	assert(s->getDuration() > 0);
	frame->setDuration(s->getDuration());
	// copy the signal into the AirFrame
	frame->setSignal(*s);
	//set priority of AirFrames above the normal priority to ensure
	//channel consistency (before any thing else happens at a time
	//point t make sure that the channel has removed every AirFrame
	//ended at t and added every AirFrame started at t)
	frame->setSchedulingPriority(airFramePriority);
	frame->setProtocolId(myProtocolId());
	frame->setBitLength(headerLength);
	frame->setId(world->getUniqueAirFrameId());
	frame->setChannel(radio->getCurrentChannel());
	frame->setCfg(MacToUWBIRPhyControlInfo::getConfigFromControlInfo(ctrlInfo));

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	// delete the Control info
	delete ctrlInfo;
	ctrlInfo = 0;

	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;
	coreEV <<"AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

	return frame;
}


void PhyLayerUWBIR::finish() {
	PhyLayerBattery::finish();
}

