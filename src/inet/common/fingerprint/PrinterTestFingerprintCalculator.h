//
// (C) 2005 Vojtech Janota
// (C) 2010 Zoltan Bojthe
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_PRINTERTESTFINGERPRINTCALCULATOR_H
#define __INET_PRINTERTESTFINGERPRINTCALCULATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API PrinterTestFingerprintCalculator : public cSingleFingerprintCalculator
{
  public:
    virtual PrinterTestFingerprintCalculator *dup() const override { return new PrinterTestFingerprintCalculator(); }
    virtual void addEvent(cEvent *event) override;
};

} // namespace inet

#endif // ifndef __INET_PRINTERTESTFINGERPRINTCALCULATOR_H

