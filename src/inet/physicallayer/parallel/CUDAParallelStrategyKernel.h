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

#ifndef __INET_CUDAPARALLELSTRATEGYKERNEL_H
#define __INET_CUDAPARALLELSTRATEGYKERNEL_H

namespace inet {

namespace physicallayer {

#define CUDA_ERROR_CHECK(block) { cudaError_t code = block; if (code != cudaSuccess) throw cRuntimeError("CUDA error: %s %s %d\n", cudaGetErrorString(code), __FILE__, __LINE__); }

typedef int64_t cuda_simtime_t;

void hostComputeAllReceptionsForTransmission(double timeScale, int radioCount, double propagationSpeed, double pathLossAlpha,
                                             double transmissionPower, double transmissionCarrierFrequency, cuda_simtime_t transmissionTime,
                                             double transmissionPositionX, double transmissionPositionY, double transmissionPositionZ,
                                             double *receptionPositionXs, double *receptionPositionYs, double *receptionPositionZs,
                                             cuda_simtime_t *propagationTimes, cuda_simtime_t *receptionTimes, double *receptionPowers);

void hostComputeMinSNIRsForAllReceptions(int transmissionCount, int radioCount, double backgroundNoisePower,
                                         cuda_simtime_t *transmissionDurations, cuda_simtime_t *receptionTimes, double *receptionPowers,
                                         double *minSNIRs);

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_CUDAPARALLELSTRATEGYKERNEL_H

