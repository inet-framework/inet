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

#include <cpar.h>
#include <InitStages.h>
#include <RadioChannelBase.h>
#include <simkerneldefs.h>
#include <simutil.h>

RadioChannelBase::RadioChannelBase()
{
    numChannels = -1;
}

void RadioChannelBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        numChannels = par("numChannels");
    }
}

//int RadioChannelBase::addRadio(IRadio *radio)
//{
//    ASSERT(false);
//    return -1;
//}

//void RadioChannelBase::removeRadio(int id)
//{
//    ASSERT(false);
//}
