#include "DeciderUWBIREDSyncOnAddress.h"

#include "DeciderUWBIRED.h"
#include "IMACFrame.h"

bool DeciderUWBIREDSyncOnAddress::initFromMap(const ParameterMap& params) {
    bool                         bInitSuccess = true;
    ParameterMap::const_iterator it           = params.find("addr");
    if(it != params.end()) {
        syncAddress = MACAddress(ParameterMap::mapped_type(it->second).longValue());
    }
    else {
        bInitSuccess = false;
        opp_warning("No addr defined in config.xml for DeciderUWBIREDSyncOnAddress!");
    }
    return DeciderUWBIRED::initFromMap(params) && bInitSuccess;
}

bool DeciderUWBIREDSyncOnAddress::attemptSync(const DetailedRadioFrame * /*frame*/) {
    if (!currentSignal.isProcessing())
        return false;

	cMessage* encaps = currentSignal.first->getEncapsulatedPacket();

	return ((check_and_cast<IMACFrame*>(encaps))->getSourceAddress()==syncAddress);
};


