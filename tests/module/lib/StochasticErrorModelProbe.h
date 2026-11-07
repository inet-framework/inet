//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STOCHASTICERRORMODELPROBE_H
#define __INET_STOCHASTICERRORMODELPROBE_H

#include <map>
#include <string>

#include "inet/physicallayer/wireless/common/errormodel/packetlevel/StochasticErrorModel.h"

namespace inet {

/**
 * A StochasticErrorModel that records the smallest and largest value that it
 * returned for each rate and signal part as scalars. On every request for the
 * packet error rate of the whole packet it also computes the rates of the
 * header and data parts, which the receivers do not request unless they
 * receive the parts separately.
 */
class StochasticErrorModelProbe : public physicallayer::StochasticErrorModel
{
  protected:
    struct Range
    {
        double min = INFINITY;
        double max = -INFINITY;
        long count = 0;
        long nanCount = 0;
        void add(double value);
    };

    mutable std::map<std::string, Range> ranges;

  protected:
    virtual void finish() override;
    void record(const char *rate, physicallayer::IRadioSignal::SignalPart part, double value) const;

  public:
    virtual double computePacketErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override;
};

} // namespace inet

#endif
