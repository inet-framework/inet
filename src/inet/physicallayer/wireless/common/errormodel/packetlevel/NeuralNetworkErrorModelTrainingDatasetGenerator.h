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

#ifndef __INET_NEURALNETWORKERRORMODELTRAININGDATASETGENERATOR_H
#define __INET_NEURALNETWORKERRORMODELTRAININGDATASETGENERATOR_H

#include <fstream>
#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"

namespace inet {
namespace physicallayer {

class INET_API NeuralNetworkErrorModelTrainingDatasetGenerator : public cSimpleModule
{
  protected:
    const char *packetNameFormat = nullptr;
    int packetCount = -1;
    int repeatCount = -1;
    const Radio *radio = nullptr;
    const IRadioMedium *radioMedium = nullptr;

    pid_t pid;
    std::ofstream traningDataset;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void openTrainingDataset();
    virtual void closeTrainingDataset();

    virtual void generateTrainingDataset();

    virtual Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> createNoisePowerFunction(W noisePowerMean, W noisePowerStddev, int frequencyDivision, int timeDivision, simtime_t startTime, simtime_t endTime, Hz startFrequency, Hz endFrequency);

  public:
    virtual ~NeuralNetworkErrorModelTrainingDatasetGenerator() { closeTrainingDataset(); }
};

} // namespace physicallayer
} // namespace inet

#endif // __INET_NEURALNETWORKERRORMODELTRAININGDATASETGENERATOR_H

