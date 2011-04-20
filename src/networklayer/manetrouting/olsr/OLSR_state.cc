/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
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

///
/// \file   OLSR_state.cc
/// \brief  Implementation of all functions needed for manipulating the internal
///     state of an OLSR node.
///

#include "OLSR_state.h"
#include "OLSR.h"

/********** MPR Selector Set Manipulation **********/

OLSR_mprsel_tuple*
OLSR_state::find_mprsel_tuple(const nsaddr_t &main_addr)
{
    for (mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
    {
        OLSR_mprsel_tuple* tuple = *it;
        if (tuple->main_addr() == main_addr)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_mprsel_tuple(OLSR_mprsel_tuple* tuple)
{
    for (mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
    {
        if (*it == tuple)
        {
            mprselset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::erase_mprsel_tuples(const nsaddr_t & main_addr)
{
    for (mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end();)
    {
        OLSR_mprsel_tuple* tuple = *it;
        if (tuple->main_addr() == main_addr)
        {
            it = mprselset_.erase(it);
            if (mprselset_.empty())
                break;
        }
        else
            it++;

    }
}

void
OLSR_state::insert_mprsel_tuple(OLSR_mprsel_tuple* tuple)
{
    mprselset_.push_back(tuple);
}

/********** Neighbor Set Manipulation **********/

OLSR_nb_tuple*
OLSR_state::find_nb_tuple(const nsaddr_t & main_addr)
{
    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
    {
        OLSR_nb_tuple* tuple = *it;
        if (tuple->nb_main_addr() == main_addr)
            return tuple;
    }
    return NULL;
}

OLSR_nb_tuple*
OLSR_state::find_sym_nb_tuple(const nsaddr_t & main_addr)
{
    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
    {
        OLSR_nb_tuple* tuple = *it;
        if (tuple->nb_main_addr() == main_addr && tuple->getStatus() == OLSR_STATUS_SYM)
            return tuple;
    }
    return NULL;
}

OLSR_nb_tuple*
OLSR_state::find_nb_tuple(const nsaddr_t & main_addr, uint8_t willingness)
{
    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
    {
        OLSR_nb_tuple* tuple = *it;
        if (tuple->nb_main_addr() == main_addr && tuple->willingness() == willingness)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_nb_tuple(OLSR_nb_tuple* tuple)
{
    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
    {
        if (*it == tuple)
        {
            nbset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::erase_nb_tuple(const nsaddr_t & main_addr)
{
    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
    {
        OLSR_nb_tuple* tuple = *it;
        if (tuple->nb_main_addr() == main_addr)
        {
            it = nbset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::insert_nb_tuple(OLSR_nb_tuple* tuple)
{
    nbset_.push_back(tuple);
}

/********** Neighbor 2 Hop Set Manipulation **********/

OLSR_nb2hop_tuple*
OLSR_state::find_nb2hop_tuple(const nsaddr_t & nb_main_addr, const nsaddr_t & nb2hop_addr)
{
    for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
    {
        OLSR_nb2hop_tuple* tuple = *it;
        if (tuple->nb_main_addr() == nb_main_addr && tuple->nb2hop_addr() == nb2hop_addr)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
{
    for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
    {
        if (*it == tuple)
        {
            nb2hopset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::erase_nb2hop_tuples(const nsaddr_t & nb_main_addr, const nsaddr_t & nb2hop_addr)
{
    for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end();)
    {
        OLSR_nb2hop_tuple* tuple = *it;
        if (tuple->nb_main_addr() == nb_main_addr && tuple->nb2hop_addr() == nb2hop_addr)
        {
            it = nb2hopset_.erase(it);
            if (nb2hopset_.empty())
                break;
        }
        else
            it++;

    }
}

void
OLSR_state::erase_nb2hop_tuples(const nsaddr_t & nb_main_addr)
{
    for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end();)
    {
        OLSR_nb2hop_tuple* tuple = *it;
        if (tuple->nb_main_addr() == nb_main_addr)
        {
            it = nb2hopset_.erase(it);
            if (nb2hopset_.empty())
                break;
        }
        else
            it++;

    }
}

void
OLSR_state::insert_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
{
    nb2hopset_.push_back(tuple);
}

/********** MPR Set Manipulation **********/

bool
OLSR_state::find_mpr_addr(const nsaddr_t & addr)
{
    mprset_t::iterator it = mprset_.find(addr);
    return (it != mprset_.end());
}

void
OLSR_state::insert_mpr_addr(const nsaddr_t & addr)
{
    mprset_.insert(addr);
}

void
OLSR_state::clear_mprset()
{
    mprset_.clear();
}

/********** Duplicate Set Manipulation **********/

OLSR_dup_tuple*
OLSR_state::find_dup_tuple(const nsaddr_t & addr, uint16_t seq_num)
{
    for (dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++)
    {
        OLSR_dup_tuple* tuple = *it;
        if (tuple->getAddr() == addr && tuple->seq_num() == seq_num)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_dup_tuple(OLSR_dup_tuple* tuple)
{
    for (dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++)
    {
        if (*it == tuple)
        {
            dupset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::insert_dup_tuple(OLSR_dup_tuple* tuple)
{
    dupset_.push_back(tuple);
}

/********** Link Set Manipulation **********/

OLSR_link_tuple*
OLSR_state::find_link_tuple(const nsaddr_t & iface_addr)
{
    for (linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
    {
        OLSR_link_tuple* tuple = *it;
        if (tuple->nb_iface_addr() == iface_addr)
            return tuple;
    }
    return NULL;
}

OLSR_link_tuple*
OLSR_state::find_sym_link_tuple(const nsaddr_t & iface_addr, double now)
{
    for (linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
    {
        OLSR_link_tuple* tuple = *it;
        if (tuple->nb_iface_addr() == iface_addr)
        {
            if (tuple->sym_time() > now)
                return tuple;
            else
                break;
        }
    }
    return NULL;
}

void
OLSR_state::erase_link_tuple(OLSR_link_tuple* tuple)
{
    for (linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
    {
        if (*it == tuple)
        {
            linkset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::insert_link_tuple(OLSR_link_tuple* tuple)
{
    linkset_.push_back(tuple);
}

/********** Topology Set Manipulation **********/

OLSR_topology_tuple*
OLSR_state::find_topology_tuple(const nsaddr_t & dest_addr, const nsaddr_t & last_addr)
{
    for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
    {
        OLSR_topology_tuple* tuple = *it;
        if (tuple->dest_addr() == dest_addr && tuple->last_addr() == last_addr)
            return tuple;
    }
    return NULL;
}

OLSR_topology_tuple*
OLSR_state::find_newer_topology_tuple(const nsaddr_t &last_addr, uint16_t ansn)
{
    for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
    {
        OLSR_topology_tuple* tuple = *it;
        if (tuple->last_addr() == last_addr && tuple->seq() > ansn)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_topology_tuple(OLSR_topology_tuple* tuple)
{
    for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
    {
        if (*it == tuple)
        {
            topologyset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::erase_older_topology_tuples(const nsaddr_t & last_addr, uint16_t ansn)
{
    for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end();)
    {
        OLSR_topology_tuple* tuple = *it;
        if (tuple->last_addr() == last_addr && tuple->seq() < ansn)
        {
            it = topologyset_.erase(it);
            if (topologyset_.empty())
                break;
        }
        else
            it++;

    }
}

void
OLSR_state::insert_topology_tuple(OLSR_topology_tuple* tuple)
{
    topologyset_.push_back(tuple);
}

/********** Interface Association Set Manipulation **********/

OLSR_iface_assoc_tuple*
OLSR_state::find_ifaceassoc_tuple(const nsaddr_t & iface_addr)
{
    for (ifaceassocset_t::iterator it = ifaceassocset_.begin();
            it != ifaceassocset_.end();
            it++)
    {
        OLSR_iface_assoc_tuple* tuple = *it;
        if (tuple->iface_addr() == iface_addr)
            return tuple;
    }
    return NULL;
}

void
OLSR_state::erase_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple)
{
    for (ifaceassocset_t::iterator it = ifaceassocset_.begin();
            it != ifaceassocset_.end();
            it++)
    {
        if (*it == tuple)
        {
            ifaceassocset_.erase(it);
            break;
        }
    }
}

void
OLSR_state::insert_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple)
{
    ifaceassocset_.push_back(tuple);
}

void OLSR_state::clear_all()
{

    for (linkset_t::iterator it = linkset_.begin(); it != linkset_.end(); it++)
        delete (*it);
    linkset_.clear();

    for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++)
        delete (*it);
    nbset_.clear();
    for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++)
        delete (*it);
    nb2hopset_.clear();
    for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++)
        delete (*it);
    topologyset_.clear();

    for (mprselset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++)
        delete (*it);
    mprselset_.clear();
    for (dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++)
        delete (*it);

    dupset_.clear();
    for (ifaceassocset_t::iterator it = ifaceassocset_.begin(); it != ifaceassocset_.end(); it++)
        delete (*it);
    ifaceassocset_.clear();
    mprset_.clear();

}

OLSR_state::OLSR_state(OLSR_state * st)
{
    for (linkset_t::iterator it = st->linkset_.begin(); it != st->linkset_.end(); it++)
    {
        OLSR_link_tuple* tuple = *it;
        linkset_.push_back(tuple->dup());
    }

    for (nbset_t::iterator it = st->nbset_.begin(); it != st->nbset_.end(); it++)
    {
        OLSR_nb_tuple* tuple = *it;
        nbset_.push_back(tuple->dup());
    }

    for (nb2hopset_t::iterator it = st->nb2hopset_.begin(); it != st->nb2hopset_.end(); it++)
    {
        OLSR_nb2hop_tuple* tuple = *it;
        nb2hopset_.push_back(tuple->dup());
    }

    for (topologyset_t::iterator it = st->topologyset_.begin(); it != st->topologyset_.end(); it++)
    {
        OLSR_topology_tuple* tuple = *it;
        topologyset_.push_back(tuple->dup());
    }

    for (mprset_t::iterator it = st->mprset_.begin(); it != st->mprset_.end(); it++)
    {
        mprset_.insert(*it);
    }

    for (mprselset_t::iterator it = st->mprselset_.begin(); it != st->mprselset_.end(); it++)
    {
        OLSR_mprsel_tuple* tuple = *it;
        mprselset_.push_back(tuple->dup());
    }

    for (dupset_t::iterator it = st->dupset_.begin(); it != st->dupset_.end(); it++)
    {
        OLSR_dup_tuple* tuple = *it;
        dupset_.push_back(tuple->dup());
    }

    for (ifaceassocset_t::iterator it = st->ifaceassocset_.begin(); it != st->ifaceassocset_.end(); it++)
    {
        OLSR_iface_assoc_tuple* tuple = *it;
        ifaceassocset_.push_back(tuple->dup());
    }
}


OLSR_state::~OLSR_state()
{
    clear_all();
}

