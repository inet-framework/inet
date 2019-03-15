//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef LORAPHY_LORALOGNORMALSHADOWING_H_
#define LORAPHY_LORALOGNORMALSHADOWING_H_

#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the log normal shadowing model.
 */
class INET_API LoRaLogNormalShadowing : public FreeSpacePathLoss
{
  protected:
    m d0;
    double gamma;
    double sigma;

  protected:
    virtual void initialize(int stage) override;

  public:
    LoRaLogNormalShadowing();
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    //virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    m computeRange(W transmissionPower) const;
};

} // namespace physicallayer

} // namespace inet

#endif /* LORAPHY_LORALOGNORMALSHADOWING_H_ */
