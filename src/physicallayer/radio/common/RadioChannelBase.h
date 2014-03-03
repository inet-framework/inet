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

#ifndef __INET_RADIOCHANNELBASE_H_
#define __INET_RADIOCHANNELBASE_H_

#include "IRadioChannel.h"

/**
 * This is an abstract base class for different radio channels.
 *
 * @author Levente Meszaros
 */
class INET_API RadioChannelBase : public cSimpleModule, public OldIRadioChannel
{
  protected:
    int numChannels;

  public:
    RadioChannelBase();
    virtual ~RadioChannelBase() {}

    virtual int getNumChannels() { return numChannels; }

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual void initialize(int stage);
};

#endif
