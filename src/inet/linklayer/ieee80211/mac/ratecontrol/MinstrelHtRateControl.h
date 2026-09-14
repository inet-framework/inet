//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MINSTRELHTRATECONTROL_H
#define __INET_MINSTRELHTRATECONTROL_H

#include <array>

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

/** Minstrel-HT's normal-ACK HT/VHT policy; see the NED type for model limits. */
class INET_API MinstrelHtRateControl : public RateControlBase
{
  protected:
    static constexpr int PROBABILITY_SCALE = 4096;
    static constexpr int SAMPLE_COLUMNS = 10;

    struct Rate {
        const physicallayer::IIeee80211Mode *mode = nullptr;
        int group = -1;
        int probability = 0; // Q12 probability, as in the reference
        int skippedIntervals = 0;
        uint64_t attempts = 0;
        uint64_t successes = 0;
        bool measured = false;
        simtime_t dataDuration;
        simtime_t exchangeDuration;
        double throughput = 0; // expected successful reference packets/second
    };

    struct Group {
        std::vector<int> rates;
        std::array<std::vector<int>, SAMPLE_COLUMNS> samples;
        int column = 0;
        int index = 0;
        int bestProbability = 0;
        std::array<int, 2> bestThroughput = {};
    };

    struct State {
        MacAddress address;
        uint64_t generation = 0;
        std::vector<Rate> rates;
        std::vector<Group> groups;
        std::array<int, 2> bestThroughput = {};
        int bestProbability = 0;
        int sampleGroup = 0;
        int sampleWait = 0;
        int sampleTries = 4;
        int sampleCount = 0;
        int slowSamples = 0;
        simtime_t lastUpdate;
    };

    std::map<MacAddress, State> stations;
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<physicallayer::IRadio> radio;
    uint64_t nextGeneration = 0;
    simtime_t interval;
    simtime_t retrySegmentDuration;
    b packetLength;
    Hz maxChannelWidth;
    int maxRetryCount = 0;
    int maxNumSpatialStreams = -1;
    bool probeEnabled = true;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void resetRateControl() override;

    // Protected computation seams allow reference-vector module tests.
    State& getState(const MacAddress& receiver);
    std::vector<const physicallayer::IIeee80211Mode *> getSupportedModes(const MacAddress& receiver) const;
    const physicallayer::IIeee80211Mode *getLegacyMode() const;
    void initializeState(State& state, const std::vector<const physicallayer::IIeee80211Mode *>& modes);
    void updateStatistics(State& state);
    void rankRates(State& state);
    int selectSample(State& state);
    int computeRetryCount(const Rate& rate) const;
    void downgradeRate(State& state, int slot);
    static double computeThroughput(int probability, simtime_t duration);

  public:
    using RateControlBase::frameTransmitted;
    virtual const physicallayer::IIeee80211Mode *getRate(const MacAddress& receiver) override;
    virtual const physicallayer::IIeee80211Mode *getRateForFrame(Packet *frame) override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool successful, bool givenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} // namespace ieee80211
} // namespace inet

#endif
