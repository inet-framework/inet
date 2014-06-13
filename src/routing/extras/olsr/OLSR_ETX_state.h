/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   Modified by Weverton Cordeiro                                         *
 *   (C) 2007 wevertoncordeiro@gmail.com                                   *
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
/// \file  OLSR_ETX_state.h
/// \brief  This header file declares and defines internal state of an OLSR_ETX node.
///

#ifndef __OLSR_ETX_state_h__
#define __OLSR_ETX_state_h__

#include <OLSR_ETX_repositories.h>
#include <OLSR_state.h>

/// This class encapsulates all data structures needed for maintaining internal state of an OLSR_ETX node.
class OLSR_ETX_state : public OLSR_state
{
    friend class OLSR_ETX;
    OLSR_ETX_parameter *parameter;
  protected:
    OLSR_ETX_link_tuple*  find_best_sym_link_tuple(const nsaddr_t &main_addr, double now);
    OLSR_ETX_state();
};

#endif
