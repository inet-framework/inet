//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SELFDOCUMENTERFINGERPRINTCALCULATOR_H
#define __INET_SELFDOCUMENTERFINGERPRINTCALCULATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API SelfDocumenterFingerprintCalculator : public cSingleFingerprintCalculator
{
  public:
    virtual SelfDocumenterFingerprintCalculator *dup() const override { return new SelfDocumenterFingerprintCalculator(); }

    virtual void addEvent(cEvent *event) override;
};

} // namespace

#endif

