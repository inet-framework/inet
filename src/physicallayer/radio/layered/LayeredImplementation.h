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

#ifndef __INET_LAYEREDIMPLEMENTATION_H_
#define __INET_LAYEREDIMPLEMENTATION_H_

#include "ImplementationBase.h"
#include "ILayeredRadioSignalTransmission.h"
#include "ILayeredRadioSignalReception.h"
#include "ILayeredRadioSignalTransmitter.h"
#include "ILayeredRadioSignalReceiver.h"
#include "IRadioCodec.h"
#include "IRadioModem.h"
#include "IRadioShaper.h"
#include "IRadioConverter.h"

class INET_API RadioSignalAnalogModel : public virtual IRadioSignalAnalogModel
{
    protected:
        const simtime_t duration;

    public:
        RadioSignalAnalogModel(const simtime_t duration) :
            duration(duration)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const simtime_t getDuration() const { return duration; }
};

class INET_API RadioSignalSampleModel : public virtual IRadioSignalSampleModel
{
    protected:
        const int sampleLength;
        const double sampleRate;
        const std::vector<W> *samples;

    public:
        RadioSignalSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples) :
            sampleLength(sampleLength),
            sampleRate(sampleRate),
            samples(samples)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual int getSampleLength() const { return sampleLength; }

        virtual double getSampleRate() const { return sampleRate; }

        virtual const std::vector<W> *getSamples() const { return samples; }
};

class INET_API RadioSignalTransmissionSampleModel : public RadioSignalSampleModel, public virtual IRadioSignalTransmissionSampleModel
{
    public:
        RadioSignalTransmissionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples) :
            RadioSignalSampleModel(sampleLength, sampleRate, samples)
        {}
};

class INET_API RadioSignalReceptionSampleModel : public RadioSignalSampleModel, public virtual IRadioSignalReceptionSampleModel
{
    protected:
        const W rssi;

    public:
        RadioSignalReceptionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples, W rssi) :
            RadioSignalSampleModel(sampleLength, sampleRate, samples),
            rssi(rssi)
        {}

        virtual const W getRSSI() const { return rssi; }
};

class INET_API RadioSignalModulation : public IRadioSignalModulation
{
    public:
        enum Type {
            /** Infrared (IR) (Clause 16) */
            IR,
            /** Frequency-hopping spread spectrum (FHSS) PHY (Clause 14) */
            FHSS,
            /** DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18) */
            DSSS,
            /** ERP-PBCC PHY (19.6) */
            ERP_PBCC,
            /** DSSS-OFDM PHY (19.7) */
            DSSS_OFDM,
            /** ERP-OFDM PHY (19.5) */
            ERP_OFDM,
            /** OFDM PHY (Clause 17) */
            OFDM,
            /** HT PHY (Clause 20) */
            HT

        };

    protected:
        const Type type;
        const int codeWordLength;
        const int constellationSize;

    public:
        RadioSignalModulation(const Type type, int codeWordLength, int constellationSize) :
            type(type),
            codeWordLength(codeWordLength),
            constellationSize(constellationSize)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual Type getType() const { return type; }

        virtual int getCodeWordLength() const { return codeWordLength; }

        virtual int getConstellationSize() const { return constellationSize; }
};

class INET_API RadioSignalSymbolModel : public virtual IRadioSignalSymbolModel
{
    protected:
        const int symbolLength;
        const double symbolRate;
        const std::vector<int> *symbols;
        const IRadioSignalModulation *modulation;

    public:
        RadioSignalSymbolModel(int symbolLength, double symbolRate, const std::vector<int> *symbols, const IRadioSignalModulation *modulation) :
            symbolLength(symbolLength),
            symbolRate(symbolRate),
            symbols(symbols),
            modulation(modulation)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual int getSymbolLength() const { return symbolLength; }

        virtual double getSymbolRate() const { return symbolRate; }

        virtual const std::vector<int> *getSymbols() const { return symbols; }

        virtual const IRadioSignalModulation *getModulation() const { return modulation; }
};

class INET_API RadioSignalTransmissionSymbolModel : public RadioSignalSymbolModel, public virtual IRadioSignalTransmissionSymbolModel
{
    public:
        RadioSignalTransmissionSymbolModel(int symbolLength, double symbolRate, const std::vector<int> *symbols, const IRadioSignalModulation *modulation) :
            RadioSignalSymbolModel(symbolLength, symbolRate, symbols, modulation)
        {}
};

class INET_API RadioSignalReceptionSymbolModel : public RadioSignalSymbolModel, public virtual IRadioSignalReceptionSymbolModel
{
    protected:
        const double ser;
        const double symbolErrorCount;

    public:
        RadioSignalReceptionSymbolModel(int symbolLength, double symbolRate, const std::vector<int> *symbols, const IRadioSignalModulation *modulation, double ser, double symbolErrorCount) :
            RadioSignalSymbolModel(symbolLength, symbolRate, symbols, modulation),
            ser(ser),
            symbolErrorCount(symbolErrorCount)
        {}

        virtual double getSER() const { return ser; }

        virtual int getSymbolErrorCount() const { return symbolErrorCount; }
};

class INET_API ForwardErrorCorrection : public IForwardErrorCorrection
{
    protected:
        const int inputBitLength;
        const int outputBitLength;

    public:
        ForwardErrorCorrection(int inputBitLength, int outputBitLength) :
            inputBitLength(inputBitLength),
            outputBitLength(outputBitLength)
        {}

        virtual void printToStream(std::ostream &stream) const;
};

class INET_API RadioSignalBitModel : public virtual IRadioSignalBitModel
{
    protected:
        const int bitLength;
        const double bitRate;
        const std::vector<bool> *bits;
        const ICyclicRedundancyCheck *cyclicRedundancyCheck;
        const IForwardErrorCorrection *forwardErrorCorrection;

    public:
        RadioSignalBitModel() :
            bitLength(-1),
            bitRate(sNaN),
            bits(NULL),
            cyclicRedundancyCheck(NULL),
            forwardErrorCorrection(NULL)
        {}

        RadioSignalBitModel(int bitLength, double bitRate, const std::vector<bool> *bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection) :
            bitLength(bitLength),
            bitRate(bitRate),
            bits(bits),
            cyclicRedundancyCheck(cyclicRedundancyCheck),
            forwardErrorCorrection(forwardErrorCorrection)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual int getBitLength() const { return bitLength; }

        virtual double getBitRate() const { return bitRate; }

        virtual const std::vector<bool> *getBits() const { return bits; }

        virtual const ICyclicRedundancyCheck *getCyclicRedundancyCheck() const { return cyclicRedundancyCheck; }

        virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
};

class INET_API RadioSignalTransmissionBitModel : public RadioSignalBitModel, public virtual IRadioSignalTransmissionBitModel
{
    public:
        RadioSignalTransmissionBitModel() :
            RadioSignalBitModel()
        {}

        RadioSignalTransmissionBitModel(int bitLength, double bitRate, const std::vector<bool> *bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection) :
            RadioSignalBitModel(bitLength, bitRate, bits, cyclicRedundancyCheck, forwardErrorCorrection)
        {}
};

class INET_API RadioSignalReceptionBitModel : public RadioSignalBitModel, public virtual IRadioSignalReceptionBitModel
{
    protected:
        const double ber;
        const int bitErrorCount;

    public:
        RadioSignalReceptionBitModel() :
            RadioSignalBitModel(),
            ber(sNaN),
            bitErrorCount(-1)
        {}

        RadioSignalReceptionBitModel(int bitLength, double bitRate, const std::vector<bool> *bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection, double ber, int bitErrorCount) :
            RadioSignalBitModel(bitLength, bitRate, bits, cyclicRedundancyCheck, forwardErrorCorrection),
            ber(ber),
            bitErrorCount(bitErrorCount)
        {}

        virtual double getBER() const { return ber; }

        virtual int getBitErrorCount() const { return bitErrorCount; }
};

class INET_API RadioSignalPacketModel : public virtual IRadioSignalPacketModel
{
    protected:
        const cPacket *packet;

    public:
        RadioSignalPacketModel() :
            packet(NULL)
        {}

        RadioSignalPacketModel(const cPacket *packet) :
            packet(packet)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const cPacket *getPacket() const { return packet; }
};

class INET_API RadioSignalTransmissionPacketModel : public RadioSignalPacketModel, public virtual IRadioSignalTransmissionPacketModel
{
    public:
        RadioSignalTransmissionPacketModel() :
            RadioSignalPacketModel()
        {}

        RadioSignalTransmissionPacketModel(const cPacket *packet) :
            RadioSignalPacketModel(packet)
        {}
};

class INET_API RadioSignalReceptionPacketModel : public RadioSignalPacketModel, public IRadioSignalReceptionPacketModel
{
    protected:
        const double per;
        const bool packetErrorless;

    public:
        RadioSignalReceptionPacketModel() :
            RadioSignalPacketModel(),
            per(sNaN),
            packetErrorless(false)
        {}

        RadioSignalReceptionPacketModel(const cPacket *packet, double per, bool packetErrorless) :
            RadioSignalPacketModel(packet),
            per(per),
            packetErrorless(packetErrorless)
        {}

        virtual double getPER() const { return per; }

        virtual bool isPacketErrorless() const { return packetErrorless; }
};

class INET_API RadioShaper : public IRadioShaper
{
    protected:
        const int samplePerSymbol;

    public:
        RadioShaper(int samplePerSymbol) :
            samplePerSymbol(samplePerSymbol)
        {}

        virtual const IRadioSignalTransmissionSampleModel *shape(const IRadioSignalTransmissionSymbolModel *symbolModel) const
        {
            const int sampleLength = symbolModel->getSymbolLength() * samplePerSymbol;
            const double sampleRate = symbolModel->getSymbolRate() * samplePerSymbol;
            return new RadioSignalTransmissionSampleModel(sampleLength, sampleRate, NULL);
        }

        virtual const IRadioSignalReceptionSymbolModel *filter(const IRadioSignalReceptionSampleModel *sampleModel) const
        {
            const int symbolLength = sampleModel->getSampleLength() / samplePerSymbol;
            const double symbolRate = sampleModel->getSampleRate() / samplePerSymbol;
            return new RadioSignalReceptionSymbolModel(symbolLength, symbolRate, NULL, NULL, 0, 0);
        }
};

class INET_API RadioModem : public IRadioModem
{
    protected:
        int preambleSymbolLength;
        const IRadioSignalModulation *modulation;

    public:
        RadioModem(int preambleSymbolLength, const IRadioSignalModulation *modulation) :
            preambleSymbolLength(preambleSymbolLength),
            modulation(modulation)
        {}

        virtual const IRadioSignalTransmissionSymbolModel *modulate(const IRadioSignalTransmissionBitModel *bitModel) const
        {
            const int codeWordLength = modulation->getCodeWordLength();
            const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + codeWordLength - 1) / codeWordLength;
            const double symbolRate = bitModel->getBitRate() / codeWordLength;
            return new RadioSignalTransmissionSymbolModel(symbolLength, symbolRate, NULL, modulation);
        }

        virtual const IRadioSignalReceptionBitModel *demodulate(const IRadioSignalReceptionSymbolModel *symbolModel) const
        {
            const int codeWordLength = modulation->getCodeWordLength();
            const int bitLength = (symbolModel->getSymbolLength() - preambleSymbolLength) * codeWordLength; // TODO: -
            const double bitRate = symbolModel->getSymbolRate() * codeWordLength;
            return new RadioSignalReceptionBitModel(bitLength, bitRate, NULL, NULL, NULL, 0, 0);
        }
};

class INET_API RadioCodec : public IRadioCodec
{
    protected:
        double bitRate;
        int headerBitLength;
        const IForwardErrorCorrection *forwardErrorCorrection;

    public:
        RadioCodec(double bitRate, int headerBitLength, const IForwardErrorCorrection *forwardErrorCorrection) :
            bitRate(bitRate),
            headerBitLength(headerBitLength),
            forwardErrorCorrection(forwardErrorCorrection)
        {}

        virtual const IRadioSignalTransmissionBitModel *encode(const IRadioSignalTransmissionPacketModel *packetModel) const
        {
            const int bitLength = headerBitLength + packetModel->getPacket()->getBitLength();
            return new RadioSignalTransmissionBitModel(bitLength, bitRate, NULL, NULL, forwardErrorCorrection);
        }

        virtual const IRadioSignalReceptionPacketModel *decode(const IRadioSignalReceptionBitModel *bitModel) const
        {
            return new RadioSignalReceptionPacketModel(NULL, 0, true);
        }
};

class INET_API ScalarRadioSignalAnalogModel : public RadioSignalAnalogModel
{
    protected:
        const W power;
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        ScalarRadioSignalAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth) :
            RadioSignalAnalogModel(duration),
            power(power),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual W getPower() const { return power; }
        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API ScalarRadioSignalTransmissionAnalogModel : public ScalarRadioSignalAnalogModel, public virtual IRadioSignalTransmissionAnalogModel
{
    public:
        ScalarRadioSignalTransmissionAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth) :
            ScalarRadioSignalAnalogModel(duration, power, carrierFrequency, bandwidth)
        {}
};

class INET_API ScalarRadioSignalReceptionAnalogModel : public ScalarRadioSignalAnalogModel, public virtual IRadioSignalReceptionAnalogModel
{
    protected:
        const double snir;

    public:
        ScalarRadioSignalReceptionAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth, double snir) :
            ScalarRadioSignalAnalogModel(duration, power, carrierFrequency, bandwidth),
            snir(snir)
        {}

        /**
         * Returns the signal to noise plus interference ratio.
         */
        virtual double getSNIR() const { return snir; }
};

class INET_API ScalarRadioConverter : public IRadioConverter
{
    protected:
        W power;
        // TODO: why carrierFrequency and bandwidth here? why not in the shaper
        Hz carrierFrequency;
        Hz bandwidth;
        double sampleRate;

    public:
        ScalarRadioConverter() :
            power(W(sNaN)),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN)),
            sampleRate(sNaN)
        {}

        ScalarRadioConverter(W power, Hz carrierFrequency, Hz bandwidth, double sampleRate) :
            power(power),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth),
            sampleRate(sampleRate)
        {}

        virtual const IRadioSignalTransmissionAnalogModel *convertDigitalToAnalog(const IRadioSignalTransmissionSampleModel *sampleModel) const
        {
            const simtime_t duration = sampleModel->getSampleLength() / sampleModel->getSampleRate();
            return new ScalarRadioSignalTransmissionAnalogModel(duration, power, carrierFrequency, bandwidth);
        }

        virtual const IRadioSignalReceptionSampleModel *convertAnalogToDigital(const IRadioSignalReceptionAnalogModel *analogModel) const
        {
            const simtime_t duration = analogModel->getDuration();
            const int sampleLength = std::ceil(duration.dbl() / sampleRate);
            return new RadioSignalReceptionSampleModel(sampleLength, sampleRate, NULL, W(0));
        }
};

class INET_API LayeredRadioSignalTransmission : public RadioSignalTransmissionBase, public virtual ILayeredRadioSignalTransmission
{
    protected:
        const IRadioSignalTransmissionPacketModel *packetModel;
        const IRadioSignalTransmissionBitModel    *bitModel;
        const IRadioSignalTransmissionSymbolModel *symbolModel;
        const IRadioSignalTransmissionSampleModel *sampleModel;
        const IRadioSignalTransmissionAnalogModel *analogModel;

    public:
        LayeredRadioSignalTransmission(const IRadioSignalTransmissionPacketModel *packetModel, const IRadioSignalTransmissionBitModel *bitModel, const IRadioSignalTransmissionSymbolModel *symbolModel, const IRadioSignalTransmissionSampleModel *sampleModel, const IRadioSignalTransmissionAnalogModel *analogModel, const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) :
            RadioSignalTransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition),
            packetModel(packetModel),
            bitModel(bitModel),
            symbolModel(symbolModel),
            sampleModel(sampleModel),
            analogModel(analogModel)
        {}

        virtual const IRadioSignalTransmissionPacketModel *getPacketModel() const { return packetModel; }
        virtual const IRadioSignalTransmissionBitModel    *getBitModel()    const { return bitModel; }
        virtual const IRadioSignalTransmissionSymbolModel *getSymbolModel() const { return symbolModel; }
        virtual const IRadioSignalTransmissionSampleModel *getSampleModel() const { return sampleModel; }
        virtual const IRadioSignalTransmissionAnalogModel *getAnalogModel() const { return analogModel; }
};

class INET_API LayeredRadioSignalReception : public RadioSignalReceptionBase, public virtual ILayeredRadioSignalReception
{
    protected:
        const IRadioSignalReceptionPacketModel *packetModel;
        const IRadioSignalReceptionBitModel    *bitModel;
        const IRadioSignalReceptionSymbolModel *symbolModel;
        const IRadioSignalReceptionSampleModel *sampleModel;
        const IRadioSignalReceptionAnalogModel *analogModel;

    public:
        LayeredRadioSignalReception(const IRadioSignalReceptionPacketModel *packetModel, const IRadioSignalReceptionBitModel *bitModel, const IRadioSignalReceptionSymbolModel *symbolModel, const IRadioSignalReceptionSampleModel *sampleModel, const IRadioSignalReceptionAnalogModel *analogModel, const IRadio *radio, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) :
            RadioSignalReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition),
            packetModel(packetModel),
            bitModel(bitModel),
            symbolModel(symbolModel),
            sampleModel(sampleModel),
            analogModel(analogModel)
        {}

        virtual const IRadioSignalReceptionPacketModel *getPacketModel() const { return packetModel; }
        virtual const IRadioSignalReceptionBitModel    *getBitModel()    const { return bitModel; }
        virtual const IRadioSignalReceptionSymbolModel *getSymbolModel() const { return symbolModel; }
        virtual const IRadioSignalReceptionSampleModel *getSampleModel() const { return sampleModel; }
        virtual const IRadioSignalReceptionAnalogModel *getAnalogModel() const { return analogModel; }
};

class INET_API LayeredRadioSignalTransmitter : public RadioSignalTransmitterBase
{
    protected:
        const IRadioCodec *encoder;
        const IRadioModem *modulator;
        const IRadioShaper *pulseShaper;
        const IRadioConverter *digitalAnalogConverter;

    public:
        LayeredRadioSignalTransmitter(const IRadioCodec *encoder, const IRadioModem *modulator, const IRadioShaper *pulseShaper, const IRadioConverter *digitalAnalogConverter) :
            encoder(encoder),
            modulator(modulator),
            pulseShaper(pulseShaper),
            digitalAnalogConverter(digitalAnalogConverter)
        {}

        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const = 0;

        virtual const IRadioCodec *getEncoder() const { return encoder; }
        virtual const IRadioModem *getModulator() const { return modulator; }
        virtual const IRadioShaper *getPulseShaper() const{ return pulseShaper; }
        virtual const IRadioConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
};

class INET_API LayeredRadioSignalReceiver : public RadioSignalReceiverBase, public virtual ILayeredRadioSignalReceiver
{
    protected:
        const IRadioCodec *decoder;
        const IRadioModem *demodulator;
        const IRadioShaper *pulseFilter;
        const IRadioConverter *analogDigitalConverter;

    public:
        LayeredRadioSignalReceiver(const IRadioCodec *decoder, const IRadioModem *demodulator, const IRadioShaper *pulseFilter, const IRadioConverter *analogDigitalConverter) :
            decoder(decoder),
            demodulator(demodulator),
            pulseFilter(pulseFilter),
            analogDigitalConverter(analogDigitalConverter)
        {}

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;

        virtual const IRadioCodec *getDecoder() const { return decoder; }
        virtual const IRadioModem *getDemodulator() const { return demodulator; }
        virtual const IRadioShaper *getPulseFilter() const { return pulseFilter; }
        virtual const IRadioConverter *getAnalogDigitalConverter() const { return analogDigitalConverter; }
};

#endif
