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

#include "inet/linklayer/ieee80211/mac/WifiMode.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/physicallayer/common/ModulationType.h"
#include "inet/physicallayer/ieee80211/Ieee80211ScalarTransmission.h"
#include "inet/physicallayer/ieee80211/errormodel/Ieee80211ErrorModelBase.h"

namespace inet {

namespace physicallayer {

using namespace ieee80211;

Ieee80211ErrorModelBase::Ieee80211ErrorModelBase() :
    opMode('\0'),
    autoHeaderSize(false),
    berTableFile(NULL)
{
}

Ieee80211ErrorModelBase::~Ieee80211ErrorModelBase()
{
    delete berTableFile;
}

void Ieee80211ErrorModelBase::initialize(int stage)
{
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
        autoHeaderSize = par("autoHeaderSize");
        const char *fname = par("berTableFile");
        std::string name(fname);
        if (!name.empty()) {
            // TODO: remove opMode from here and also from BerParseFile
            berTableFile = new BerParseFile(opMode);
            berTableFile->parseFile(fname);
        }
    }
}

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISNIR *snir) const
{
    const Ieee80211ScalarTransmission *ieee80211Transmission = check_and_cast<const Ieee80211ScalarTransmission *>(snir->getReception()->getTransmission());
    int bitLength = ieee80211Transmission->getPayloadBitLength();
    double bitrate = ieee80211Transmission->getBitrate().get();
    WifiPreamble preambleUsed = ieee80211Transmission->getPreambleMode();
    char opMode = ieee80211Transmission->getOpMode();

    uint32_t headerSize;
    if (opMode == 'b')
        headerSize = HEADER_WITHOUT_PREAMBLE;
    else
        headerSize = 24;
    ModulationType modeBody = WifiModulationType::getModulationType(opMode, bitrate);
    ModulationType modeHeader = WifiModulationType::getPlcpHeaderMode(modeBody, preambleUsed);
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
    else
        throw cRuntimeError("Radio model not supported yet, must be a,b,g or p");
    double minSNIR = snir->computeMin();
    double headerNoError = GetChunkSuccessRate(modeHeader, minSNIR, headerSize);

    // probability of no bit error in the MPDU
    double mpduNoError;
    if (berTableFile)
        mpduNoError = 1 - berTableFile->getPer(bitrate, minSNIR, bitLength / 8);
    else
        mpduNoError = GetChunkSuccessRate(modeBody, minSNIR, bitLength);

    EV << "bit length = " << bitLength << " packet error rate = " << 1 - mpduNoError << " header error rate = " << 1 - headerNoError << endl;

// TODO: use this code instead of the other, changes fingerprints!
//    if (headerNoError >= 1)
//        headerNoError = 1;
//    if (MpduNoError >= 1)
//        MpduNoError = 1;
//    return 1 - headerNoError * MpduNoError;
    if (mpduNoError >= 1 && headerNoError >= 1)
        return 0.0;
    if (dblrand() > headerNoError)
        return 1.0;
    else if (dblrand() > mpduNoError)
        return 1.0;
    else
        return 0.0;
}

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISNIR *snir) const
{
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISNIR *snir) const
{
    return NaN;
}

} // namespace physicallayer

} // namespace inet

