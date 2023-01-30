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

#ifndef __INET_NEURALNETWORKERRORMODEL_H
#define __INET_NEURALNETWORKERRORMODEL_H

#include <fstream>

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"

#include <fdeep/fdeep.hpp>

namespace inet {

namespace physicallayer {


class INET_API NeuralNetworkErrorModel : public ErrorModelBase
{
  protected:
    const char *modelNameFormat = nullptr;
    std::map<std::string, fdeep::model> models;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual std::vector<float> fillSnirTensor(const ScalarSnir *snir, int timeDivision, int frequencyDivision) const;
    virtual std::vector<float> fillSnirTensor(const DimensionalSnir *snir, int timeDivision, int frequencyDivision) const;

    virtual std::string computeModelName(const ISnir *snir) const;

  public:
    virtual ~NeuralNetworkErrorModel() {  }

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NEURALNETWORKERRORMODEL_H

