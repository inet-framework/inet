//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// TODO update header guard macro
#ifndef __INET_NEURALNETWORKERRORMODELEVALUATOR_H
#define __INET_NEURALNETWORKERRORMODELEVALUATOR_H

#include <fstream>
#include "inet/physicallayer/common/packetlevel/Radio.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211RadioErrorModelEvaluator : public cSimpleModule
{
  protected:
    int repeatCount = -1;
    const Radio *radio = nullptr;
    const IRadioMedium *radioMedium = nullptr;

    std::ofstream snirsFile;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void openSnirsFile();
    virtual void closeSnirsFile();

    virtual void evaluateErrorModel();

    virtual Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> assembleNoisePowerFunction(std::vector<double> snirs, int frequencyDivision, int timeDivision, simtime_t startTime, simtime_t endTime, Hz startFrequency, Hz endFrequency);

  public:
    virtual ~Ieee80211RadioErrorModelEvaluator() { closeSnirsFile(); }
};

} // namespace physicallayer
} // namespace inet

#endif // __INET_NEURALNETWORKERRORMODELEVALUATOR_H

