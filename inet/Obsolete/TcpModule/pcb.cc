// -*- C++ -*-
//
//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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

// Description:
//      Implementation of a Protocol Control Block

#include "pcb.h"

using std::ostream;

Register_Class(PCB);

void PCB::_init()
{
  //_faddr = IPADDRESS_UNDEF;
  _fport = PORT_UNDEF;
  //_faddr = IPADDRESS_UNDEF;
  _fport = PORT_UNDEF;
}


PCB::PCB(const PCB& pcb) : cObject()
{
  setName(pcb.name());
  operator=(pcb);
}

PCB::PCB(IPAddress laddr, PortNumber lport, IPAddress faddr, PortNumber fport) : cObject()
{
  _init();

  _faddr = faddr;
  _fport = fport;
  _laddr = laddr;
  _lport = lport;
}


PCB::PCB() : cObject("PCB")
{
  _init();
}

PCB::PCB(const char* name) : cObject(name)
{
  _init();
}

PCB::~PCB()
{
}

void PCB::info(char* buf)
{
  cObject::info(buf);
  sprintf(buf + strlen(buf), " PCB: Local: %s, %d; Remote: %s, %d",
          _laddr.str().c_str(), (int) _lport,  _faddr.str().c_str(), (int) _fport);
}

void PCB::writeContents(ostream& os)
{
  cObject::writeContents(os);
  os << " PCB:" << endl
     << "  Local Address:   " << _laddr << endl
     << "  Local Port:      " << _lport << endl
     << "  Foreign Address: " << _faddr << endl
     << "  Foreign Port:    " << _fport << endl;
}


PCB& PCB::operator=(const PCB& pcb)
{
  if (this != &pcb)
    {
      cObject::operator=(pcb);

      _faddr = pcb.fAddr();
      _fport = pcb.fPort();
      _laddr = pcb.lAddr();
      _lport = pcb.fPort();
    }

  return *this;
}
