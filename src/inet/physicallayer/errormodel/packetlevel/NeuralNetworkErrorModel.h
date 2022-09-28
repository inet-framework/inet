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
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/base/packetlevel/ErrorModelBase.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"


namespace inet {

namespace physicallayer {

class INET_API NeuralNet {
  public:
  std::vector<uint8_t> modeldata;
  const tflite::Model *model;



  static tflite::AllOpsResolver resolver;

  static tflite::MicroErrorReporter micro_error_reporter;

  tflite::MicroInterpreter *interpreter;

  const size_t arena_size = 64 * 1024 * 1024;
  uint8_t *arena = nullptr;


  NeuralNet(const char *filename) {

      std::ifstream stream(filename, std::ios::in | std::ios::binary);

      modeldata = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
      std::cout << "modeldata length: " << modeldata.size() << std::endl;
      model = tflite::GetModel(modeldata.data());

      if (model->version() != TFLITE_SCHEMA_VERSION) {
        std::cout << "VERSION MISMATCH" << std::endl;
      TF_LITE_REPORT_ERROR(&micro_error_reporter,
          "Model provided is schema version %d not equal "
          "to supported version %d.\n",
          model->version(), TFLITE_SCHEMA_VERSION);
    }


      arena = new uint8_t[arena_size];

      // Build an interpreter to run the model with.
      interpreter = new tflite::MicroInterpreter(model, resolver, arena, arena_size, &micro_error_reporter);

      // Allocate memory from the tensor_arena for the model's tensors.
      TfLiteStatus allocate_status = interpreter->AllocateTensors();

  }

  //float run(const float *input, float *output) { }

  ~NeuralNet() {
    delete interpreter;
    delete[] arena;
  };

};

class INET_API NeuralNetworkErrorModel : public ErrorModelBase
{
  protected:
    const char *modelNameFormat = nullptr;
    std::map<std::string, NeuralNet *> models;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void fillSnirTensor(const ScalarSnir *snir, int timeDivision, int frequencyDivision, TfLiteTensor* in) const;
    virtual void fillSnirTensor(const DimensionalSnir *snir, int timeDivision, int frequencyDivision, TfLiteTensor* i) const;

    virtual std::string computeModelName(const ISnir *snir) const;

  public:
    virtual ~NeuralNetworkErrorModel() { for (auto it : models) delete it.second; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NEURALNETWORKERRORMODEL_H

