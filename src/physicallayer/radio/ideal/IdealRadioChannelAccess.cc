//
// Copyright (C) 2013 OpenSim Ltd
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

#include "InitStages.h"
#include "IdealRadioChannelAccess.h"
#include "ModuleAccess.h"

IdealRadioChannelAccess::~IdealRadioChannelAccess()
{
    if (idealRadioChannel && radioChannelEntry)
    {
        IdealRadioChannel *radioChannel = dynamic_cast<IdealRadioChannel *>(simulation.getModuleByPath("radioChannel"));
        if (radioChannel)
            radioChannel->unregisterRadio(radioChannelEntry);
        radioChannelEntry = NULL;
    }
}

void IdealRadioChannelAccess::initialize(int stage)
{
    RadioBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        node = getContainingNode(this);
        idealRadioChannel = check_and_cast<IdealRadioChannel *>(simulation.getModuleByPath("radioChannel"));
        radioChannelEntry = idealRadioChannel->registerRadio((cModule *)this);
    }
}

void IdealRadioChannelAccess::sendToChannel(IdealRadioFrame *radioFrame)
{
    EV << "Sending " << radioFrame << " to radio channel.\n";
    idealRadioChannel->sendToChannel(radioChannelEntry, radioFrame);
}
