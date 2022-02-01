//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FINGERPRINTCALCULATOR_H
#define __INET_FINGERPRINTCALCULATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

#define INET_FINGERPRINT_INGREDIENTS    "~UNID"

class INET_API FingerprintCalculator : public cSingleFingerprintCalculator
{
  protected:
    enum FingerprintIngredient {
        NETWORK_COMMUNICATION_FILTER = '~',
        PACKET_UPDATE_FILTER         = 'U',
        NETWORK_NODE_PATH            = 'N',
        NETWORK_INTERFACE_PATH       = 'I',
        PACKET_DATA                  = 'D',
    };

  protected:
    bool networkCommunicationFilter = false;
    bool packetUpdateFilter = false;

  protected:
    virtual void parseIngredients(const char *s) override;
    virtual cSingleFingerprintCalculator::FingerprintIngredient validateIngredient(char ch) override;
    virtual bool addEventIngredient(cEvent *event, cSingleFingerprintCalculator::FingerprintIngredient ingredient) override;

  public:
    virtual FingerprintCalculator *dup() const override { return new FingerprintCalculator(); }

    virtual void addEvent(cEvent *event) override;
};

} // namespace

#endif

