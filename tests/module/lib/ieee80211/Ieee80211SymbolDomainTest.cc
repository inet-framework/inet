//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "Ieee80211SymbolDomainTest.h"
#include "inet/common/ModuleAccess.h"
#include <fstream>

namespace inet {

Define_Module(Ieee80211SymbolDomainTest);

void Ieee80211SymbolDomainTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ieee80211LayeredEncoder = getModuleFromPar<Ieee80211LayeredEncoder>(par("ieee80211LayeredEncoderModule"), this);
        ieee80211OFDMModulator = getModuleFromPar<Ieee80211OFDMModulator>(par("ieee80211OFDMModulatorModule"), this);
        ieee80211OFDMDemodulator = getModuleFromPar<Ieee80211OFDMDemodulator>(par("ieee80211OFDMDemodulatorModule"), this);
        ieee80211LayeredDecoder = getModuleFromPar<Ieee80211LayeredDecoder>(par("ieee80211LayeredDecoderModule"), this);
        serializer = getModuleFromPar<DummySerializer>(par("serializerModule"), this);
        parseInput(par("testFile").stringValue());
    }
    else if (stage == INITSTAGE_LAST)
    {
        test();
    }
}

void Ieee80211SymbolDomainTest::parseInput(const char* fileName)
{
    std::ifstream file(fileName);
    std::string in;
    while (file >> in)
    {
        for (unsigned int i = 0; i < in.size(); i++)
        {
            if (in.at(i) == '0')
                input.appendBit(false);
            else if (in.at(i) == '1')
                input.appendBit(true);
            else
                throw cRuntimeError("Unexpected input format = %s", in.c_str());
        }
    }
}

void Ieee80211SymbolDomainTest::test() const
{
    TransmissionPacketModel packetModel;
    serializer->setDummyOutputBits(input);
    const ITransmissionBitModel *bitModel = ieee80211LayeredEncoder->encode(&packetModel);
    const ITransmissionSymbolModel *transmissionSymbolModel = ieee80211OFDMModulator->modulate(bitModel);
    ReceptionSymbolModel receptionSymbolModel(0, 0, transmissionSymbolModel->getSymbols(), 0, 0);
    const IReceptionBitModel *receptionBitModel = ieee80211OFDMDemodulator->demodulate(&receptionSymbolModel);
    const IReceptionPacketModel *receptionPacketModel = ieee80211LayeredDecoder->decode(receptionBitModel);
    delete bitModel;
    delete transmissionSymbolModel;
    delete receptionBitModel;
    delete receptionPacketModel;
}

} /* namespace inet */

