//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ReceiverBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"

namespace inet {

namespace physicallayer {

Ieee80211ReceiverBase::Ieee80211ReceiverBase() :
    modeSet(nullptr),
    band(nullptr),
    channel(nullptr)
{
}

Ieee80211ReceiverBase::~Ieee80211ReceiverBase()
{
    delete channel;
}

void Ieee80211ReceiverBase::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opMode = par("opMode");
        setModeSet(*opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr);
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

std::ostream& Ieee80211ReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modeSet, printFieldToString(modeSet, level + 1, evFlags))
               << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return FlatReceiverBase::printToStream(stream, level);
}

void Ieee80211ReceiverBase::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

void Ieee80211ReceiverBase::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211ReceiverBase::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        setCenterFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211ReceiverBase::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}


static void printSnirTensor(const DimensionalSnir *dimensionalSnir)
{
    const INoise *noise = dimensionalSnir->getNoise();
    const IReception *reception = dimensionalSnir->getReception();
    auto transmission = check_and_cast<const DimensionalTransmission *>(reception->getTransmission());
    const Ieee80211DimensionalTransmission *dimensionalTransmission = check_and_cast<const Ieee80211DimensionalTransmission *>(transmission);

    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_INFO << "Reception power begin " << endl;
    EV_INFO << *dimensionalReception->getPower() << endl;
    EV_INFO << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snirFunction = receptionPower->divide(noisePower);

    const Ieee80211OfdmMode *mode = check_and_cast<const Ieee80211OfdmMode *>(dimensionalTransmission->getMode());

    int timeDivision = (reception->getHeaderDuration() + reception->getDataDuration())
        / mode->getSymbolInterval();

    int frequencyDivision = mode->getDataMode()->getNumberOfTotalSubcarriers();

    auto startTime = reception->getHeaderStartTime();
    auto endTime = reception->getDataEndTime();

    auto centerFrequency = transmission->getCenterFrequency();
    auto bandwidth = transmission->getBandwidth();

    // 20MHz is the distance between the centers of the two outermost subcarriers,
    // but each of them sticks out 312.5/2 kHz outside of the 20Mhz,
    // because the null subcarrier is there right at the carrier frequency, and
    // that shifts all 26 (+6 "guard") subcarriers at each side outward by 312.5/2 kHz.

    auto subcarrierWidth = bandwidth / 64.0;

    // TODO: null subcarrier!
    auto startFrequency = transmission->getCenterFrequency() - subcarrierWidth * 26;
    auto endFrequency = transmission->getCenterFrequency() + subcarrierWidth * 26;

    int symbolCount = timeDivision * frequencyDivision;
    std::vector<math::Interval<simsec, Hz>> symbolIntervals;
    for (int i = 0; i < timeDivision; i++) {
        simtime_t symbolStartTime = (startTime * (double)(timeDivision - i) + endTime * (double)i) / timeDivision;
        simtime_t symbolEndTime = (startTime * (double)(timeDivision - i - 1) + endTime * (double)(i + 1)) / timeDivision;
        for (int j = 0; j < frequencyDivision; j++) {
            // TODO: null subcarrier!
            Hz symbolStartFrequency = centerFrequency + subcarrierWidth * (-26 + j);
            Hz symbolEndFrequency = centerFrequency + subcarrierWidth * (-25 + j);

            math::Point<simsec, Hz> symbolStartPoint(simsec(symbolStartTime), symbolStartFrequency);
            math::Point<simsec, Hz> symbolEndPoint(simsec(symbolEndTime), symbolEndFrequency);
            math::Interval<simsec, Hz> symbolInterval(symbolStartPoint, symbolEndPoint, 0b11, 0b00, 0b00);
            symbolIntervals.push_back(symbolInterval);
        }
    }
    math::Point<simsec, Hz> startPoint(simsec(startTime), centerFrequency - bandwidth / 2);
    math::Point<simsec, Hz> endPoint(simsec(endTime), centerFrequency + bandwidth / 2);
    math::Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);


    EV_INFO << std::endl << "SNIR function: " << std::endl;
    snirFunction->printStructure(EV_INFO);

    EV_INFO << std::endl << "Computing SNIR mean tensor for " << symbolCount << " symbols" << std::endl;


    std::vector<float> result;
    result.resize(symbolCount);

    snirFunction->partition(interval, [&] (const math::Interval<simsec, Hz>& i1, const math::IFunction<double, math::Domain<simsec, Hz>> *f1) {
        auto intervalStartTime = std::get<0>(i1.getLower()).get();
        auto intervalEndTime = std::get<0>(i1.getUpper()).get();
        auto intervalStartFrequency = std::get<1>(i1.getLower());
        auto intervalEndFrequency = std::get<1>(i1.getUpper());
        auto startTimeIndex = std::max(0, (int)std::floor((intervalStartTime - startTime).dbl() / (endTime - startTime).dbl() * timeDivision));
        auto endTimeIndex = std::min(timeDivision - 1, (int)std::ceil((intervalEndTime - startTime).dbl() / (endTime - startTime).dbl() * timeDivision));

        auto startFrequencyIndex = std::max(0, (int)std::floor((intervalStartFrequency - startFrequency).get() / (endFrequency - startFrequency).get() * frequencyDivision));
        auto endFrequencyIndex = std::min(frequencyDivision - 1, (int)std::ceil((intervalEndFrequency - startFrequency).get() / (endFrequency - startFrequency).get() * frequencyDivision));
        for (int i = startTimeIndex; i <= endTimeIndex; i++) {
            for (int j = startFrequencyIndex; j <= endFrequencyIndex; j++) {
                int symbolIndex = frequencyDivision * i + j;
                ASSERT(0 <= symbolIndex && symbolIndex < symbolCount);
                auto i2 = symbolIntervals[symbolIndex].getIntersected(i1);
                double v = i2.getVolume() * f1->getMean(i2);
                ASSERT(!std::isnan(v));
                result[symbolIndex] += v;
            }
        }
    });


    double area = (endTime - startTime).dbl() / timeDivision * bandwidth.get() / frequencyDivision;


    for (int i = 0; i < symbolCount; i++) {
        result[i] /= area;
        EV_INFO << result[i] << ", ";
    }
    EV_INFO << std::endl;

}



const IReceptionResult *Ieee80211ReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto transmission = check_and_cast<const Ieee80211TransmissionBase *>(reception->getTransmission());
    auto receptionResult = FlatReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    packet->addTagIfAbsent<Ieee80211ModeInd>()->setMode(transmission->getMode());
    packet->addTagIfAbsent<Ieee80211ChannelInd>()->setChannel(transmission->getChannel());

    DimensionalSnir *dimensionalSnir = check_and_cast<DimensionalSnir *>(const_cast<ISnir *>(snir));

    printSnirTensor(dimensionalSnir);

    return receptionResult;
}

} // namespace physicallayer

} // namespace inet

