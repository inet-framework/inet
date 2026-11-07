//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cmath>
#include <cstdio>
#include <sstream>

#include "inet/common/SimpleModule.h"
#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IObstacleLoss.h"

namespace inet {

class ObstacleLossTest : public SimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
};

Define_Module(ObstacleLossTest);

void ObstacleLossTest::initialize(int stage)
{
    if (stage != INITSTAGE_LAST)
        return;
    auto medium = check_and_cast<physicallayer::IRadioMedium *>(getModuleByPath(par("radioMediumModule")));
    auto obstacleLoss = medium->getObstacleLoss();
    Coord tx(par("txX").doubleValueInUnit("m"), par("txY").doubleValueInUnit("m"), par("txZ").doubleValueInUnit("m"));
    Coord rx(par("rxX").doubleValueInUnit("m"), par("rxY").doubleValueInUnit("m"), par("rxZ").doubleValueInUnit("m"));
    std::istringstream frequencies(par("frequencies").stdstringValue());
    double frequency;
    while (frequencies >> frequency) {
        double loss = obstacleLoss->computeObstacleLoss(Hz(frequency), tx, rx);
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "obstacle loss at %g Hz: %.3f dB", frequency, -10 * std::log10(loss));
        EV << buffer << endl;
    }
}

} // namespace inet
