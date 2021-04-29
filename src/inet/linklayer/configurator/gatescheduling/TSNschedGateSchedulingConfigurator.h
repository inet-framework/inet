//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TSNSCHEDGATESCHEDULINGCONFIGURATOR_H
#define __INET_TSNSCHEDGATESCHEDULINGCONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateSchedulingConfiguratorBase.h"

namespace inet {

class INET_API TSNschedGateSchedulingConfigurator : public GateSchedulingConfiguratorBase
{
  protected:
    virtual std::string getJavaName(std::string name) const;
    virtual void writeJavaCode(const Input& input, std::string fileName) const;
    virtual void compileJavaCode(std::string fileName) const;
    virtual void executeJavaCode(std::string className) const;
    virtual Output *readGateScheduling(std::string directoryName) const;

    virtual Output *computeGateScheduling(const Input& input) const override;
};

} // namespace inet

#endif

