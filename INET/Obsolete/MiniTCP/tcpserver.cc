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

#include <omnetpp.h>
#include "nw.h"
#include "tcpserver.h"

class TcpServer : public cSimpleModule
{
private:
    void receiveSyn(TcpTcb* block, cMessage* msg);
    void sendSyn(TcpTcb* block);
    void receiveAck(TcpTcb* block, cMessage* msg);
    void checkAck(TcpTcb* block);
    void sendData(TcpTcb* block);
    void timeRetrans(TcpTcb* block, cMessage* msg);

    cOutVector cwnd_size;
    cOutVector send_seq_no;
    cOutVector rec_ack_no;
    cOutVector through;
    cOutVector total_throughput;

    //cMessage* msgtime;
    //double rtime;

    // msg for retransmission timeout
    //cMessage* msgtime;

    bool debug;

    // totla msg length in byte
    long msgl;

    // TCP throughput
    double th_put;

    double sum;
    // Total TCP throughput
    double total_t;
    // count for calculation of Total TCP throughput
    int count;

    double dact_msg;
    int act_msg;

public:
    Module_Class_Members(TcpServer, cSimpleModule, 16384);

    //virtual functions to be redefined
    // virtual void initialize();
    virtual void activity();
};

Define_Module(TcpServer);

void TcpServer::activity()
{
    cwnd_size.setName("congestion window");
    send_seq_no.setName("Send No");
    rec_ack_no.setName("Rec ACK No");
    through.setName("Throughput");
    total_throughput.setName("Total throughput");

    // create a TcpTcb struct defined in header file, with name "block"
    TcpTcb* block = new TcpTcb;

    debug = par("debug");
    block->snd_mss = par("snd_mss");
    long conw = par("snd_mss");
    block->cwn_wnd = conw - 1;
    // total msg_length in byte
    block->msg_length = par("msg_length");
    msgl = par("msg_length");
    block->sstth = 65535L;
    block->snd_wnd = 16000L;
    block->snd_nxt = 0L;
    block->snd_una = 0L;
    block->dupacks = 0;
    block->save_snd_ack = 0L;
    block->th_snd_ack = 0L;
    block->rt_time = 1.5;

    sum = 0.0;
    total_t = 0.0;
    count = 0;

    //msgtime = new cMessage("TIME_OUT", TIME_OUT);
    //scheduleAt(simTime()+10.0, msgtime);


    for(;;)
    {
        cMessage* msgin = receive();
        int type = msgin->kind();

        switch(type)
        {
        case SYN:
            receiveSyn(block, msgin);
            break;

        case ACK:
            receiveAck(block, msgin);

            break;

        case TIME_OUT:
            timeRetrans(block, msgin);
            break;

        default:
            error("Unexpected msg received in server, msg kind is %d", msgin->kind());
        } // switch(type)

    } // for(;;)

}



void TcpServer:: receiveSyn(TcpTcb* block, cMessage* msg)
{
    block->snd_ack = msg->par("seq_num");
    block->snd_ack++;
    delete msg;

    sendSyn(block);
}



void TcpServer:: sendSyn(TcpTcb* block)
{
    cMessage* msgsyn = new cMessage("SYN", SYN);
    // Server's ISS = 0
    msgsyn->addPar("seq_num") = block->snd_nxt;
    send_seq_no.record(block->snd_nxt);
    msgsyn->addPar("ack_num") = block->snd_ack;
    // SYN consumes 1 byte
    msgsyn->setLength( (1 + TCP_HEAD) * 8);
    send(msgsyn, "to_ip");

    block->snd_nxt++;
}



void TcpServer:: receiveAck(TcpTcb* block, cMessage* msg)
{
    long s = msg->par("seq_num");
    if(block->snd_ack == s)
    {
        block->snd_ack = block->snd_ack + ( msg->length()/8 - TCP_HEAD );
    }

    block->rcv_ack = msg->par("ack_num");
    rec_ack_no.record(block->rcv_ack);
    delete msg;

    // if all msg received, end simulation
    if(block->rcv_ack > msgl)
    {
        ev << " All messages sent and acked " << endl;
        ev << " Received ACK No. is " << block->rcv_ack << endl;
        ev << " Total msg length is " << msgl << endl;
        ev << " ending simulation " << endl;

        endSimulation();
    }

    checkAck(block);
}



void TcpServer:: sendData(TcpTcb* block)
{

    block->total_wnd = (block->snd_wnd < block->cwn_wnd)? block->snd_wnd : block->cwn_wnd;
    block->avail_wnd = (block->total_wnd + block->snd_una) - (block->snd_nxt);

    if(block->avail_wnd > 0)
    {
        // for retransmission timer
        double rtime = block->rt_time;

        dact_msg  = (double) block->avail_wnd / block->snd_mss;
        act_msg = ceil(dact_msg);

        for (int i=0; i<act_msg; i++)
        {
            block->seg_length = MIN( MIN(block->avail_wnd, block->snd_mss), block->msg_length);
            if(block->seg_length <= 0)
                break;

            block->snd_wnd -= block->seg_length;
            block->msg_length -= block->seg_length;

            cMessage *msgout = new cMessage("DATA", DATA);
            msgout->addPar("seq_num") = block->snd_nxt;
            msgout->addPar("ack_num") = block->snd_ack;
            msgout->setLength( (block->seg_length + TCP_HEAD) * 8);
            send(msgout, "to_ip");
            send_seq_no.record(block->snd_nxt);

            // ******** for Retransmission TIME_OUT msg ********
            //
            cMessage* copy = new cMessage("TIME_OUT", TIME_OUT);
            copy->addPar("seq_num") = block->snd_nxt;
            copy->addPar("ack_num") = block->snd_ack;
            copy->addPar("seg_len") = block->seg_length;
            copy->addPar("rtime") = rtime;
            scheduleAt(simTime()+rtime, copy);


            block->snd_nxt += block->seg_length ;
            if(block->msg_length <= 0)
                break;
        } // for (long i=0; i<act_msg; i++)

    } // if(block->avail_wnd > 0)

}



void TcpServer:: checkAck(TcpTcb* block)
{
    if(block->rcv_ack > 1) count++;

    // normal ACK received
    //
    if( (block->rcv_ack <= block->snd_nxt) && (block->rcv_ack > block->snd_una) )
    {
        block->dupacks = 0;

        // increase cwn_wnd and snd_wnd as many as acked bytes
        block->snd_wnd = block->snd_wnd + (block->rcv_ack - block->snd_una);

        if(block->cwn_wnd <= block->sstth)
        {
            block->cwn_wnd = block->cwn_wnd + (block->rcv_ack - block->snd_una);
            cwnd_size.record(block->cwn_wnd);
        }
        // Congestion Avoidance
        else
        {
            block->cwn_wnd = block->cwn_wnd + (block->snd_mss * block->snd_mss)/block->cwn_wnd + block->snd_mss/8;
            cwnd_size.record(block->cwn_wnd);
        }

        // update block->snd_una to block->rcv_ack
        block->snd_una = block->rcv_ack;

        if(block->rcv_ack > 1)
        {
            th_put = (double) block->rcv_ack / simTime();
            through.record(th_put);
        }

        sendData(block);

    } // if( (block->rcv_ack <= block->snd_nxt) && (block->rcv_ack > block->snd_una) )

    // ACK for not yet sent data
    //
    else if(block->rcv_ack > block->snd_nxt)
    {
        error("TCP error: Unsent data acked");
    }

    // Duplicate ACK
    //
    else if(block->rcv_ack <= block->snd_una)
    {
        if(debug) ev << " Server received duplicate ack " << endl;
        block->dupacks++;

        if(block->dupacks == 3)
        {
            if(debug) ev << " Server received 3 duplicate acks " << endl;
            if(debug) ev << " Performing retransmission " << endl;

            block->sstth = MAX(MIN(block->snd_wnd, block->cwn_wnd)/2, 2*block->snd_mss);
            block->cwn_wnd = block->sstth + 3 * block->snd_mss;
            cwnd_size.record(block->cwn_wnd);

            // Retransmission
            cMessage* msgrt = new cMessage("DATA", DATA);
            msgrt->addPar("seq_num") = block->rcv_ack;
            msgrt->addPar("ack_num") = block->snd_ack;

            // FIXME originally not 2*block->snd_mss but block->snd_mss
            msgrt->setLength( (block->snd_mss + TCP_HEAD) * 8);
            send(msgrt, "to_ip");

            send_seq_no.record(block->rcv_ack);

        } // if(block->dupacks == 3)


        else if(block->dupacks > 3)
        {
            block->cwn_wnd += block->snd_mss;
            cwnd_size.record(block->cwn_wnd);

            sendData(block);
        }

        th_put = (double) block->rcv_ack / simTime();
        through.record(th_put);

    } // else if(block->rcv_ack == block->snd_una)

    if(block->rcv_ack > 1)
    {
        sum += th_put;
        total_t = sum / (double) count;
        total_throughput.record(total_t);
    }

}



void TcpServer:: timeRetrans(TcpTcb* block, cMessage* msg)
{
    long seq = msg->par("seq_num");
    long ack = msg->par("ack_num");

    if(seq >= block->snd_una)
    {
        double rt_1 = msg->par("rtime");
        double rt_2 = 2 * rt_1;
        if(rt_2 > 64.0)
        {
            rt_2 = 64.0;
        }

        long seg = msg->par("seg_len");

        delete msg;

        cMessage* msgout = new cMessage("DATA", DATA);
        msgout->addPar("seq_num") = seq;
        msgout->addPar("ack_num") = ack;
        msgout->setLength( (seg + TCP_HEAD) * 8);
        send(msgout, "to_ip");
        send_seq_no.record(seq);

        // for next retransmission timeout msg
        //
        cMessage* copi = new cMessage("TIME_OUT", TIME_OUT);
        copi->addPar("seq_num") = seq;
        copi->addPar("ack_num") = ack;
        copi->addPar("seg_len") = seg;
        copi->addPar("rtime") = rt_2;
        scheduleAt(simTime()+rt_2, copi);


        // reset block values for congestion avoidance
        //
        block->cwn_wnd = block->snd_mss;
        cwnd_size.record(block->cwn_wnd);
        block->sstth = 2 * block->snd_mss;
    }

    else
    {
        delete msg;
    }

}