//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/parallel/CUDAParallelStrategy.h"
#include "inet/physicallayer/analogmodel/ScalarAnalogModel.h"
#include "inet/physicallayer/analogmodel/ScalarTransmission.h"
#include "inet/physicallayer/analogmodel/ScalarReception.h"
#include "inet/physicallayer/analogmodel/ScalarSNIR.h"
#include "inet/physicallayer/backgroundnoise/IsotropicScalarBackgroundNoise.h"
#include "inet/physicallayer/common/Arrival.h"
#include "inet/physicallayer/common/Interference.h"
#include "inet/physicallayer/common/ReceptionDecision.h"
#include "inet/physicallayer/common/RadioMedium.h"
#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

using namespace inet;

Define_Module(CUDAParallelStrategy);

CUDAParallelStrategy::CUDAParallelStrategy() :
    radioMedium(NULL),
    hostRadioPositionXs(NULL),
    hostRadioPositionYs(NULL),
    hostRadioPositionZs(NULL),
    hostPropagationTimes(NULL),
    hostReceptionTimes(NULL),
    hostReceptionPowers(NULL),
    deviceRadioPositionXs(NULL),
    deviceRadioPositionYs(NULL),
    deviceRadioPositionZs(NULL),
    devicePropagationTimes(NULL),
    deviceReceptionTimes(NULL),
    deviceReceptionPowers(NULL),
    pathLossAlpha(NaN),
    backgroundNoisePower(NaN)
{
}

CUDAParallelStrategy::~CUDAParallelStrategy()
{
    delete hostRadioPositionXs;
    delete hostRadioPositionYs;
    delete hostRadioPositionZs;
    delete hostPropagationTimes;
    delete hostReceptionTimes;
    delete hostReceptionPowers;
    CUDA_ERROR_CHECK(cudaFree(deviceRadioPositionXs));
    CUDA_ERROR_CHECK(cudaFree(deviceRadioPositionYs));
    CUDA_ERROR_CHECK(cudaFree(deviceRadioPositionZs));
    CUDA_ERROR_CHECK(cudaFree(devicePropagationTimes));
    CUDA_ERROR_CHECK(cudaFree(deviceReceptionTimes));
    CUDA_ERROR_CHECK(cudaFree(deviceReceptionPowers));
}

void CUDAParallelStrategy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = check_and_cast<RadioMedium *>(getModuleByPath(par("radioMediumModule")));
    }
    else if (stage == INITSTAGE_LAST - 1) {
        int radioCount = radioMedium->radios.size();
        int radioSize = radioCount * sizeof(double);
        int receptionDoublesSize = radioCount * sizeof(double);
        int receptionTimesSize = radioCount * sizeof(cuda_simtime_t);
        hostRadioPositionXs = new double[radioCount];
        hostRadioPositionYs = new double[radioCount];
        hostRadioPositionZs = new double[radioCount];
        hostPropagationTimes = new cuda_simtime_t[radioCount];
        hostReceptionTimes = new cuda_simtime_t[radioCount];
        hostReceptionPowers = new double[radioCount];
        CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceRadioPositionXs, radioSize));
        CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceRadioPositionYs, radioSize));
        CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceRadioPositionZs, radioSize));
        CUDA_ERROR_CHECK(cudaMalloc((void**)&devicePropagationTimes, receptionTimesSize));
        CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceReceptionTimes, receptionTimesSize));
        CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceReceptionPowers, receptionDoublesSize));
        check_and_cast<const ScalarAnalogModel *>(radioMedium->getAnalogModel());
        pathLossAlpha = check_and_cast<const FreeSpacePathLoss *>(radioMedium->getPathLoss())->getAlpha();
        backgroundNoisePower = check_and_cast<const IsotropicScalarBackgroundNoise *>(radioMedium->getBackgroundNoise())->getPower().get();
    }
}

void CUDAParallelStrategy::printToStream(std::ostream& stream) const
{
    stream << "CUDAParalellStrategy";
}

void CUDAParallelStrategy::computeAllReceptionsForTransmission(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions) const
{
    // compute the propagation times, reception times, and reception powers for all receivers
    int radioCount = radios->size();
    int radioSize = radioCount * sizeof(double);
    int receptionDoublesSize = radioCount * sizeof(double);
    int receptionTimesSize = radioCount * sizeof(cuda_simtime_t);
    double timeScale = (double)SimTime::getScale();
    double propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed().get();
    const ITransmission *transmission = transmissions->at(transmissions->size() - 1);
    const ScalarTransmission *scalarTransmission = static_cast<const ScalarTransmission *>(transmission);
    const Coord transmissionPosition = scalarTransmission->getStartPosition();
    simtime_t transmissionDuration = transmission->getEndTime() - transmission->getStartTime();
    Hz carrierFrequency = scalarTransmission->getCarrierFrequency();
    Hz bandwidth = scalarTransmission->getBandwidth();
    EV_DEBUG << "Computing all receptions for last transmission, radioCount = " << radioCount << ", transmissionCount = " << transmissions->size() << endl;

    // prepare host data
    EV_TRACE << "Preparing host data" << endl;
    int index = 0;
    for (std::vector<const IRadio *>::const_iterator it = radios->begin(); it != radios->end(); it++)
    {
        const IRadio *radio = *it;
        Coord position = radio->getAntenna()->getMobility()->getCurrentPosition();
        hostRadioPositionXs[index] = position.x;
        hostRadioPositionYs[index] = position.y;
        hostRadioPositionZs[index] = position.z;
        index++;
    }

    // copy data from host to device
    EV_TRACE << "Copying host data to device memory" << endl;
    CUDA_ERROR_CHECK(cudaMemcpy(deviceRadioPositionXs, hostRadioPositionXs, radioSize, cudaMemcpyHostToDevice));
    CUDA_ERROR_CHECK(cudaMemcpy(deviceRadioPositionYs, hostRadioPositionYs, radioSize, cudaMemcpyHostToDevice));
    CUDA_ERROR_CHECK(cudaMemcpy(deviceRadioPositionZs, hostRadioPositionZs, radioSize, cudaMemcpyHostToDevice));

    // start the computation on the device
    EV_TRACE << "Starting computation on device" << endl;

    hostComputeAllReceptionsForTransmission(
            timeScale, radioCount, propagationSpeed, pathLossAlpha,
            scalarTransmission->getPower().get(), carrierFrequency.get(), scalarTransmission->getStartTime().raw(),
            transmissionPosition.x, transmissionPosition.y, transmissionPosition.z,
            deviceRadioPositionXs, deviceRadioPositionYs, deviceRadioPositionZs,
            devicePropagationTimes, deviceReceptionTimes, deviceReceptionPowers);

//    CUDA_ERROR_CHECK(cudaThreadSynchronize());

    // copy data from device to host
    EV_TRACE << "Copying device data to host memory" << endl;
    CUDA_ERROR_CHECK(cudaMemcpy(hostPropagationTimes, devicePropagationTimes, receptionTimesSize, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(hostReceptionTimes, deviceReceptionTimes, receptionTimesSize, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(hostReceptionPowers, deviceReceptionPowers, receptionDoublesSize, cudaMemcpyDeviceToHost));

    simtime_t maxArrivalEndTime = simTime();
    for (int receiverIndex = 0; receiverIndex < radioCount; receiverIndex++) {
        const IRadio *receiver = radios->at(receiverIndex);
        if (receiver != transmission->getTransmitter()) {
            IMobility *mobility = receiver->getAntenna()->getMobility();
            const Coord startArrivalPosition = mobility->getCurrentPosition();
            const Coord endArrivalPosition = startArrivalPosition;
            simtime_t startArrivalTime;
            startArrivalTime.setRaw(hostReceptionTimes[receiverIndex]);
            simtime_t endArrivalTime;
            endArrivalTime.setRaw(hostReceptionTimes[receiverIndex] + transmissionDuration.raw());
            simtime_t startPropagationTime;
            startPropagationTime.setRaw(hostPropagationTimes[receiverIndex]);
            const simtime_t endPropagationTime = startPropagationTime;
            const EulerAngles startArrivalOrientation = mobility->getCurrentAngularPosition();
            const EulerAngles endArrivalOrientation = startArrivalOrientation;
            const Arrival *arrival = new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
            const IListening *listening = receiver->getReceiver()->createListening(receiver, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition);
            W receptionPower = W(hostReceptionPowers[receiverIndex]);
            const ScalarReception *reception = new ScalarReception(receiver, transmission, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation, carrierFrequency, bandwidth, receptionPower);
            if (endArrivalTime > maxArrivalEndTime)
                maxArrivalEndTime = endArrivalTime;
            radioMedium->setCachedArrival(receiver, transmission, arrival);
            radioMedium->setCachedListening(receiver, transmission, listening);
            radioMedium->setCachedReception(receiver, transmission, reception);
            EV_TRACE << "Computation result, transmission id = " << transmission->getId() << ", receiver id = " << receiver->getId() << ", arrivalTime = " << startArrivalTime << ", receptionPower = " << receptionPower << endl;
        }
    }
    radioMedium->getTransmissionCacheEntry(transmission)->interferenceEndTime = maxArrivalEndTime + radioMedium->maxTransmissionDuration;
}

// TODO: we could do better than this by recomputing only those SNIRs which are actually invalid
void CUDAParallelStrategy::computeAllMinSNIRsForAllReceptions(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions) const
{
    // compute the minimum SNIR for all receptions
    int transmissionCount = transmissions->size();
    int radioCount = radios->size();
    int receptionCount = transmissionCount * radioCount;
    int transmissionTimesSize = transmissionCount * sizeof(cuda_simtime_t);
    int receptionDoublesSize = receptionCount * sizeof(double);
    int receptionTimesSize = receptionCount * sizeof(cuda_simtime_t);
    EV_DEBUG << "Computing all min SNIRs for all transmissions, transmission count = " << transmissionCount << ", radio count = " << radioCount << ", reception count = " << receptionCount << endl;

    // allocate host memory
    EV_TRACE << "Allocating host memory" << endl;
    cuda_simtime_t *hostTransmissionDurations = new cuda_simtime_t[transmissionCount];
    cuda_simtime_t *hostReceptionTimes = new cuda_simtime_t[receptionCount];
    double *hostReceptionPowers = new double[receptionCount];
    double *hostMinSNIRs = new double[receptionCount];

    // prepare host data
    EV_TRACE << "Preparing host data" << endl;
    int index = 0;
    for (std::vector<const ITransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const ITransmission *transmission = *it;
        hostTransmissionDurations[index] = (transmission->getEndTime() - transmission->getStartTime()).raw();
        index++;
    }
    for (int receptionIndex = 0; receptionIndex < receptionCount; receptionIndex++) {
        int radioIndex = receptionIndex % radioCount;
        int transmissionIndex = receptionIndex / radioCount;
        const ITransmission *transmission = transmissions->at(transmissionIndex);
        const IRadio *receiver = radios->at(radioIndex);
        if (receiver == transmission->getTransmitter()) {
            hostReceptionTimes[receptionIndex] = -1;
            hostReceptionPowers[receptionIndex] = NaN;
        }
        else {
            const RadioMedium::TransmissionCacheEntry& transmissionCacheEntry = radioMedium->transmissionCache[transmissionIndex];
            if (transmissionCacheEntry.receptionCacheEntries) {
                const RadioMedium::ReceptionCacheEntry& receptionCacheEntry = transmissionCacheEntry.receptionCacheEntries->at(radioIndex);
                const ScalarReception *scalarReception = static_cast<const ScalarReception *>(receptionCacheEntry.reception);
                hostReceptionTimes[receptionIndex] = scalarReception->getStartTime().raw();
                hostReceptionPowers[receptionIndex] = scalarReception->getPower().get();
            }
        }
        EV_TRACE << "Prepared host data, transmission id = " << transmissionIndex << ", radio id = " << radioIndex << ", reception time = " << hostReceptionTimes[receptionIndex] << ", transmission duration =  " << hostTransmissionDurations[transmissionIndex] << ", reception power = " << hostReceptionPowers[receptionIndex] << endl;
    }

    // allocate device memory
    EV_TRACE << "Allocating device memory" << endl;
    cuda_simtime_t *deviceTransmissionDurations;
    cuda_simtime_t *deviceReceptionTimes;
    double *deviceReceptionPowers;
    double *deviceMinSNIRs;
    CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceTransmissionDurations, transmissionTimesSize));
    CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceReceptionTimes, receptionTimesSize));
    CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceReceptionPowers, receptionDoublesSize));
    CUDA_ERROR_CHECK(cudaMalloc((void**)&deviceMinSNIRs, receptionDoublesSize));

    // copy data from host to device
    EV_TRACE << "Copying host data to device memory" << endl;
    CUDA_ERROR_CHECK(cudaMemcpy(deviceTransmissionDurations, hostTransmissionDurations, transmissionTimesSize, cudaMemcpyHostToDevice));
    CUDA_ERROR_CHECK(cudaMemcpy(deviceReceptionTimes, hostReceptionTimes, receptionTimesSize, cudaMemcpyHostToDevice));
    CUDA_ERROR_CHECK(cudaMemcpy(deviceReceptionPowers, hostReceptionPowers, receptionDoublesSize, cudaMemcpyHostToDevice));

    // start the computation on the device
    EV_TRACE << "Starting computation on device" << endl;

    hostComputeMinSNIRsForAllReceptions(
            transmissionCount, radioCount, backgroundNoisePower,
            deviceTransmissionDurations, deviceReceptionTimes, deviceReceptionPowers,
            deviceMinSNIRs);

//    CUDA_ERROR_CHECK(cudaThreadSynchronize());

    // copy data from device to host
    EV_TRACE << "Copying device data to host memory" << endl;
    CUDA_ERROR_CHECK(cudaMemcpy(hostMinSNIRs, deviceMinSNIRs, receptionDoublesSize, cudaMemcpyDeviceToHost));

    for (int receptionIndex = 0; receptionIndex < receptionCount; receptionIndex++) {
        int radioIndex = receptionIndex % radioCount;
        int transmissionIndex = receptionIndex / radioCount;
        const IRadio *receiver = radios->at(radioIndex);
        const ITransmission *transmission = transmissions->at(transmissionIndex);
        if (receiver != transmission->getTransmitter()) {
            const ScalarReception *reception = static_cast<const ScalarReception *>(radioMedium->getReception(receiver, transmission));
            // TODO: fake interference to avoid its computation?
//            const IInterference *interference = radioMedium->getCachedInterference(receiver, transmission);
//            if (!interference)
//                radioMedium->setCachedInterference(receiver, transmission, new Interference(NULL, NULL));
            const ScalarSNIR *snir = dynamic_cast<const ScalarSNIR *>(radioMedium->getCachedSNIR(receiver, transmission));
            if (!snir) {
                snir = new ScalarSNIR(reception, NULL);
                radioMedium->setCachedSNIR(receiver, transmission, snir);
            }
            snir->minSNIR = hostMinSNIRs[receptionIndex];
            EV_TRACE << "Computation result, transmission id = " << transmission->getId() << ", receiver id = " << receiver->getId() << ", minSNIR = " << snir->minSNIR << endl;
        }
    }

    // release resources
    EV_TRACE << "Freeing device memory" << endl;
    CUDA_ERROR_CHECK(cudaFree(deviceTransmissionDurations));
    CUDA_ERROR_CHECK(cudaFree(deviceReceptionTimes));
    CUDA_ERROR_CHECK(cudaFree(deviceReceptionPowers));
    CUDA_ERROR_CHECK(cudaFree(deviceMinSNIRs));

    EV_TRACE << "Freeing host memory" << endl;
    delete hostTransmissionDurations;
    delete hostReceptionTimes;
    delete hostReceptionPowers;
    delete hostMinSNIRs;
}

void CUDAParallelStrategy::computeCache(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions) const
{
    computeAllReceptionsForTransmission(radios, transmissions);
    computeAllMinSNIRsForAllReceptions(radios, transmissions);
}

} // namespace physicallayer

} // namespace inet

