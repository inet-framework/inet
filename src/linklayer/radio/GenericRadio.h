//
// Copyright (C) 2006 Andras Varga
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

