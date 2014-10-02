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
#include "DummySerializer.h"
#include "SignalPacketModel.h"

#define ENCODED_SIGNAL_FIELD_LENGTH 48

namespace inet {
namespace physicallayer {

void Ieee80211LayeredDecoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        // TODO: maybe they should be modules
        deserializer = new DummySerializer();
        descrambler = new Ieee80211Scrambler(new Ieee80211Scrambling("1011101", "0001001"));
        signalFECDecoder = new ConvolutionalCoder(new Ieee80211ConvolutionalCode(1,2));
        signalDeinterleaver = new Ieee80211Interleaver(new Ieee80211Interleaving(BPSKModulation::singleton.getCodeWordLength() * 48, BPSKModulation::singleton.getCodeWordLength()));
    }
}


const IReceptionPacketModel* Ieee80211LayeredDecoder::decode(const IReceptionBitModel* bitModel) const
{
    const Ieee80211ConvolutionalCode *fec = dynamic_cast<const Ieee80211ConvolutionalCode *>(bitModel->getForwardErrorCorrection());
    const Ieee80211Interleaving *deinterleaving = dynamic_cast<const Ieee80211Interleaving *>(bitModel->getInterleaverInfo());
    ASSERT(fec != NULL);
    ASSERT(deinterleaving != NULL);
    Ieee80211Interleaver deinterleaver(deinterleaving);
    ConvolutionalCoder fecDecoder(fec);
    const BitVector *bits = bitModel->getBits();
    BitVector signalField;
    for (unsigned int i = 0; i < ENCODED_SIGNAL_FIELD_LENGTH; i++)
        signalField.appendBit(bits->getBit(i));
    BitVector decodedSignalField = decodeSignalField(signalField);
    BitVector dataField;
    for (unsigned int i = ENCODED_SIGNAL_FIELD_LENGTH; i < bits->getSize(); i++)
        dataField.appendBit(bits->getBit(i));
    BitVector decodedDataField = decodeDataField(dataField, fecDecoder, deinterleaver);
    BitVector decodedBits;
    for (unsigned int i = 0; i < decodedSignalField.getSize(); i++)
        decodedBits.appendBit(decodedSignalField.getBit(i));
    for (unsigned int i = 0; i < decodedDataField.getSize(); i++)
        decodedBits.appendBit(decodedDataField.getBit(i));
    return createPacketModel(decodedBits);
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

const IReceptionPacketModel* Ieee80211LayeredDecoder::createPacketModel(const BitVector& decodedBits) const
{
    double per = -1;
    bool packetErrorless = false; // TODO: compute packet error rate, packetErrorLess
    const cPacket *packet = deserializer->deserialize(decodedBits);
    return new ReceptionPacketModel(packet, per, packetErrorless);
}

Ieee80211LayeredDecoder::~Ieee80211LayeredDecoder()
{
    delete signalDeinterleaver;
    delete descrambler;
    delete signalFECDecoder;
    delete deserializer;
}

} /* namespace physicallayer */
} /* namespace inet */
