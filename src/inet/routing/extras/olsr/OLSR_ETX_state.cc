/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   Modified by Weverton Cordeiro                                         *
 *   (C) 2007 wevertoncordeiro@gmail.com                                   *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "inet/common/INETDefs.h"

#include "OLSR_ETX_state.h"
#include "OLSR_ETX.h"

namespace inet {

namespace inetmanet {

OLSR_ETX_state::OLSR_ETX_state()
{
    parameter = &(dynamic_cast<OLSR_ETX*>(getOwner())->parameter_);
}

OLSR_ETX_link_tuple*  OLSR_ETX_state::find_best_sym_link_tuple(const nsaddr_t &main_addr, double now)
{
    OLSR_ETX_link_tuple* best = nullptr;

    for (auto it = ifaceassocset_.begin();
            it != ifaceassocset_.end(); it++)
    {
        OLSR_ETX_iface_assoc_tuple* iface_assoc_tuple = *it;
        if (iface_assoc_tuple->main_addr() == main_addr)
        {
            OLSR_link_tuple *tupleAux = find_sym_link_tuple(iface_assoc_tuple->iface_addr(), now);
            if (tupleAux == nullptr)
                continue;
            OLSR_ETX_link_tuple* tuple =
                dynamic_cast<OLSR_ETX_link_tuple*> (tupleAux);
            if (best == nullptr)
                best = tuple;
            else
            {
                if (parameter->link_delay())
                {
                    if (tuple->nb_link_delay() < best->nb_link_delay())
                        best = tuple;
                }
                else
                {
                    switch (parameter->link_quality())
                    {
                    case OLSR_ETX_BEHAVIOR_ETX:
                        if (tuple->etx() < best->etx())
                            best = tuple;
                        break;

                    case OLSR_ETX_BEHAVIOR_ML:
                        if (tuple->etx() > best->etx())
                            best = tuple;
                        break;
                    case OLSR_ETX_BEHAVIOR_NONE:
                    default:
                        // best = tuple;
                        break;
                    }
                }
            }
        }
    }
    if (best == nullptr)
    {
        OLSR_link_tuple *tuple = find_sym_link_tuple(main_addr, now);
        if (tuple!=nullptr)
            best = check_and_cast<OLSR_ETX_link_tuple*>(tuple);
    }
    return best;
}

} // namespace inetmanet

} // namespace inet

