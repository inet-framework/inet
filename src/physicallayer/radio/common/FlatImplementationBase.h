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

#ifndef __INET_FLATIMPLEMENTATIONBASE_H_
#define __INET_FLATIMPLEMENTATIONBASE_H_

#include "ImplementationBase.h"
#include "IModulation.h"

class INET_API FlatRadioSignalTransmissionBase : public RadioSignalTransmissionBase
{
    protected:
        const IModulation *modulation;
        const int headerBitLength;
        const int payloadBitLength;
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        FlatRadioSignalTransmissionBase(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth) :
            RadioSignalTransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
            modulation(modulation),
            headerBitLength(headerBitLength),
            payloadBitLength(payloadBitLength),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual const IModulation *getModulation() const { return modulation; }
        virtual int getHeaderBitLength() const { return headerBitLength; }
        virtual int getPayloadBitLength() const { return payloadBitLength; }
        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API FlatRadioSignalReceptionBase : public RadioSignalReceptionBase
{
    protected:
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        FlatRadioSignalReceptionBase(const IRadio *receiver, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz carrierFrequency, Hz bandwidth) :
            RadioSignalReceptionBase(receiver, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API FlatRadioSignalNoiseBase : public RadioSignalNoiseBase
{
    protected:
        const Hz carrierFrequency;
        const Hz bandwidth;

    public:
        FlatRadioSignalNoiseBase(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth) :
            RadioSignalNoiseBase(startTime, endTime),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API FlatRadioSignalTransmitterBase : public RadioSignalTransmitterBase
{
    protected:
        const IModulation *modulation;
        int headerBitLength;
        Hz carrierFrequency;
        Hz bandwidth;

    protected:
        virtual void initialize(int stage);

    public:
        FlatRadioSignalTransmitterBase() :
            modulation(NULL),
            headerBitLength(-1),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN))
        {}

        virtual const IModulation *getModulation() const { return modulation; }

        virtual int getHeaderBitLength() const { return headerBitLength; }
        virtual void setHeaderBitLength(int headerBitLength) { this->headerBitLength = headerBitLength; }

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }

        virtual Hz getBandwidth() const { return bandwidth; }
        virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

#endif
