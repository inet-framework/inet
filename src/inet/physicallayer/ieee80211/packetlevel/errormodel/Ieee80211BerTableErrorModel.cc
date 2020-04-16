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

#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211BerTableErrorModel.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

namespace inet {

namespace physicallayer {

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
        else if (!strcmp("g(erp)", opModeString))
            opMode = 'g';
        else if (!strcmp("g(mixed)", opModeString))
            opMode = 'g';
        else if (!strcmp("a", opModeString))
            opMode = 'a';
        else if (!strcmp("p", opModeString))
            opMode = 'p';
        else
            throw cRuntimeError("Unknown opMode");
        berTableFile = new BerParseFile(opMode);
        berTableFile->parseFile(fname);
    }
}

double Ieee80211BerTableErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    const ITransmission *transmission = snir->getReception()->getTransmission();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    double bitrate = flatTransmission->getBitrate().get();
    b dataLength = flatTransmission->getDataLength();
    return berTableFile->getPer(bitrate, getScalarSnir(snir), B(dataLength).get());
}

double Ieee80211BerTableErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

double Ieee80211BerTableErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

} // namespace physicallayer

} // namespace inet

