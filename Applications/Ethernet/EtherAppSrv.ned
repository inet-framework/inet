//
// Copyright (C) 2003 CTIE, Monash University
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


//
// Server side of the EtherCli model -- generates EtherAppResp packets
// with the number of bytes requested by the client in EtherAppReq.
// It should be connected directly to EtherLLC or a Ieee802NicXXX module.
//
simple EtherAppSrv
    parameters:
        registerSAP : bool, // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        writeScalars: bool; // enable/disable recording statistics in omnetpp.sca
    gates:
        in: in;
        out: out;
endsimple
