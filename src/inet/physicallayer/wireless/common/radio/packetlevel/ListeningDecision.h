//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LISTENINGDECISION_H
#define __INET_LISTENINGDECISION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IListeningDecision.h"

namespace inet {
namespace physicallayer {

class INET_API ListeningDecision : public IListeningDecision, public cObject
{
  protected:
    const IListening *listening;
    const bool isListeningPossible_;

  public:
    ListeningDecision(const IListening *listening, bool isListeningPossible_);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IListening *getListening() const override { return listening; }

    virtual bool isListeningPossible() const override { return isListeningPossible_; }
};

} // namespace physicallayer
} // namespace inet

#endif

