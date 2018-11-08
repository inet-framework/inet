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
#include "inet/routing/extras/olsr/Olrs_Etx_state.h"
#include "inet/routing/extras/olsr/Olrs_Etx.h"

namespace inet {

namespace inetmanet {

Olsr_Etx_state::Olsr_Etx_state(Olsr_Etx_parameter *p)
{
    parameter = p;
}

Olsr_Etx_link_tuple*  Olsr_Etx_state::find_best_sym_link_tuple(const nsaddr_t &main_addr, double now)
{
    Olsr_Etx_link_tuple* best = nullptr;

    for (auto it = ifaceassocset_.begin();
            it != ifaceassocset_.end(); it++)
    {
        OLSR_ETX_iface_assoc_tuple* iface_assoc_tuple = *it;
        if (iface_assoc_tuple->main_addr() == main_addr)
        {
            Olsr_link_tuple *tupleAux = find_sym_link_tuple(iface_assoc_tuple->iface_addr(), now);
            if (tupleAux == nullptr)
                continue;
            Olsr_Etx_link_tuple* tuple =
                dynamic_cast<Olsr_Etx_link_tuple*> (tupleAux);
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
        Olsr_link_tuple *tuple = find_sym_link_tuple(main_addr, now);
        if (tuple!=nullptr)
            best = check_and_cast<Olsr_Etx_link_tuple*>(tuple);
    }
    return best;
}

} // namespace inetmanet

} // namespace inet


