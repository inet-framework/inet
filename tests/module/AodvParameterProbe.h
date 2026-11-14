// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_PARAMETER_TEST_PROBE_H
#define __AODV_PARAMETER_TEST_PROBE_H
#include "inet/common/SimpleModule.h"
using namespace inet;
class AodvParameterProbe : public SimpleModule
{
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            auto aodv = getModuleByPath("^.dut.aodv");
            for (auto pair : {std::make_pair("expectedBlacklist", "blacklistTimeout"),
                    std::make_pair("expectedMyRoute", "myRouteTimeout")}) {
                double expected = par(pair.first).doubleValue();
                if (expected >= 0)
                    ASSERT(std::abs(aodv->par(pair.second).doubleValue() - expected) < 1e-9);
            }
        }
    }
    void finish() override { std::cout << "AODV ParameterValidation: PASS\n"; }
};
Define_Module(AodvParameterProbe);
#endif
