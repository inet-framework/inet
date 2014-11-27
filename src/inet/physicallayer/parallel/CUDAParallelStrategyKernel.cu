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

#include "inet/physicallayer/parallel/CUDAParallelStrategyKernel.h"

namespace inet {

namespace physicallayer {

using namespace inet;

__global__ void deviceShift(void *buffer, int offset, int size)
{
    for (int i = 0; i < size - offset; i++)
        *((int8_t *)buffer + i) = *((int8_t *)buffer + offset + i);
}

__global__ void deviceComputeAllReceptionsForTransmission(double timeScale, int radioCount, double propagationSpeed, double pathLossAlpha,
                                                          double transmissionPower, double transmissionCarrierFrequency, cuda_simtime_t transmissionTime,
                                                          double transmissionPositionX, double transmissionPositionY, double transmissionPositionZ,
                                                          double *receptionPositionXs, double *receptionPositionYs, double *receptionPositionZs,
                                                          cuda_simtime_t *propagationTimes, cuda_simtime_t *receptionTimes, double *receptionPowers)
{
    int radioIndex = blockIdx.x * blockDim.x + threadIdx.x;
    if (radioIndex < radioCount)
    {
        double dx = transmissionPositionX - receptionPositionXs[radioIndex];
        double dy = transmissionPositionY - receptionPositionYs[radioIndex];
        double dz = transmissionPositionZ - receptionPositionZs[radioIndex];
        double distance = sqrt(dx * dx + dy * dy + dz * dz);

        cuda_simtime_t propagationTime = distance / propagationSpeed * timeScale;
        propagationTimes[radioIndex] = propagationTime;
        cuda_simtime_t receptionTime = transmissionTime + propagationTime;
        receptionTimes[radioIndex] = receptionTime;

        double waveLength = propagationSpeed / transmissionCarrierFrequency;
        // NOTE: this check allows to get the same result from the GPU and the CPU when the pathLossAlpha is exactly 2
        double ratio = waveLength / distance;
        double raisedRatio = pathLossAlpha == 2.0 ? ratio * ratio : pow(ratio, pathLossAlpha);
        double pathLoss = distance == 0.0 ? 1.0 : raisedRatio / (16.0 * M_PI * M_PI);
        double receptionPower = pathLoss * transmissionPower;
        receptionPowers[radioIndex] = receptionPower;
    }
}

__global__ void deviceComputeAllMinSNIRsForAllReceptions2(int transmissionCount, int radioCount, double backgroundNoisePower,
                                                          cuda_simtime_t *transmissionDurations, cuda_simtime_t *receptionTimes, double *receptionPowers,
                                                          double *minSNIRs)
{
    int receptionCount = transmissionCount * radioCount;
    int candidateTransmissionIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int candidateRadioIndex = blockIdx.y * blockDim.y + threadIdx.y;
    int candidateReceptionIndex = candidateRadioIndex + radioCount * candidateTransmissionIndex;
    int newTransmissionIndex = transmissionCount - 1;
    cuda_simtime_t newTransmissionDuration = transmissionDurations[newTransmissionIndex];
    if (candidateTransmissionIndex < transmissionCount && candidateRadioIndex < radioCount)
    {
        cuda_simtime_t candidateTransmissionDuration = transmissionDurations[candidateTransmissionIndex];
        cuda_simtime_t candidateReceptionStartTime = receptionTimes[candidateReceptionIndex];
        cuda_simtime_t candidateReceptionEndTime = candidateReceptionStartTime + candidateTransmissionDuration;

        cuda_simtime_t newReceptionStartTime = receptionTimes[candidateRadioIndex + radioCount * newTransmissionIndex];
        cuda_simtime_t newReceptionEndTime = newReceptionStartTime + newTransmissionDuration;
        cuda_simtime_t interferenceStartTime = candidateReceptionStartTime > newReceptionStartTime ? candidateReceptionStartTime : newReceptionStartTime;
        cuda_simtime_t interferenceEndTime = candidateReceptionEndTime < newReceptionEndTime ? candidateReceptionEndTime : newReceptionEndTime;

        if (interferenceEndTime >= interferenceStartTime) {
            double maximumNoisePower = 0;
            for (int otherReceptionIndex = candidateRadioIndex; otherReceptionIndex < receptionCount; otherReceptionIndex += radioCount)
            {
                int otherTransmissionIndex = otherReceptionIndex / radioCount;
                cuda_simtime_t otherTransmissionDuration = transmissionDurations[otherTransmissionIndex];
                cuda_simtime_t otherReceptionStartTime = receptionTimes[otherReceptionIndex];
                cuda_simtime_t otherReceptionEndTime = otherReceptionStartTime + otherTransmissionDuration;
                bool isOtherStartOverlapping = interferenceStartTime <= otherReceptionStartTime && otherReceptionStartTime <= interferenceEndTime;
                bool isOtherEndOverlapping = interferenceStartTime <= otherReceptionEndTime && otherReceptionEndTime <= interferenceEndTime;
                if (isOtherStartOverlapping || isOtherEndOverlapping)
                {
                    double startNoisePower = 0;
                    double endNoisePower = 0;
                    for (int noiseReceptionIndex = candidateRadioIndex; noiseReceptionIndex < receptionCount; noiseReceptionIndex += radioCount)
                    {
                        if (noiseReceptionIndex != candidateReceptionIndex)
                        {
                            int noiseTransmissionIndex = noiseReceptionIndex / radioCount;
                            cuda_simtime_t noiseTransmissionDuration = transmissionDurations[noiseTransmissionIndex];
                            cuda_simtime_t noiseReceptionStartTime = receptionTimes[noiseReceptionIndex];
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
            minSNIRs[candidateReceptionIndex] = candidateNoisePower / (maximumNoisePower + backgroundNoisePower);
        }
    }
}

__global__ void deviceComputeAllMinSNIRsForAllReceptions(int transmissionCount, int radioCount, double backgroundNoisePower,
                                                         cuda_simtime_t *transmissionDurations, cuda_simtime_t *receptionTimes, double *receptionPowers,
                                                         double *minSNIRs)
{
    int receptionCount = transmissionCount * radioCount;
    int candidateTransmissionIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int candidateRadioIndex = blockIdx.y * blockDim.y + threadIdx.y;
    int candidateReceptionIndex = candidateRadioIndex + radioCount * candidateTransmissionIndex;
    if (candidateTransmissionIndex < transmissionCount && candidateRadioIndex < radioCount)
    {
        cuda_simtime_t candidateTransmissionDuration = transmissionDurations[candidateTransmissionIndex];
        cuda_simtime_t candidateReceptionStartTime = receptionTimes[candidateReceptionIndex];
        cuda_simtime_t candidateReceptionEndTime = candidateReceptionStartTime + candidateTransmissionDuration;
        double maximumNoisePower = 0;
        for (int otherReceptionIndex = candidateRadioIndex; otherReceptionIndex < receptionCount; otherReceptionIndex += radioCount)
        {
            int otherTransmissionIndex = otherReceptionIndex / radioCount;
            cuda_simtime_t otherTransmissionDuration = transmissionDurations[otherTransmissionIndex];
            cuda_simtime_t otherReceptionStartTime = receptionTimes[otherReceptionIndex];
            cuda_simtime_t otherReceptionEndTime = otherReceptionStartTime + otherTransmissionDuration;
            bool isOtherStartOverlapping = candidateReceptionStartTime <= otherReceptionStartTime && otherReceptionStartTime <= candidateReceptionEndTime;
            bool isOtherEndOverlapping = candidateReceptionStartTime <= otherReceptionEndTime && otherReceptionEndTime <= candidateReceptionEndTime;
            if (isOtherStartOverlapping || isOtherEndOverlapping)
            {
                double startNoisePower = 0;
                double endNoisePower = 0;
                for (int noiseReceptionIndex = candidateRadioIndex; noiseReceptionIndex < receptionCount; noiseReceptionIndex += radioCount)
                {
                    if (noiseReceptionIndex != candidateReceptionIndex)
                    {
                        int noiseTransmissionIndex = noiseReceptionIndex / radioCount;
                        cuda_simtime_t noiseTransmissionDuration = transmissionDurations[noiseTransmissionIndex];
                        cuda_simtime_t noiseReceptionStartTime = receptionTimes[noiseReceptionIndex];
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
        minSNIRs[candidateReceptionIndex] = candidateNoisePower / (maximumNoisePower + backgroundNoisePower);
    }
}

void hostShift(void *buffer, int offset, int size)
{
    dim3 blockSize;
    blockSize.x = 1;
    dim3 gridSize;
    gridSize.x = 1;
    deviceShift<<<gridSize, blockSize>>>(buffer, offset, size);
}

void hostComputeAllReceptionsForTransmission(double timeScale, int radioCount, double propagationSpeed, double pathLossAlpha,
                                             double transmissionPower, double transmissionCarrierFrequency, cuda_simtime_t transmissionTime,
                                             double transmissionPositionX, double transmissionPositionY, double transmissionPositionZ,
                                             double *receptionPositionXs, double *receptionPositionYs, double *receptionPositionZs,
                                             cuda_simtime_t *propagationTimes, cuda_simtime_t *receptionTimes, double *receptionPowers)
{
    dim3 blockSize;
    blockSize.x = 4;
    dim3 gridSize;
    gridSize.x = radioCount / blockSize.x + 1;
    deviceComputeAllReceptionsForTransmission<<<gridSize, blockSize>>>(
            timeScale, radioCount, propagationSpeed, pathLossAlpha,
            transmissionPower, transmissionCarrierFrequency, transmissionTime,
            transmissionPositionX, transmissionPositionY, transmissionPositionZ,
            receptionPositionXs, receptionPositionYs, receptionPositionZs,
            propagationTimes, receptionTimes, receptionPowers);
}

void hostComputeAllMinSNIRsForAllReceptions2(int transmissionCount, int radioCount, double backgroundNoisePower,
                                             cuda_simtime_t *transmissionDurations, cuda_simtime_t *receptionTimes, double *receptionPowers,
                                             double *minSNIRs)
{
    dim3 blockSize;
    blockSize.x = 4;
    blockSize.y = 4;
    dim3 gridSize;
    gridSize.x = transmissionCount / blockSize.x + 1;
    gridSize.y = radioCount / blockSize.y + 1;
    deviceComputeAllMinSNIRsForAllReceptions2<<<gridSize, blockSize>>>(
        transmissionCount, radioCount, backgroundNoisePower,
        transmissionDurations, receptionTimes, receptionPowers,
        minSNIRs);
}

void hostComputeAllMinSNIRsForAllReceptions(int transmissionCount, int radioCount, double backgroundNoisePower,
                                            cuda_simtime_t *transmissionDurations, cuda_simtime_t *receptionTimes, double *receptionPowers,
                                            double *minSNIRs)
{
    dim3 blockSize;
    blockSize.x = 4;
    blockSize.y = 4;
    dim3 gridSize;
    gridSize.x = transmissionCount / blockSize.x + 1;
    gridSize.y = radioCount / blockSize.y + 1;
    deviceComputeAllMinSNIRsForAllReceptions<<<gridSize, blockSize>>>(
        transmissionCount, radioCount, backgroundNoisePower,
        transmissionDurations, receptionTimes, receptionPowers,
        minSNIRs);
}

} // namespace physicallayer

} // namespace inet

