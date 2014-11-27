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

#ifndef __INET_CUDAPARALLELSTRATEGY_H
#define __INET_CUDAPARALLELSTRATEGY_H

#include <cuda_runtime.h>
#include "inet/physicallayer/parallel/CUDAParallelStrategyKernel.h"
#include "inet/physicallayer/contract/IParallelStrategy.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/contract/ITransmission.h"
#include "inet/physicallayer/contract/IReception.h"
#include "inet/physicallayer/contract/IListening.h"
#include "inet/physicallayer/contract/IReceptionDecision.h"

namespace inet {

namespace physicallayer {

class RadioMedium;

/**
 * Assuming single channel narrowband scalar transmissions, scalar background
 * noise, scalar receivers, free space path loss, and ignoring movement during
 * transmission, propagation and reception.
 */
class INET_API CUDAParallelStrategy : public cModule, public IParallelStrategy
{
    protected:
        RadioMedium *radioMedium;

        int baseRadioId;
        int baseTransmissionId;

        int allocatedRadioCount;
        int allocatedTransmissionCount;
        int allocatedReceptionCount;

        int computedRadioCount;
        int computedTransmissionCount;
        int computedReceptionCount;

        double *hostRadioPositionXs;
        double *hostRadioPositionYs;
        double *hostRadioPositionZs;
        cuda_simtime_t *hostPropagationTimes;
        cuda_simtime_t *hostReceptionTimes;
        double *hostReceptionPowers;

        double *deviceRadioPositionXs;
        double *deviceRadioPositionYs;
        double *deviceRadioPositionZs;
        cuda_simtime_t *devicePropagationTimes;
        cuda_simtime_t *deviceReceptionTimes;
        double *deviceReceptionPowers;

        double pathLossAlpha;
        double backgroundNoisePower;

        double *hostAllMinSNIRs;

        cuda_simtime_t *deviceTransmissionDurations;
        cuda_simtime_t *deviceAllReceptionTimes;
        double *deviceAllReceptionPowers;
        double *deviceAllMinSNIRs;

    protected:
        virtual int numInitStages() const { return INITSTAGE_LAST; }
        virtual void initialize(int stage);
        virtual void reallocateDeviceBuffer(void **buffer, int oldSize, int newSize);
        virtual void reallocateBuffers(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions);

        /**
         * Computes the propagation times, reception times, and reception powers for all receivers for the last transmission.
         */
        virtual void computeAllReceptionsForTransmission(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions);

        /**
         * Computes the minimum SNIRs for all receptions as affected by the last transmission.
         */
        virtual void computeAllMinSNIRsForAllReceptions2(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions);

        // TODO: delete
        virtual void computeAllMinSNIRsForAllReceptions(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions);

    public:
        CUDAParallelStrategy();
        virtual ~CUDAParallelStrategy();

        virtual void printToStream(std::ostream& stream) const;
        virtual void computeCache(const std::vector<const IRadio *> *radios, const std::vector<const ITransmission *> *transmissions);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_CUDAPARALLELSTRATEGY_H

