//
// Copyright (C) 2006 Andras Varga
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef GENERICRADIO_H
#define GENERICRADIO_H

#include "AbstractRadio.h"

/**
 * Generic radio module, employing PathLossReceptionModel and GenericRadioModel.
 */
class INET_API GenericRadio : public AbstractRadio
{
  protected:
    virtual IReceptionModel *createReceptionModel() {return (IReceptionModel *)createOne("PathLossReceptionModel");}
    virtual IRadioModel *createRadioModel() {return (IRadioModel *)createOne("GenericRadioModel");}
};

#endif

