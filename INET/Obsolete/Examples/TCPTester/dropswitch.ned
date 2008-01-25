//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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
// Modified switch
//
simple DropSwitch
{
    parameters:
        volatile double server_port;
        volatile double pk_delay;
        volatile double burst_delete_probability; // prob that a burst will be deleted
        volatile double error_burst_len;    // number of packets do be dropped in case of burst error
        volatile double delete_probability; // in percent !!!
        volatile double syn_client_del;
        volatile double syn_server_del;
        volatile double ack_client_del;
        volatile double ack_server_del;
        volatile double tcp_server_del;
        volatile double tcp_client_del;
        volatile double fin_client_del;
        volatile double fin_server_del;
    gates:
        input in[];
        output out[];
}
