//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "Ieee80211ScalarReceiver.h"
#include "ModulationType.h"
#include "WifiMode.h"
#include "BerParseFile.h"
#include "Ieee80211Consts.h"
#include "yans-error-rate-model.h"
#include "nist-error-rate-model.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211ScalarReceiver);

Ieee80211ScalarReceiver::~Ieee80211ScalarReceiver()
{
    delete errorModel;
    delete parseTable;
}

void Ieee80211ScalarReceiver::initialize(int stage)
{
    ScalarReceiver::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opModeString = par("opMode");
        if (!strcmp("b", opModeString))
            opMode = 'b';
        else if (!strcmp("g", opModeString))
            opMode = 'g';
        else if (!strcmp("a", opModeString))
            opMode = 'a';
        else if (!strcmp("p", opModeString))
            opMode = 'p';
        else
            opMode = 'g';
        const char *preambleModeString = par("preambleMode");
        if (!strcmp("short", preambleModeString))
            preambleMode = WIFI_PREAMBLE_SHORT;
        else if (!strcmp("long", preambleModeString))
            preambleMode = WIFI_PREAMBLE_LONG;
        else
            throw cRuntimeError("Unknown preamble mode");
        const char *errorModelString = par("errorModel");
        if (!strcmp("yans", errorModelString))
            errorModel = new YansErrorRateModel();
        else if (!strcmp("nist", errorModelString))
            errorModel = new NistErrorRateModel();
        else
            opp_error("Error %s model is not valid", errorModelString);
        autoHeaderSize = par("autoHeaderSize");
        parseTable = NULL;
        const char *fname = par("berTableFile");
        std::string name(fname);
        if (!name.empty()) {
            parseTable = new BerParseFile(opMode);
            parseTable->parseFile(fname);
        }
    }
}

bool Ieee80211ScalarReceiver::computeHasBitError(const IListening *listening, double minSNIR, int bitLength, double bitrate) const
{
    ModulationType modeBody;
    ModulationType modeHeader;

    // TODO: use transmission's opMode for error detection
    WifiPreamble preambleUsed = preambleMode;
    double headerNoError;
    uint32_t headerSize;
    if (opMode == 'b')
        headerSize = HEADER_WITHOUT_PREAMBLE;
    else
        headerSize = 24;

    modeBody = WifiModulationType::getModulationType(opMode, bitrate);
    modeHeader = WifiModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
    if (opMode == 'g') {
        if (autoHeaderSize) {
            ModulationType modeBodyA = WifiModulationType::getModulationType('a', bitrate);
            headerSize = ceil(SIMTIME_DBL(WifiModulationType::getPlcpHeaderDuration(modeBodyA, preambleUsed)) * modeHeader.getDataRate());
        }
    }
    else if (opMode == 'b' || opMode == 'a' || opMode == 'p') {
        if (autoHeaderSize)
            headerSize = ceil(SIMTIME_DBL(WifiModulationType::getPlcpHeaderDuration(modeBody, preambleUsed)) * modeHeader.getDataRate());
    }
    else {
        opp_error("Radio model not supported yet, must be a,b,g or p");
    }

    headerNoError = errorModel->GetChunkSuccessRate(modeHeader, minSNIR, headerSize);
    // probability of no bit error in the MPDU
    double MpduNoError;
    if (parseTable)
        MpduNoError = 1 - parseTable->getPer(bitrate, minSNIR, bitLength / 8);
    else
        MpduNoError = errorModel->GetChunkSuccessRate(modeBody, minSNIR, bitLength);

    EV << "bit length = " << bitLength << " packet error rate = " << 1 - MpduNoError << " header error rate = " << 1 - headerNoError << endl;
    if (MpduNoError >= 1 && headerNoError >= 1)
        return false;
    if (dblrand() > headerNoError)
        return true;
    else if (dblrand() > MpduNoError)
        return true;
    else
        return false;
}

} // namespace physicallayer

} // namespace inet

