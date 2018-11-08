/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
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
/// \file   OLSR.h
/// \brief  Header file for OLSR agent and related classes.
///
/// Here are defined all timers used by OLSR, including those for managing internal
/// state and those for sending messages. Class OLSR is also defined, therefore this
/// file has signatures for the most important methods. Lots of constants are also
/// defined.
///

#ifndef __OLSROPT_omnet_h__

#define __OLSROPT_omnet_h__
#include "inet/routing/extras/olsr/Olrs.h"

namespace inet {

namespace inetmanet {

class OlsrOpt : public Olsr
{
  private:
    friend class Olsr_HelloTimer;
    friend class Olsr_TcTimer;
    friend class Olsr_MidTimer;
    friend class Olsr_DupTupleTimer;
    friend class Olsr_LinkTupleTimer;
    friend class Olsr_Nb2hopTupleTimer;
    friend class Olsr_MprSelTupleTimer;
    friend class Olsr_TopologyTupleTimer;
    friend class Olsr_IfaceAssocTupleTimer;
    friend class Olsr_MsgTimer;
    friend class Olsr_Timer;

  protected:

    virtual bool        link_sensing(OlsrMsg&, const nsaddr_t &, const nsaddr_t &, const int &) override;
    virtual bool        populate_nbset(OlsrMsg&) override;
    virtual bool        populate_nb2hopset(OlsrMsg&) override;

    virtual void        recv_olsr(Packet *) override;

    virtual bool        process_hello(OlsrMsg&, const nsaddr_t &, const nsaddr_t &, const int &) override;
    virtual bool        process_tc(OlsrMsg&, const nsaddr_t &, const int &) override;
    virtual int         update_topology_tuples(OlsrMsg& msg, int index);
    virtual void nb_loss(Olsr_link_tuple* tuple) override;

};

} // namespace inetmanet

} // namespace inet

#endif

