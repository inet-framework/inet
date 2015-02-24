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

#include "inet/physicallayer/base/FlatTransmissionBase.h"
#include "inet/physicallayer/ieee80211/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/ieee80211/errormodel/Ieee80211BerTableErrorModel.h"
#include "inet/physicallayer/ieee80211/errormodel/Ieee80211NistErrorModel.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"

namespace inet {

namespace physicallayer {

using namespace ieee80211;

Define_Module(Ieee80211BerTableErrorModel);

Ieee80211BerTableErrorModel::Ieee80211BerTableErrorModel() :
    berTableFile(nullptr)
{
}

Ieee80211BerTableErrorModel::~Ieee80211BerTableErrorModel()
{
    delete berTableFile;
}

void Ieee80211BerTableErrorModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *fname = par("berTableFile");
        if (fname == nullptr)
            throw cRuntimeError("BER file parameter is mandatory");
        // TODO: remove and cleanup opMode from here and also from BerParseFile, this should depend on the received signal
        char opMode;
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
        berTableFile = new BerParseFile(opMode);
        berTableFile->parseFile(fname);
    }
}

double Ieee80211BerTableErrorModel::computePacketErrorRate(const ISNIR *snir) const
{
    const ITransmission *transmission = snir->getReception()->getTransmission();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    const Ieee80211TransmissionBase *ieee80211Transmission = check_and_cast<const Ieee80211TransmissionBase *>(transmission);
    const IIeee80211Mode *mode = ieee80211Transmission->getMode();

    Ieee80211NistErrorModel e;
    // probability of no bit error in the header
    double minSNIR = snir->getMin();
    int headerBitLength = flatTransmission->getHeaderBitLength();
    double headerSuccessRate = e.GetChunkSuccessRate(mode->getHeaderMode(), minSNIR, headerBitLength);

    // probability of no bit error in the MPDU
    int payloadBitLength = flatTransmission->getPayloadBitLength();
    double payloadSuccessRate;
    if (berTableFile) {
        double bitrate = flatTransmission->getBitrate().get();
        payloadSuccessRate = 1 - berTableFile->getPer(bitrate, minSNIR, payloadBitLength / 8);
    }
    else
        payloadSuccessRate = e.GetChunkSuccessRate(mode->getDataMode(), minSNIR, payloadBitLength);

    EV_DEBUG << "min SNIR = " << minSNIR << ", bit length = " << payloadBitLength << ", header error rate = " << 1 - headerSuccessRate << ", payload error rate = " << 1 - payloadSuccessRate << endl;

    if (headerSuccessRate >= 1)
        headerSuccessRate = 1;
    if (payloadSuccessRate >= 1)
        payloadSuccessRate = 1;
    return 1 - headerSuccessRate * payloadSuccessRate;
//    const ITransmission *transmission = snir->getReception()->getTransmission();
//    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
//    double bitrate = flatTransmission->getBitrate().get();
//    double minSNIR = snir->getMin();
//    int payloadBitLength = flatTransmission->getPayloadBitLength();
//    return berTableFile->getPer(bitrate, minSNIR, payloadBitLength / 8);
}

double Ieee80211BerTableErrorModel::computeBitErrorRate(const ISNIR *snir) const
{
    return NaN;
}

double Ieee80211BerTableErrorModel::computeSymbolErrorRate(const ISNIR *snir) const
{
    return NaN;
}

} // namespace physicallayer

} // namespace inet

