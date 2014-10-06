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

#include "Ieee80211LayeredDecoder.h"
#include "Ieee80211ConvolutionalCode.h"
#include "BPSKModulation.h"
#include "QPSKModulation.h"
#include "QAM16Modulation.h"
#include "QAM64Modulation.h"
#include "IAPSKModulation.h"
#include "DummySerializer.h"
#include "SignalPacketModel.h"

#define OFDM_SYMBOL_SIZE 48
#define ENCODED_SIGNAL_FIELD_LENGTH 48
// Table L-7—Bit assignment for SIGNAL field
#define SIGNAL_RATE_FIELD_START 0
#define SIGNAL_RATE_FIELD_END 3
#define SIGNAL_LENGTH_FIELD_START 5
#define SIGNAL_LENGTH_FIELD_END 16
#define SIGNAL_PARITY_FIELD 17
#define PPDU_SERVICE_FIELD_LENGTH 16
#define PPDU_TAIL_BITS_LENGTH 6
#define SYMBOLS_PER_DATA_FIELD 6

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211LayeredDecoder);

void Ieee80211LayeredDecoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        // TODO: maybe they should be modules
        deserializer = check_and_cast<DummySerializer *>(getSubmodule("deserializer")); // FIXME
        descrambling = new Ieee80211Scrambling("1011101", "0001001");
        descrambler = new Ieee80211Scrambler(descrambling);
        signalFECDecoder = new ConvolutionalCoder(new Ieee80211ConvolutionalCode(1,2));
        signalDeinterleaver = new Ieee80211Interleaver(new Ieee80211Interleaving(BPSKModulation::singleton.getCodeWordLength() * OFDM_SYMBOL_SIZE, BPSKModulation::singleton.getCodeWordLength()));
    }
}

const Ieee80211ConvolutionalCode* Ieee80211LayeredDecoder::getFecFromSignalFieldRate(const ShortBitVector& rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    // FIXME: memory leaks
    if (rate == ShortBitVector("1101") || rate == ShortBitVector("0101") || rate == ShortBitVector("1001"))
        return new Ieee80211ConvolutionalCode(1, 2);
    else if (rate == ShortBitVector("1111") || rate == ShortBitVector("0111") || rate == ShortBitVector("1011") ||
             rate == ShortBitVector("0111"))
        return new Ieee80211ConvolutionalCode(3, 4);
    else if (rate == ShortBitVector("0001"))
        return new Ieee80211ConvolutionalCode(2, 3);
    else
        throw cRuntimeError("Unknown rate field  = %s", rate.toString().c_str());
}

const APSKModulationBase* Ieee80211LayeredDecoder::getModulationFromSignalFieldRate(const ShortBitVector& rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    if (rate == ShortBitVector("1101") || rate == ShortBitVector("1111"))
        return &BPSKModulation::singleton;
    else if (rate == ShortBitVector("0101") || rate == ShortBitVector("0111"))
        return &QPSKModulation::singleton;
    else if (rate == ShortBitVector("1001") || rate == ShortBitVector("1011"))
        return &QAM16Modulation::singleton;
    else if(rate == ShortBitVector("0001") || rate == ShortBitVector("0011"))
        return &QAM64Modulation::singleton;
    else
        throw cRuntimeError("Unknown rate field = %s", rate.toString().c_str());
}

const IReceptionPacketModel* Ieee80211LayeredDecoder::decode(const IReceptionBitModel* bitModel) const
{
    const BitVector *bits = bitModel->getBits();
    BitVector signalField;
    for (unsigned int i = 0; i < ENCODED_SIGNAL_FIELD_LENGTH; i++)
        signalField.appendBit(bits->getBit(i));
    BitVector decodedSignalField = decodeSignalField(signalField);
    ShortBitVector signalFieldRate = getSignalFieldRate(decodedSignalField);
    const Ieee80211ConvolutionalCode *fec = getFecFromSignalFieldRate(signalFieldRate);
    const IModulation *modulationScheme = bitModel->getModulation();
    ASSERT(modulationScheme != NULL);
    const Ieee80211Interleaving *deinterleaving = getInterleavingFromModulation(modulationScheme);
    BitVector dataField;
    Ieee80211Interleaver deinterleaver(deinterleaving);
    ConvolutionalCoder fecDecoder(fec);
    unsigned int psduLengthInBits = getSignalFieldLength(decodedSignalField) * 8;
    unsigned int dataFieldLengthInBits = psduLengthInBits + PPDU_SERVICE_FIELD_LENGTH + PPDU_TAIL_BITS_LENGTH;
    dataFieldLengthInBits += calculatePadding(dataFieldLengthInBits, modulationScheme, fec);
    ASSERT(dataFieldLengthInBits % fec->getCodeRatePuncturingK() == 0);
    unsigned int encodedDataFieldLengthInBits = dataFieldLengthInBits * fec->getCodeRatePuncturingN() / fec->getCodeRatePuncturingK();
    if (dataFieldLengthInBits + ENCODED_SIGNAL_FIELD_LENGTH > bits->getSize())
        throw cRuntimeError("The calculated data field length = %d is greater then the actual bitvector length = %d", dataFieldLengthInBits, bits->getSize());
    for (unsigned int i = 0; i < encodedDataFieldLengthInBits; i++)
        dataField.appendBit(bits->getBit(ENCODED_SIGNAL_FIELD_LENGTH+i));
    BitVector decodedDataField = decodeDataField(dataField, fecDecoder, deinterleaver);
    BitVector decodedBits;
    for (unsigned int i = 0; i < decodedSignalField.getSize(); i++)
        decodedBits.appendBit(decodedSignalField.getBit(i));
    for (unsigned int i = 0; i < decodedDataField.getSize(); i++)
        decodedBits.appendBit(decodedDataField.getBit(i));
    return createPacketModel(decodedBits, descrambling, fec, deinterleaving);
}

const Ieee80211Interleaving* Ieee80211LayeredDecoder::getInterleavingFromModulation(const IModulation *modulationScheme) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    return new Ieee80211Interleaving(dataModulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE, dataModulationScheme->getCodeWordLength()); // FIXME: memory leak
}

BitVector Ieee80211LayeredDecoder::decodeSignalField(const BitVector& signalField) const
{
    BitVector deinterleavedSignalField = signalDeinterleaver->deinterleave(signalField);
    return signalFECDecoder->decode(deinterleavedSignalField);
}

BitVector Ieee80211LayeredDecoder::decodeDataField(const BitVector& dataField, const ConvolutionalCoder& fecDecoder, const Ieee80211Interleaver& deinterleaver) const
{
    BitVector deinterleavedDataField = deinterleaver.deinterleave(dataField);
    BitVector fecDecodedDataField = fecDecoder.decode(deinterleavedDataField);
    return descrambler->descramble(fecDecodedDataField);
}

const IReceptionPacketModel* Ieee80211LayeredDecoder::createPacketModel(const BitVector& decodedBits, const Ieee80211Scrambling *scrambling, const Ieee80211ConvolutionalCode *fec, const Ieee80211Interleaving *interleaving) const
{
    double per = -1;
    bool packetErrorless = false; // TODO: compute packet error rate, packetErrorLess
    const cPacket *packet = deserializer->deserialize(decodedBits);
    return new ReceptionPacketModel(packet, fec, scrambling, interleaving, per, packetErrorless);
    return NULL;
}

ShortBitVector Ieee80211LayeredDecoder::getSignalFieldRate(const BitVector& signalField) const
{
    ShortBitVector rate;
    for (int i = SIGNAL_RATE_FIELD_START; i <= SIGNAL_RATE_FIELD_END; i++)
        rate.appendBit(signalField.getBit(i));
    return rate;
}

unsigned int Ieee80211LayeredDecoder::getSignalFieldLength(const BitVector& signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField.getBit(i));
    return length.toDecimal();
}

unsigned int Ieee80211LayeredDecoder::calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    unsigned int codedBitsPerOFDMSymbol = dataModulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    if (dataFieldLengthInBits / SYMBOLS_PER_DATA_FIELD > dataBitsPerOFDMSymbol)
    {
        throw cRuntimeError("Incorrect data field length = %d", dataFieldLengthInBits);
        return -1;
    }
    else
        return SYMBOLS_PER_DATA_FIELD * dataBitsPerOFDMSymbol - dataFieldLengthInBits;
}

Ieee80211LayeredDecoder::~Ieee80211LayeredDecoder()
{
    delete signalDeinterleaver;
    delete descrambler;
    delete signalFECDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */
