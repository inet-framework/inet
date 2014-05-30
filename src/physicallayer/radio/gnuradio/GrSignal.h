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

#ifndef __INET_GRSIGNAL_H_
#define __INET_GRSIGNAL_H_

#include <gnuradio/gr_complex.h>
#include "IRadioSignalTransmission.h"
#include "ImplementationBase.h"


// Operations:
// - combine more signals that have different sample rates, carrier frequencies, and start times
//
class INET_API GrSignal
{
        const simtime_t startTime;
        const simtime_t endTime;
        std::vector<gr_complex> samples; // amplitude + phase
        const Hz sampleRate;
        const Hz carrierFrequency;

    public:
        GrSignal(simtime_t startTime, simtime_t endTime, Hz sampleRate, Hz carrierFrequency);
        GrSignal(simtime_t startTime, simtime_t endTime, const gr_complex *samples, size_t len, Hz sampleRate, Hz carrierFrequency);

        simtime_t getStartTime() const { return startTime; }
        simtime_t getEndTime() const { return endTime; }
        const gr_complex *getSamples() const { return samples.data(); }
        size_t getNumSamples() const { return samples.size(); }
        gr_complex getSampleAt(simtime_t t) const;
        Hz getSampleRate() const { return sampleRate; }
        Hz getCarrierFrequency() const { return carrierFrequency; }
        simtime_t getDuration() const { return endTime - startTime; }
        void addSignal(const GrSignal &interferingSignal);
};

class INET_API GrSignalTransmission : public RadioSignalTransmissionBase
{
        GrSignal physicalSignal;
    public:
        GrSignalTransmission(const IRadio *radio, const cPacket *macFrame,
                             const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition,
                             const GrSignal &physicalSignal);

        virtual ~GrSignalTransmission() {}

        const GrSignal &getPhysicalSignal() const { return physicalSignal; }
};

class INET_API GrSignalReception : public RadioSignalReceptionBase
{
        GrSignal physicalSignal;
    public:
        GrSignalReception(const IRadio *receiver, const IRadioSignalTransmission *transmission,
                          const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition,
                          const GrSignal &physicalSignal);

        const GrSignal &getPhysicalSignal() const { return physicalSignal; }
};

class INET_API GrSignalNoise : public RadioSignalNoiseBase
{
        GrSignal physicalSignal;
        static size_t numberOfSamples(simtime_t startTime, simtime_t endTime, Hz sampleRate);
        static gr_complex *generateSamples(simtime_t startTime, simtime_t endTime, Hz sampleRate, double amplitude);
    public:
        GrSignalNoise(simtime_t startTime, simtime_t endTime, Hz sampleRate, double amplitude, Hz carrierFrequency);
        const GrSignal &getPhysicalSignal() const { return physicalSignal; }
};

#endif
