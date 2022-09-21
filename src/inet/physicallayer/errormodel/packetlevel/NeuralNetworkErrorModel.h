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

#include "inet/common/Units.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/base/packetlevel/ErrorModelBase.h"
//#include "keras2cpp/model.h"

namespace inet {

namespace physicallayer {

class INET_API NeuralNetworkErrorModel : public ErrorModelBase
{
  protected:
    const char *modelNameFormat = nullptr;
    //std::map<std::string, keras2cpp::Model *> models;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    //virtual void fillSnirTensor(const ScalarSnir *snir, int timeDivision, int frequencyDivision, keras2cpp::Tensor& in) const;
   // virtual void fillSnirTensor(const DimensionalSnir *snir, int timeDivision, int frequencyDivision, keras2cpp::Tensor& in) const;

    virtual std::string computeModelName(const ISnir *snir) const;

  public:
   // virtual ~NeuralNetworkErrorModel() { for (auto it : models) delete it.second; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NEURALNETWORKERRORMODEL_H

