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
    // the result of isNetworkNode() for each module by module ID: 0 is not known yet, 1 is false, 2 is true;
    // the calculator asks it for each packet event, and cModule::getProperties() is a NED lookup each time;
    // a module ID is unique in a run, and Envir creates the calculator again for each run
    std::vector<uint8_t> networkNodeFlags;

  protected:
    // the same as inet::findContainingNode(), but it remembers the result of isNetworkNode() for each module
    cModule *findContainingNode(const cModule *module);

    virtual void parseIngredients(const char *s) override;
    virtual cSingleFingerprintCalculator::FingerprintIngredient validateIngredient(char ch) override;
    virtual bool addEventIngredient(cEvent *event, cSingleFingerprintCalculator::FingerprintIngredient ingredient) override;

  public:
    virtual FingerprintCalculator *dup() const override { return new FingerprintCalculator(); }

    virtual void addEvent(cEvent *event) override;
};

} // namespace

#endif

