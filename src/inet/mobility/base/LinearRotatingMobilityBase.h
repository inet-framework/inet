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

#ifndef __INET_LINEARROTATINGMOBILITYBASE_H
#define __INET_LINEARROTATINGMOBILITYBASE_H

#include "inet/mobility/base/RotatingMobilityBase.h"

namespace inet {

class INET_API LinearRotatingMobilityBase : public RotatingMobilityBase
{
  protected:
    /** @brief End position of current linear movement. */
    EulerAngles targetOrientation;

  protected:
    virtual void initializeOrientation() override;

    virtual EulerAngles slerp(EulerAngles from, EulerAngles to, double delta);

    virtual void rotate() override;

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetOrientation and nextChange has been reached, and its task is
     * to set a new targetOrientation and nextChange. At the end of the movement
     * sequence, it should set nextChange to -1.
     */
    virtual void setTargetOrientation() = 0;

  public:
    LinearRotatingMobilityBase();
};

} // namespace inet

#endif /* LINEARROTATINGMOBILITYBASE_H_ */

