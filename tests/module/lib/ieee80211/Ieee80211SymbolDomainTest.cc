//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "Ieee80211SymbolDomainTest.h"
#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include <fstream>

namespace inet {

Define_Module(Ieee80211SymbolDomainTest);

void Ieee80211SymbolDomainTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ieee80211OFDMSignalEncoder = getModuleFromPar<Ieee80211OfdmEncoderModule>(par("ieee80211OFDMSignalEncoderModule"), this);
        ieee80211OFDMDataEncoder = getModuleFromPar<Ieee80211OfdmEncoderModule>(par("ieee80211OFDMDataEncoderModule"), this);
        ieee80211OFDMSignalModulator = getModuleFromPar<Ieee80211OfdmModulatorModule>(par("ieee80211OFDMSignalModulatorModule"), this);
        ieee80211OFDMDataModulator = getModuleFromPar<Ieee80211OfdmModulatorModule>(par("ieee80211OFDMDataModulatorModule"), this);
        ieee80211OFDMSignalDemodulator = getModuleFromPar<Ieee80211OfdmDemodulatorModule>(par("ieee80211OFDMSignalDemodulatorModule"), this);
        ieee80211OFDMDataDemodulator = getModuleFromPar<Ieee80211OfdmDemodulatorModule>(par("ieee80211OFDMDataDemodulatorModule"), this);
        ieee80211OFDMSignalDecoder = getModuleFromPar<Ieee80211OfdmDecoderModule>(par("ieee80211OFDMSignalDecoderModule"), this);
        ieee80211OFDMDataDecoder = getModuleFromPar<Ieee80211OfdmDecoderModule>(par("ieee80211OFDMDataDecoderModule"), this);
        parseInput(par("testFile"));
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
    const auto& signalField = makeShared<BytesChunk>(std::vector<uint8_t>(3));
    const auto& dataField = makeShared<BytesChunk>(std::vector<uint8_t>(input.getNumberOfBytes() - 3));
    for (unsigned int i = 0; i < 3; i++)
        signalField->setByte(i, input.getBytes()[i]);
    for (unsigned int i = 3; i < input.getNumberOfBytes(); i++)
        dataField->setByte(i - 3, input.getBytes()[i]);
    auto signalPacket = new Packet("signal", signalField);
    auto dataPacket = new Packet("data", dataField);
    TransmissionPacketModel signalPacketModel(signalPacket, bps(NaN), bps(NaN));
    TransmissionPacketModel dataPacketModel(dataPacket, bps(NaN), bps(NaN));
    const ITransmissionBitModel *signalBitModel = ieee80211OFDMSignalEncoder->encode(&signalPacketModel);
    const ITransmissionBitModel *dataBitModel = ieee80211OFDMDataEncoder->encode(&dataPacketModel);
    const ITransmissionSymbolModel *transmissionSignalSymbolModel = ieee80211OFDMSignalModulator->modulate(signalBitModel);
    const ITransmissionSymbolModel *transmissionDataSymbolModel = ieee80211OFDMDataModulator->modulate(dataBitModel);
    auto receptionSignalSymbols = new std::vector<const ISymbol *>();
    for (auto symbol : *transmissionSignalSymbolModel->getAllSymbols())
        receptionSignalSymbols->push_back(new Ieee80211OfdmSymbol(*check_and_cast<const Ieee80211OfdmSymbol *>(symbol)));
    ReceptionSymbolModel receptionSignalSymbolModel(0, 0, 0, 0, receptionSignalSymbols, NaN);
    const IReceptionBitModel *receptionSignalBitModel = ieee80211OFDMSignalDemodulator->demodulate(&receptionSignalSymbolModel);
    auto receptionDataSymbols = new std::vector<const ISymbol *>();
    for (auto symbol : *transmissionDataSymbolModel->getAllSymbols())
        receptionDataSymbols->push_back(new Ieee80211OfdmSymbol(*check_and_cast<const Ieee80211OfdmSymbol *>(symbol)));
    ReceptionSymbolModel receptionDataSymbolModel(0, 0, 0, 0, receptionDataSymbols, NaN);
    const IReceptionBitModel *receptionDataBitModel = ieee80211OFDMDataDemodulator->demodulate(&receptionDataSymbolModel);
    const IReceptionPacketModel *receptionSignalPacketModel = ieee80211OFDMSignalDecoder->decode(receptionSignalBitModel);
    const IReceptionPacketModel *receptionDataPacketModel = ieee80211OFDMDataDecoder->decode(receptionDataBitModel);
    delete dataPacket;
    delete signalPacket;
    delete dataBitModel;
    delete signalBitModel;
    delete transmissionDataSymbolModel;
    delete transmissionSignalSymbolModel;
    delete receptionDataBitModel;
    delete receptionSignalBitModel;
    delete receptionDataPacketModel->getPacket();
    delete receptionSignalPacketModel->getPacket();
    delete receptionDataPacketModel;
    delete receptionSignalPacketModel;
}

} /* namespace inet */

