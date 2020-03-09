//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_FINGERPRINTCALCULATOR_H_
#define __INET_FINGERPRINTCALCULATOR_H_

#include "inet/common/INETDefs.h"

namespace inet {

#define INET_FINGERPRINT_INGREDIENTS "~NIDP"

class INET_API FingerprintCalculator : public cSingleFingerprintCalculator
{
  protected:
    enum FingerprintIngredient {
        NETWORK_COMMUNICATION_FILTER = '~',
        NETWORK_NODE_PATH            = 'N',
        NETWORK_INTERFACE_PATH       = 'I',
        PACKET_DATA                  = 'D',
        PROGRESS_CONVERTER_FILTER              = 'P',     // decode cProgress::PACKET_END to packet, filtering out other cProgress messages
    };

  protected:
    bool filterEvents = false;
    bool filterProgress = false;

  protected:
    virtual void parseIngredients(const char *s) override;
    virtual cSingleFingerprintCalculator::FingerprintIngredient validateIngredient(char ch) override;
    virtual bool addEventIngredient(cEvent *event, cSingleFingerprintCalculator::FingerprintIngredient ingredient) override;

  public:
    virtual FingerprintCalculator *dup() const override { return new FingerprintCalculator(); }

    virtual void addEvent(cEvent *event) override;
};

} // namespace

#endif // #ifndef __INET_FINGERPRINTCALCULATOR_H_

