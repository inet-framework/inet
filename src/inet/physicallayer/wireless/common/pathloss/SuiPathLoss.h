//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/***************************************************************************
* author:      Konrad Polys, Krzysztof Grochla
*
* copyright:   (c) 2013 The Institute of Theoretical and Applied Informatics
*                       of the Polish Academy of Sciences, Project
*                       LIDER/10/194/L-3/11/ supported by NCBIR
*
***************************************************************************/

#ifndef __INET_SUIPATHLOSS_H
#define __INET_SUIPATHLOSS_H

#include <string>

#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the empirical Stanford University Interim path loss model.
 *
 * @author Konrad Polys, Krzysztof Grochla
 */
class INET_API SuiPathLoss : public PathLossBase
{
  protected:
    /** @brief Transmitter antenna high */
    m ht;

    /** @brief Receiver antenna high */
    m hr;

    double a, b, c, d, s;

  protected:
    virtual void initialize(int stage) override;

  public:
    SuiPathLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const override { return m(NaN); }
};

} // namespace physicallayer

} // namespace inet

#endif

