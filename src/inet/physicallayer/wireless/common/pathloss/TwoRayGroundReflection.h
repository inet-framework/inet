//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#ifndef __INET_TWORAYGROUNDREFLECTION_H
#define __INET_TWORAYGROUNDREFLECTION_H

#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the two ray ground radio path loss model.
 */
class INET_API TwoRayGroundReflection : public FreeSpacePathLoss
{
  protected:
    /**
     * The path loss of one reception: the distance and the altitudes of the
     * two ends are taken when the reception is computed.
     */
    class INET_API ReceptionPathLoss : public IReceptionPathLoss {
      protected:
        const TwoRayGroundReflection *pathLoss;
        mps propagationSpeed;
        m distance;
        m transmitterAltitude;
        m receiverAltitude;

      public:
        ReceptionPathLoss(const TwoRayGroundReflection *pathLoss, mps propagationSpeed, m distance, m transmitterAltitude, m receiverAltitude) :
            pathLoss(pathLoss), propagationSpeed(propagationSpeed), distance(distance), transmitterAltitude(transmitterAltitude), receiverAltitude(receiverAltitude) {}
        virtual double computePathLoss(Hz frequency) const override { return pathLoss->computeTwoRayGroundReflection(propagationSpeed, frequency, distance, transmitterAltitude, receiverAltitude); }
        virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    };

  protected:
    const physicalenvironment::IPhysicalEnvironment *physicalEnvironment = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual double computeTwoRayGroundReflection(mps propagationSpeed, Hz frequency, m distance, m transmitterAltitude, m receiverAltitude) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IReceptionPathLoss> computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

