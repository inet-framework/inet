// -*- C++ -*-
//
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

/*
    file: PPPFrame.h
    Purpose: PPP frame format definition
    author: Jochen Reber
*/


#ifndef __PPPFRAME_H
#define __PPPFRAME_H

#include <iostream>
#include <omnetpp.h>

using std::ostream;

/*  -------------------------------------------------
        Constants
    -------------------------------------------------   */

/* PPP header length:
    2*flag + 1 add + 1 contr + 2 prot = 6 byte */
const int PPP_HEADER_LENGTH = 6;


/*  -------------------------------------------------
        Enumerations
    -------------------------------------------------   */

enum PPP_ProtocolFieldId
{
    PPP_PROT_UNDEF    = 0x0,
    PPP_PROT_IP     = 0x0021,
    PPP_PROT_LCD    = 0xc021,
    PPP_PROT_NCD    = 0x8021
};

/*  -------------------------------------------------
        Main class: PPPFrame
    -------------------------------------------------
    field simulated:
        protocol
    constant fields not simulated:
        flag (0x7e), address (0xff), control (0x03),
        CRC (biterror)

*/

class PPPFrame: public cPacket
{
private:
    PPP_ProtocolFieldId _protocol;

public:

    // constructors
    PPPFrame();
    PPPFrame(const PPPFrame &p);

    // assignment operator
    virtual PPPFrame& operator=(const PPPFrame& p);
    virtual cObject *dup() const { return new PPPFrame(*this); }

    // info functions
    virtual void info(char *buf);
    virtual void writeContents(ostream& os);

    // header length functions
    int headerBitLength() { return 8 * PPP_HEADER_LENGTH; }
    int headerByteLength() { return PPP_HEADER_LENGTH; }

    // PPP fields
    PPP_ProtocolFieldId protocol() { return _protocol; }
    void setProtocol( PPP_ProtocolFieldId p ) { _protocol = p; }

};

#endif


