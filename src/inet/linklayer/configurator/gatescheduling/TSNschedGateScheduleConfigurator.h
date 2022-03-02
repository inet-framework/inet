//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TSNSCHEDGATESCHEDULECONFIGURATOR_H
#define __INET_TSNSCHEDGATESCHEDULECONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

namespace inet {

class INET_API TSNschedGateScheduleConfigurator : public GateScheduleConfiguratorBase
{
  protected:
    virtual cValueMap *convertInputToJson(const Input& input) const;
    virtual Output *convertJsonToOutput(const Input& input, const cValueMap *json) const;

    virtual void writeInputToFile(const Input& input, std::string fileName) const;
    virtual Output *readOutputFromFile(const Input& input, std::string fileName) const;

    virtual void executeTSNsched(std::string fileName) const;

    virtual Output *computeGateScheduling(const Input& input) const override;
};

} // namespace inet

#endif

