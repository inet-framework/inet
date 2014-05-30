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

#include <cmath>
#include "bbn_transmitter.h"
#include "ByteArrayMessage.h"
#include "GrTransmitter.h"

using namespace std;

GrSignal::GrSignal(simtime_t startTime, simtime_t endTime, Hz sampleRate, Hz carrierFrequency)
    : startTime(startTime), endTime(endTime), sampleRate(sampleRate), carrierFrequency(carrierFrequency)
{
    ASSERT(endTime >= startTime);

    size_t numSamples = (size_t)floor(SIMTIME_DBL(endTime - startTime) * sampleRate.get());
    samples.resize(numSamples);
}

GrSignal::GrSignal(simtime_t startTime, simtime_t endTime, const gr_complex *samples, size_t len, Hz sampleRate, Hz carrierFrequency)
    : startTime(startTime), endTime(endTime), samples(samples, samples+len), sampleRate(sampleRate), carrierFrequency(carrierFrequency)
{
}

gr_complex GrSignal::getSampleAt(simtime_t t) const
{
    double delta = 1.0 / sampleRate.get();
    double dblIndex = SIMTIME_DBL(t - startTime) / delta;
    int i = floor(dblIndex);
    if (i < 0 || i >= (int)samples.size())
        return gr_complex();

    if (i+1 < (int)samples.size())
    {
        // linear interpolation
        float alpha = (dblIndex - i);
        return alpha * samples[i] + (1 - alpha) * samples[i+1];
    }
    else
        return samples[i];
}

void GrSignal::addSignal(const GrSignal &interferingSignal)
{
    if (interferingSignal.getCarrierFrequency() != carrierFrequency)
        throw cRuntimeError("GrSignal: combination of multiple frequency signals is not yet implemented.");

    if (interferingSignal.getEndTime() < startTime || endTime < interferingSignal.getStartTime()) // no overlap
        return;

    if (startTime == interferingSignal.getStartTime() && endTime <= interferingSignal.getEndTime() &&
            sampleRate == interferingSignal.getSampleRate())
    {
        // faster computation in case of coinciding time points
        const gr_complex *interferingSamples = interferingSignal.getSamples();
        for (size_t i = 0; i < samples.size(); i++)
            samples[i] += interferingSamples[i];
    }
    else
    {
        // interpolate
        size_t i;
        simtime_t t;
        simtime_t delta = 1.0 / sampleRate.get();
        for (i = 0, t = startTime; i < samples.size(); i++, t += delta)
            samples[i] += interferingSignal.getSampleAt(t);
    }
}

GrSignalTransmission::GrSignalTransmission(const IRadio *radio, const cPacket *macFrame,
                                           const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition,
                                           const GrSignal &physicalSignal) :
    RadioSignalTransmissionBase(radio, macFrame, startTime, endTime, startPosition, endPosition),
    physicalSignal(physicalSignal)
{
}

GrSignalReception::GrSignalReception(const IRadio *receiver, const IRadioSignalTransmission *transmission,
                                     const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition,
                                     const GrSignal &physicalSignal)
    : RadioSignalReceptionBase(receiver, transmission, startTime, endTime, startPosition, endPosition),
      physicalSignal(physicalSignal)
{
}

GrSignalNoise::GrSignalNoise(simtime_t startTime, simtime_t endTime, Hz sampleRate, double amplitude, Hz carrierFrequency)
    : RadioSignalNoiseBase(startTime, endTime),
      physicalSignal(
              startTime,
              endTime,
              generateSamples(startTime, endTime, sampleRate, amplitude),
              numberOfSamples(startTime, endTime, sampleRate),
              sampleRate,
              carrierFrequency)
{
}

size_t GrSignalNoise::numberOfSamples(simtime_t startTime, simtime_t endTime, Hz sampleRate)
{
    return (size_t)floor(SIMTIME_DBL(endTime - startTime) * sampleRate.get());
}

gr_complex *GrSignalNoise::generateSamples(simtime_t startTime, simtime_t endTime, Hz sampleRate, double amplitude)
{
    size_t numSamples = numberOfSamples(startTime, endTime, sampleRate);
    gr_complex *samples = new gr_complex[numSamples];
    // TODO add gaussian noise samples
    return samples;
}
