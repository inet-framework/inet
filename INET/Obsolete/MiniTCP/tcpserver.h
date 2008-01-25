//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Copyright 2004 Joungwoong Lee (zipizigi), University of Tsukuba
//

#ifndef _MINI_TCPSERVER_H_
#define _MINI_TCPSERVER_H_

#include <omnetpp.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
//#define AVAIL(a, b, c)    ((a)+(b)-(c))
#define TCP_HEAD 20

struct TcpTcb
{
    long cwn_wnd, snd_wnd, avail_wnd, total_wnd;
    long snd_ack, rcv_ack, save_snd_ack, th_snd_ack, snd_una, snd_nxt, sstth;
    long msg_length, seg_length, snd_mss;

    short dupacks;

    double rt_time;
};

#endif // _TCPSERVER_H_