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

#include "CUDARadioChannel.h"
#include "ScalarImplementation.h"

Define_Module(CUDARadioChannel);

#define cudaErrorCheck(code) { gpuAssert((code), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line)
{
   if (code != cudaSuccess) 
      throw cRuntimeError("CUDA error: %s %s %d\n", cudaGetErrorString(code), file, line);
}

// TODO: for fingerprint equality: (document these somewhere) 
// TODO:  - add -O0 compiler and -prec-sqrt=true compiler flags,
// TODO:  - use int64_t instead of double for simulation times
// TODO: extract and share parts that are common with the CPU based implementation
__global__ void computeReceptions(
        double timeScale, int transmissionCount, int radioCount,
        double propagationSpeed, double alpha,
        cuda_simtime_t *transmissionStartTimes, double *transmissionPositionXs, double *transmissionPositionYs, double *transmissionPositionZs, double *transmissionPowers, double *transmissionCarrierFrequencies,
        double *receptionPositionXs, double *receptionPositionYs, double *receptionPositionZs,
        double *receptionPowers, cuda_simtime_t *receptionStartTimes)
{
    int transmissionIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int radioIndex = blockIdx.y * blockDim.y + threadIdx.y;
    if (transmissionIndex < transmissionCount && radioIndex < radioCount)
    {
        int receptionIndex = radioIndex + radioCount * transmissionIndex;
        double dx = transmissionPositionXs[transmissionIndex] - receptionPositionXs[radioIndex];
        double dy = transmissionPositionYs[transmissionIndex] - receptionPositionYs[radioIndex];
        double dz = transmissionPositionZs[transmissionIndex] - receptionPositionZs[radioIndex];
        double distance = sqrt(dx * dx + dy * dy + dz * dz);

        double waveLength = propagationSpeed / transmissionCarrierFrequencies[transmissionIndex];
        // NOTE: this check allows to get the same result from the GPU and the CPU when the alpha is exactly 2
        double ratio = waveLength / distance;
        double raisedRatio = alpha == 2.0 ? ratio * ratio : pow(ratio, alpha);
        double pathLoss = distance == 0.0 ? 1.0 : ratio / (16.0 * M_PI * M_PI);
        double receptionPower = pathLoss * transmissionPowers[transmissionIndex];
        receptionPowers[receptionIndex] = receptionPower;

        cuda_simtime_t propagationTime = distance / propagationSpeed * timeScale;
        cuda_simtime_t receptionTime = transmissionStartTimes[transmissionIndex] + propagationTime;
        receptionStartTimes[receptionIndex] = receptionTime;
    }
}

__global__ void computeSNRMinimums(
        int transmissionCount, int radioCount,
        cuda_simtime_t *transmissionDurations, double *receptionPowers, cuda_simtime_t *receptionStartTimes, double backgroundNoisePower,
        double *snrMinimums)
{
    int receptionCount = transmissionCount * radioCount;
    int candidateTransmissionIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int candidateRadioIndex = blockIdx.y * blockDim.y + threadIdx.y;
    int candidateReceptionIndex = candidateRadioIndex + radioCount * candidateTransmissionIndex;
    cuda_simtime_t candidateTransmissionDuration = transmissionDurations[candidateTransmissionIndex];
    cuda_simtime_t candidateReceptionStartTime = receptionStartTimes[candidateReceptionIndex];
    cuda_simtime_t candidateReceptionEndTime = candidateReceptionStartTime + candidateTransmissionDuration;
    double maximumNoisePower = 0;
    for (int otherReceptionIndex = candidateRadioIndex; otherReceptionIndex < receptionCount; otherReceptionIndex += radioCount)
    {
        int otherTransmissionIndex = otherReceptionIndex / radioCount;
        cuda_simtime_t otherTransmissionDuration = transmissionDurations[otherTransmissionIndex];
        cuda_simtime_t otherReceptionStartTime = receptionStartTimes[otherReceptionIndex];
        cuda_simtime_t otherReceptionEndTime = otherReceptionStartTime + otherTransmissionDuration;
        bool isOtherStartOverlapping = candidateReceptionStartTime <= otherReceptionStartTime && otherReceptionStartTime <= candidateReceptionEndTime;
        bool isOtherEndOverlapping = candidateReceptionStartTime <= otherReceptionEndTime && otherReceptionEndTime <= candidateReceptionEndTime;
        if (isOtherStartOverlapping || isOtherEndOverlapping)
        {
            double startNoisePower = backgroundNoisePower;
            double endNoisePower = backgroundNoisePower;
            for (int noiseReceptionIndex = candidateRadioIndex; noiseReceptionIndex < receptionCount; noiseReceptionIndex += radioCount)
            {
                if (noiseReceptionIndex != candidateReceptionIndex)
                {
                    int noiseTransmissionIndex = noiseReceptionIndex / radioCount;
                    cuda_simtime_t noiseTransmissionDuration = transmissionDurations[noiseTransmissionIndex];
                    cuda_simtime_t noiseReceptionStartTime = receptionStartTimes[noiseReceptionIndex];
                    cuda_simtime_t noiseReceptionEndTime = noiseReceptionStartTime + noiseTransmissionDuration;
                    double noisePower = receptionPowers[noiseReceptionIndex];
                    if (isOtherStartOverlapping && noiseReceptionStartTime <= otherReceptionStartTime && otherReceptionStartTime <= noiseReceptionEndTime)
                        startNoisePower += noisePower;
                    if (isOtherEndOverlapping && noiseReceptionStartTime <= otherReceptionEndTime && otherReceptionEndTime <= noiseReceptionEndTime)
                        endNoisePower += noisePower;
                }
            }
            if (isOtherStartOverlapping && startNoisePower > maximumNoisePower)
                maximumNoisePower = startNoisePower;
            if (isOtherEndOverlapping && endNoisePower > maximumNoisePower)
                maximumNoisePower = endNoisePower;
        }
    }
    double candidateNoisePower = receptionPowers[candidateReceptionIndex];
    snrMinimums[candidateReceptionIndex] = candidateNoisePower / maximumNoisePower;
}

void CUDARadioChannel::computeCache(const std::vector<const IRadio *> *radios, const std::vector<const IRadioSignalTransmission *> *transmissions)
{
    // for all transmissions compute the start and end reception times at the start and end reception positions
    int transmissionCount = transmissions->size();
    int radioCount = radios->size();
    int receptionCount = transmissionCount * radioCount;
    int transmissionSize = transmissionCount * sizeof(double);
    int radioSize = radioCount * sizeof(double);
    int receptionSize = receptionCount * sizeof(double);
    double timeScale = (double) SimTime::getScale();
    double alpha = check_and_cast<const ScalarRadioSignalFreeSpaceAttenuation *>(attenuation)->getAlpha();
    double backgroundNoisePower = check_and_cast<const ScalarRadioBackgroundNoise *>(backgroundNoise)->getPower().get();
    EV_DEBUG << "Radio channel is computing cache with transmission count: " << transmissionCount << ", reception count: " << radioCount << ", arrival count: " << receptionCount << endl;

    // allocate host memory
    EV_DEBUG << "Allocating host memory" << endl;
    cuda_simtime_t *hostTransmissionStartTimes = new cuda_simtime_t[transmissionCount];
    cuda_simtime_t *hostTransmissionDurations = new cuda_simtime_t[transmissionCount];
    double *hostTransmissionPositionXs = new double[transmissionCount];
    double *hostTransmissionPositionYs = new double[transmissionCount];
    double *hostTransmissionPositionZs = new double[transmissionCount];
    double *hostTransmissionPowers = new double[transmissionCount];
    double *hostTransmissionCarrierFrequencies = new double[transmissionCount];
    double *hostRadioPositionXs = new double[radioCount];
    double *hostRadioPositionYs = new double[radioCount];
    double *hostRadioPositionZs = new double[radioCount];
    double *hostReceptionPowers = new double[receptionCount];
    cuda_simtime_t *hostReceptionStartTimes = new cuda_simtime_t[receptionCount];
    double *hostSNRMinimums = new double[receptionCount];

    // prepare host data
    EV_DEBUG << "Preparing host data" << endl;
    int index = 0;
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const IRadioSignalTransmission *transmission = *it;
        Coord startPosition = transmission->getStartPosition();
        hostTransmissionStartTimes[index] = transmission->getStartTime().raw();
        hostTransmissionDurations[index] = (transmission->getStartTime() - transmission->getEndTime()).raw();
        hostTransmissionPositionXs[index] = startPosition.x;
        hostTransmissionPositionYs[index] = startPosition.y;
        hostTransmissionPositionZs[index] = startPosition.z;
        const ScalarRadioSignalTransmission *scalarTransmission = check_and_cast<const ScalarRadioSignalTransmission *>(transmission);
        hostTransmissionPowers[index] = scalarTransmission->getPower().get();
        hostTransmissionCarrierFrequencies[index] = scalarTransmission->getCarrierFrequency().get();
        index++;
    }
    index = 0;
    for (std::vector<const IRadio *>::const_iterator it = radios->begin(); it != radios->end(); it++)
    {
        const IRadio *radio = *it;
        Coord startPosition = radio->getAntenna()->getMobility()->getCurrentPosition();
        hostRadioPositionXs[index] = startPosition.x;
        hostRadioPositionYs[index] = startPosition.y;
        hostRadioPositionZs[index] = startPosition.z;
        index++;
    }

    // allocate device memory
    EV_DEBUG << "Allocating device memory" << endl;
    cuda_simtime_t *deviceTransmissionStartTimes;
    cuda_simtime_t *deviceTransmissionDurations;
    double *deviceTransmissionPositionXs;
    double *deviceTransmissionPositionYs;
    double *deviceTransmissionPositionZs;
    double *deviceTransmissionPowers;
    double *deviceTransmissionCarrierFrequencies;
    double *deviceRadioPositionXs;
    double *deviceRadioPositionYs;
    double *deviceRadioPositionZs;
    double *deviceReceptionPowers;
    cuda_simtime_t *deviceReceptionStartTimes;
    double *deviceSNRMinimums;
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionStartTimes, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionDurations, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionPositionXs, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionPositionYs, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionPositionZs, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionPowers, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceTransmissionCarrierFrequencies, transmissionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceRadioPositionXs, radioSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceRadioPositionYs, radioSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceRadioPositionZs, radioSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceReceptionPowers, receptionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceReceptionStartTimes, receptionSize)); 
    cudaErrorCheck(cudaMalloc((void**)&deviceSNRMinimums, receptionSize)); 

    // copy data from host to device
    EV_DEBUG << "Copying host data to device memory" << endl;
    cudaErrorCheck(cudaMemcpy(deviceTransmissionStartTimes, hostTransmissionStartTimes, transmissionSize, cudaMemcpyHostToDevice)); 
    cudaErrorCheck(cudaMemcpy(deviceTransmissionDurations, hostTransmissionDurations, transmissionSize, cudaMemcpyHostToDevice)); 
    cudaErrorCheck(cudaMemcpy(deviceTransmissionPositionXs, hostTransmissionPositionXs, transmissionSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceTransmissionPositionYs, hostTransmissionPositionYs, transmissionSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceTransmissionPositionZs, hostTransmissionPositionZs, transmissionSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceTransmissionPowers, hostTransmissionPowers, transmissionSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceTransmissionCarrierFrequencies, hostTransmissionCarrierFrequencies, transmissionSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceRadioPositionXs, hostRadioPositionXs, radioSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceRadioPositionYs, hostRadioPositionYs, radioSize, cudaMemcpyHostToDevice));
    cudaErrorCheck(cudaMemcpy(deviceRadioPositionZs, hostRadioPositionZs, radioSize, cudaMemcpyHostToDevice));

    // start the computation on the device
    EV_DEBUG << "Starting computation on device" << endl;
    dim3 blockSize;
    blockSize.x = 4;
    blockSize.y = 4;
    dim3 gridSize;
    gridSize.x = transmissionCount / blockSize.x + 1;
    gridSize.y = radioCount / blockSize.y + 1;

    computeReceptions<<<gridSize, blockSize>>>(
            timeScale, transmissionCount, radioCount,
            propagation->getPropagationSpeed().get(), alpha, 
            deviceTransmissionStartTimes, deviceTransmissionPositionXs, deviceTransmissionPositionYs, deviceTransmissionPositionZs, deviceTransmissionPowers, deviceTransmissionCarrierFrequencies,
            deviceRadioPositionXs, deviceRadioPositionYs, deviceRadioPositionZs,
            deviceReceptionPowers, deviceReceptionStartTimes);
    
    cudaErrorCheck(cudaThreadSynchronize());

    computeSNRMinimums<<<gridSize, blockSize>>>(
            transmissionCount, radioCount,
            deviceTransmissionDurations, deviceReceptionPowers, deviceReceptionStartTimes, backgroundNoisePower,
            deviceSNRMinimums);

    // copy data from device to host
    EV_DEBUG << "Copying device data to host memory" << endl;
    cudaErrorCheck(cudaMemcpy(hostReceptionPowers, deviceReceptionPowers, receptionSize, cudaMemcpyDeviceToHost)); 
    cudaErrorCheck(cudaMemcpy(hostReceptionStartTimes, deviceReceptionStartTimes, receptionSize, cudaMemcpyDeviceToHost)); 
    cudaErrorCheck(cudaMemcpy(hostSNRMinimums, deviceSNRMinimums, receptionSize, cudaMemcpyDeviceToHost)); 

    EV_DEBUG << "Reception times: ";
    for (int receptionIndex = 0; receptionIndex < receptionCount; receptionIndex++) {
        simtime_t receptionStartTime;
        receptionStartTime.setRaw(hostReceptionStartTimes[receptionIndex]);
        EV_DEBUG << receptionStartTime << " ";
    }
    EV_DEBUG << endl;

    EV_DEBUG << "Reception powers: ";
    for (int receptionIndex = 0; receptionIndex < receptionCount; receptionIndex++)
        EV_DEBUG << hostReceptionPowers[receptionIndex] << " ";
    EV_DEBUG << endl;

    EV_DEBUG << "SNR minimums: ";
    for (int receptionIndex = 0; receptionIndex < receptionCount; receptionIndex++) {
        int radioIndex = receptionIndex % radioCount;
        int transmissionIndex = receptionIndex / radioCount;
        const IRadio *radio = radios->at(radioIndex);
        const IRadioSignalTransmission *transmission = transmissions->at(transmissionIndex);
        simtime_t receptionStartTime;
        simtime_t receptionEndTime;
        receptionStartTime.setRaw(hostReceptionStartTimes[receptionIndex]);
        receptionEndTime.setRaw(receptionStartTime.raw() + (transmission->getStartTime() - transmission->getEndTime()).raw());
        W receptionPower = W(hostReceptionPowers[receptionIndex]);
        const ScalarRadioSignalTransmission *scalarTransmission = check_and_cast<const ScalarRadioSignalTransmission *>(transmission);
        // TODO: add reception coordinates
        ScalarRadioSignalReception *reception = new ScalarRadioSignalReception(radio, transmission, receptionStartTime, receptionEndTime, Coord(), Coord(), receptionPower, scalarTransmission->getCarrierFrequency(), scalarTransmission->getBandwidth());
        double snrMinimum = hostSNRMinimums[receptionIndex];
        double snrThreshold = 0; // TODO: check_and_cast<const ScalarSNRRadioDecider *>(radio->getDecider())->getSNRThreshold();
        RadioReceptionIndication *indication = new RadioReceptionIndication();
        indication->setMinSNIR(snrMinimum);
        RadioSignalReceptionDecision *decision = new RadioSignalReceptionDecision(reception, indication, true, true, snrMinimum > snrThreshold);
        setCachedDecision(radio, transmission, decision);
        EV_DEBUG << snrMinimum << " ";
    }
    EV_DEBUG << endl;

    // release resources
    EV_DEBUG << "Freeing device memory" << endl;
    cudaErrorCheck(cudaFree(deviceTransmissionStartTimes));
    cudaErrorCheck(cudaFree(deviceTransmissionDurations));
    cudaErrorCheck(cudaFree(deviceTransmissionPositionXs));
    cudaErrorCheck(cudaFree(deviceTransmissionPositionYs));
    cudaErrorCheck(cudaFree(deviceTransmissionPositionZs));
    cudaErrorCheck(cudaFree(deviceTransmissionPowers));
    cudaErrorCheck(cudaFree(deviceTransmissionCarrierFrequencies));
    cudaErrorCheck(cudaFree(deviceRadioPositionXs));
    cudaErrorCheck(cudaFree(deviceRadioPositionYs));
    cudaErrorCheck(cudaFree(deviceRadioPositionZs));
    cudaErrorCheck(cudaFree(deviceReceptionPowers));
    cudaErrorCheck(cudaFree(deviceReceptionStartTimes));
    cudaErrorCheck(cudaFree(deviceSNRMinimums));

    EV_DEBUG << "Freeing host memory" << endl;
    delete hostTransmissionStartTimes;
    delete hostTransmissionDurations;
    delete hostTransmissionPositionXs;
    delete hostTransmissionPositionYs;
    delete hostTransmissionPositionZs;
    delete hostTransmissionPowers;
    delete hostTransmissionCarrierFrequencies;
    delete hostRadioPositionXs;
    delete hostRadioPositionYs;
    delete hostRadioPositionZs;
    delete hostReceptionPowers;
    delete hostReceptionStartTimes;
    delete hostSNRMinimums;
}

void CUDARadioChannel::transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission)
{
    // TODO: start computation here in a background thread
    RadioChannel::transmitToChannel(radio, transmission);
}

const IRadioSignalReceptionDecision *CUDARadioChannel::receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    // KLUDGE: for testing
    if (cache.size() == 0)
        const_cast<CUDARadioChannel *>(this)->computeCache((const std::vector<const IRadio *> *)(&radios), (const std::vector<const IRadioSignalTransmission *> *)(&transmissions));
    return RadioChannel::receiveFromChannel(radio, listening, transmission);
}
