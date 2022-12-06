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

#include "inet/common/StringFormat.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/errormodel/packetlevel/NeuralNetworkErrorModel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

#include <fstream>

namespace inet {

namespace physicallayer {


// Write an std::string to EV.
inline static void EV_logger(const std::string& str)
{
    EV << str << std::flush;
}



Define_Module(NeuralNetworkErrorModel);

static std::string getModulationName(const IModulation *modulation)
{
    std::string s = modulation->getClassName();
    int prefixLength = strlen("inet::physicallayer::");
    return s.substr(prefixLength, s.length() - prefixLength - 10);
}

void NeuralNetworkErrorModel::initialize(int stage)
{
    ErrorModelBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        modelNameFormat = par("modelNameFormat");
        // TODO: make this database global to allow sharing among module instances
        cStringTokenizer tokenizer(par("neuralNetworkModelFilenames"), ",");
        while (tokenizer.hasMoreTokens()) {
            std::string filename = tokenizer.nextToken();

            auto index = filename.rfind('/');
            if (index == std::string::npos)
                index = 0;
            else
                index++;
            auto modelName = filename.substr(index, filename.rfind('.') - index);
            EV_INFO << "Loading neural network model " << modelName << std::endl;

            std::string modelFile = "results/" + modelName + ".json";
            std::cout << "modelfile: " << modelFile << std::endl;
            models.insert({modelName, fdeep::load_model(modelFile, true, EV_logger)});
        }
    }

}

std::ostream& NeuralNetworkErrorModel::printToStream(std::ostream& stream, int level) const
{
    return stream << "NeuralNetworkErrorModel";
}

double NeuralNetworkErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();

    auto name = computeModelName(snir);
    auto it = models.find(name);
    if (it == models.end())
        throw cRuntimeError("Unkown neural network model %s", name.c_str());
    auto model = it->second;
    auto reception = snir->getReception();
    auto transmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
    int timeDivision = (transmission->getHeaderDuration() + transmission->getDataDuration()) / transmission->getSymbolTime();
    int frequencyDivision;
    auto modulation = transmission->getModulation();
    if (auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation))
        frequencyDivision = ofdmModulation->getNumSubcarriers();
    else
        frequencyDivision = 1;

    std::vector<float> input;
    if (auto scalarSnir = dynamic_cast<const ScalarSnir *>(snir))
        input = fillSnirTensor(scalarSnir, timeDivision, frequencyDivision);
    else if (auto dimensionalSnir = dynamic_cast<const DimensionalSnir *>(snir))
        input = fillSnirTensor(dimensionalSnir, timeDivision, frequencyDivision);
    else
        throw cRuntimeError("Unknown SNIR representation");
    EV_TRACE << "Input tensor:";

    for (int i = 0; i < input.size(); i++) {
        if (i != 0)
            EV_TRACE << ", ";
        EV_TRACE << input[i];
    }
    EV_TRACE << std::endl;

    const auto result = model.predict({
        fdeep::tensor(fdeep::tensor_shape(timeDivision, frequencyDivision), input)
    });

    auto predicted = result[0].get_ignore_rank(fdeep::tensor_pos(0));

    double packetErrorRate = std::min(1.0f, std::max(0.0f, predicted));
    EV_DEBUG << "Computed packet error rate is " << packetErrorRate << std::endl;
    return packetErrorRate;

}

std::vector<float> NeuralNetworkErrorModel::fillSnirTensor(const ScalarSnir *snir, int timeDivision, int frequencyDivision) const
{
    Enter_Method_Silent();
    auto scalarSnir = check_and_cast<const ScalarSnir *>(snir);
    auto reception = snir->getReception();
    auto startTime = reception->getHeaderStartTime();
    auto endTime = reception->getDataEndTime();
    int symbolCount = timeDivision * frequencyDivision;
    EV_DEBUG << "Computing SNIR mean tensor for " << symbolCount << " symbols" << std::endl;

    std::vector<float> result;
    result.reserve(symbolCount);

    for (int i = 0; i < timeDivision; i++) {
        simtime_t symbolStartTime = (startTime * (double)(timeDivision - i) + endTime * (double)i) / timeDivision;
        simtime_t symbolEndTime = (startTime * (double)(timeDivision - i - 1) + endTime * (double)(i + 1)) / timeDivision;
        double snirMean = scalarSnir->computeMean(symbolStartTime, symbolEndTime);
        for (int j = 0; j < frequencyDivision; j++) {
            result.push_back(snirMean);
            EV_TRACE << snirMean << ", " << std::endl;
        }
    }
    EV_TRACE << std::endl;

    return result;
}

std::vector<float> NeuralNetworkErrorModel::fillSnirTensor(const DimensionalSnir *snir, int timeDivision, int frequencyDivision) const
{
    Enter_Method_Silent();
    auto dimensionalSnir = check_and_cast<const DimensionalSnir *>(snir);
    auto snirFunction = dimensionalSnir->getSnir();
    auto reception = snir->getReception();
    auto startTime = reception->getHeaderStartTime();
    auto endTime = reception->getDataEndTime();
    auto transmission = check_and_cast<const DimensionalTransmission *>(reception->getTransmission());
    auto centerFrequency = transmission->getCenterFrequency();
    auto bandwidth = kHz(312.5) * 53; // transmission->getBandwidth();
    auto startFrequency = centerFrequency - bandwidth / 2;
    auto endFrequency = centerFrequency + bandwidth / 2;
    int symbolCount = timeDivision * frequencyDivision;
    std::vector<math::Interval<simsec, Hz>> symbolIntervals;
    for (int i = 0; i < timeDivision; i++) {
        simtime_t symbolStartTime = (startTime * (double)(timeDivision - i) + endTime * (double)i) / timeDivision;
        simtime_t symbolEndTime = (startTime * (double)(timeDivision - i - 1) + endTime * (double)(i + 1)) / timeDivision;
        for (int j = 0; j < frequencyDivision; j++) {
            Hz symbolStartFrequency = (startFrequency * (double)(frequencyDivision - j) + endFrequency * (double)j) / frequencyDivision;
            Hz symbolEndFrequency = (startFrequency * (double)(frequencyDivision - j - 1) + endFrequency * (double)(j + 1)) / frequencyDivision;
            math::Point<simsec, Hz> symbolStartPoint(simsec(symbolStartTime), symbolStartFrequency);
            math::Point<simsec, Hz> symbolEndPoint(simsec(symbolEndTime), symbolEndFrequency);
            math::Interval<simsec, Hz> symbolInterval(symbolStartPoint, symbolEndPoint, 0b11, 0b00, 0b00);
            symbolIntervals.push_back(symbolInterval);
        }
    }
    math::Point<simsec, Hz> startPoint(simsec(startTime), centerFrequency - bandwidth / 2);
    math::Point<simsec, Hz> endPoint(simsec(endTime), centerFrequency + bandwidth / 2);
    math::Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);


    EV_TRACE << std::endl << "SNIR function: " << std::endl;
    snirFunction->printStructure(EV_TRACE);
    EV_DEBUG << "Computing SNIR mean tensor for " << symbolCount << " symbols" << std::endl;


    std::vector<float> result;
    result.resize(symbolCount);

    snirFunction->partition(interval, [&] (const math::Interval<simsec, Hz>& i1, const math::IFunction<double, math::Domain<simsec, Hz>> *f1) {
        auto intervalStartTime = std::get<0>(i1.getLower()).get();
        auto intervalEndTime = std::get<0>(i1.getUpper()).get();
        auto intervalStartFrequency = std::get<1>(i1.getLower());
        auto intervalEndFrequency = std::get<1>(i1.getUpper());
        auto startTimeIndex = std::max(0, (int)std::floor((intervalStartTime - startTime).dbl() / (endTime - startTime).dbl() * timeDivision));
        auto endTimeIndex = std::min(timeDivision - 1, (int)std::ceil((intervalEndTime - startTime).dbl() / (endTime - startTime).dbl() * timeDivision));
        auto startFrequencyIndex = std::max(0, (int)std::floor((intervalStartFrequency - startFrequency).get() / (endFrequency - startFrequency).get() * frequencyDivision));
        auto endFrequencyIndex = std::min(frequencyDivision - 1, (int)std::ceil((intervalEndFrequency - startFrequency).get() / (endFrequency - startFrequency).get() * frequencyDivision));
        for (int i = startTimeIndex; i <= endTimeIndex; i++) {
            for (int j = startFrequencyIndex; j <= endFrequencyIndex; j++) {
                int symbolIndex = frequencyDivision * i + j;
                ASSERT(0 <= symbolIndex && symbolIndex < symbolCount);
                auto i2 = symbolIntervals[symbolIndex].getIntersected(i1);
                double v = i2.getVolume() * f1->getMean(i2);
                ASSERT(!std::isnan(v));
                result[symbolIndex] += v;
            }
        }
    });


    double area = (endTime - startTime).dbl() / timeDivision * bandwidth.get() / frequencyDivision;


    for (int i = 0; i < symbolCount; i++) {
        result[i] /= area;
        EV_TRACE << result[i] << ", ";
    }
    EV_TRACE << std::endl;

    return result;
}

double NeuralNetworkErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

double NeuralNetworkErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

std::string NeuralNetworkErrorModel::computeModelName(const ISnir *snir) const
{
    // KLUDGE
    return "Ieee80211Radio_24Mbps_Ieee80211Ofdm_Qam16_2.412GHz_20MHz";

    auto reception = snir->getReception();
    auto transmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
    auto radio = check_and_cast<const Radio *>(transmission->getTransmitter());
    auto centerFrequency = transmission->getCenterFrequency();
    auto bandwidth = transmission->getBandwidth();
    auto bitrate = transmission->getBitrate();
    auto modulation = transmission->getModulation();
    auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation);
    auto subcarrierModulation = ofdmModulation != nullptr ? ofdmModulation->getSubcarrierModulation() : nullptr;
    std::string modelName = StringFormat::formatString(modelNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'c':
                result = std::string(strrchr(radio->getClassName(), ':') + 1);
                break;
            case 'r':
                result = bitrate.str();
                break;
            case 'm':
                result = getModulationName(modulation);
                break;
            case 'M':
                result = getModulationName(subcarrierModulation);
                break;
            case 'f':
                result = centerFrequency.str();
                break;
            case 'b':
                result = bandwidth.str();
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    modelName.erase(std::remove_if(modelName.begin(), modelName.end(), isspace), modelName.end());
    return modelName;
}

} // namespace physicallayer

} // namespace inet

