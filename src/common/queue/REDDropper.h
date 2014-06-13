//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef __INET_REDDROPPER_H_
#define __INET_REDDROPPER_H_

#include "INETDefs.h"
#include "AlgorithmicDropperBase.h"

/**
 * Implementation of Random Early Detection (RED).
 */
class REDDropper : public AlgorithmicDropperBase
{
  protected:
    double wq;
    double *minths;
    double *maxths;
    double *maxps;

    double avg;

  public:
    REDDropper() : wq(0), minths(NULL), maxths(NULL), maxps(NULL), avg(0.0) {}
  protected:
    virtual ~REDDropper();
    virtual void initialize();
    virtual bool shouldDrop(cPacket *packet);
};

#endif
