//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211BERTABLEERRORMODEL_H
#define __INET_IEEE80211BERTABLEERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/BerParseFile.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211BerTableErrorModel : public ErrorModelBase
{
  protected:
    BerParseFile *berTableFile;

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211BerTableErrorModel();
    virtual ~Ieee80211BerTableErrorModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211BerTableErrorModel"; }

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

