//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#ifndef IEEE80211RADIO_H
#define IEEE80211RADIO_H

#include "AbstractRadio.h"

/**
 * Radio for the IEEE 802.11 model. Just a AbstractRadio with PathLossReceptionModel
 * and Ieee80211RadioModel.
 */
class INET_API Ieee80211Radio : public AbstractRadio
{
  protected:
    virtual IReceptionModel *createReceptionModel() {return (IReceptionModel *)createOne("PathLossReceptionModel");}
    virtual IRadioModel *createRadioModel() {return (IRadioModel *)createOne("Ieee80211RadioModel");}
};

#endif

