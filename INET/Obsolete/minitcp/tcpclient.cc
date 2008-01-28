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

class TcpClient : public cSimpleModule
{
private:
    void receiveSyn(TcpTcb* bloc, cMessage* msg);
    void sendSyn(TcpTcb* bloc);
    void checkSeq(TcpTcb* bloc, cMessage* msg);
    void sendAck(TcpTcb* bloc);

    cOutVector rec_seq_no;

public:
  Module_Class_Members(TcpClient, cSimpleModule, 16384);

  virtual void activity();
};

Define_Module(TcpClient);

void TcpClient::activity()
{

    rec_seq_no.setName("Rec Seq No");

    // create a TcpTcb struct defined in header file, with name "bloc"
    TcpTcb* bloc = new TcpTcb;

    bool debug = par("debug");
    bloc->snd_mss = par("snd_mss");
    bloc->cwn_wnd = par("snd_mss");
    // total msg_length in byte
    bloc->msg_length = par("msg_length");

    bloc->sstth = 65535L;
    bloc->snd_wnd = 65535L;
    bloc->snd_nxt = 1000L;
    bloc->snd_una = 1000L;
    bloc->dupacks = 0;
    bloc->save_snd_ack = 0L;
    bloc->th_snd_ack = 0L;
    bloc->rt_time = 1.5L;


    for(;;)
    {
        cMessage* msgin = receive();
        int type = msgin->kind();

        switch(type)
        {
        // open_active call from applclient
        case OPEN_ACTIVE:
            delete msgin;
            sendSyn(bloc);
            break;

        case SYN:
            receiveSyn(bloc, msgin);
            break;

        case DATA:
            checkSeq(bloc, msgin);

            break;

        default:
            ev << " ******** error ******** " << endl;
            ev << " unexpected msg received in client, msg kind is " << msgin->kind() << endl;
        } // switch(type)

    } // for(;;)

}



void TcpClient:: sendSyn(TcpTcb* bloc)
{
    cMessage* msgsyn = new cMessage("SYN", SYN);
    // Client's ISS = 1000
    msgsyn->addPar("seq_num") = bloc->snd_nxt;
    // SYN consumes 1 byte
    msgsyn->setLength( (1 + TCP_HEAD) * 8);
    send(msgsyn, "to_ip");

    bloc->snd_nxt++;
}



void TcpClient:: receiveSyn(TcpTcb* bloc, cMessage* msg)
{
    bloc->snd_ack = msg->par("seq_num");
    bloc->snd_ack++;
    delete msg;

    bloc->snd_una = bloc->snd_nxt;
    sendAck(bloc);
}



void TcpClient:: sendAck(TcpTcb* bloc)
{
    bloc->seg_length = 0;

    cMessage *msgout = new cMessage("ACK", ACK);
    msgout->addPar("seq_num") = bloc->snd_nxt;
    msgout->addPar("ack_num") = bloc->snd_ack;
    msgout->setLength( (bloc->seg_length + TCP_HEAD) * 8);
    send(msgout, "to_ip");

}



void TcpClient:: checkSeq(TcpTcb* bloc, cMessage* msg)
{
    long seq = msg->par("seq_num");

    // normal Seq received
    //
    if(bloc->snd_ack == seq)
    {
        rec_seq_no.record(seq);

        bloc->snd_ack = bloc->snd_ack + msg->length()/8 - TCP_HEAD;

        // All lost segments are now received
        //
        if( (bloc->th_snd_ack != 0) && (bloc->snd_ack >= bloc->th_snd_ack) )
        {
            long m_len = (long) msg->length()/8 - TCP_HEAD;
            long a_len = bloc->save_snd_ack - bloc->snd_ack + m_len;

            // in case retransmitted segment exceed bloc->save_snd_ack
            // the exceeded mount(= m_len - a_len) must be added to bloc->snd_ack
            if(m_len > a_len)
            {
                bloc->snd_ack = bloc->save_snd_ack + (m_len - a_len);
            }

            // update snd_ack to save_snd_ack
            else
            {
                bloc->snd_ack = bloc->save_snd_ack;
            }

            // reset th_snd_ack & save_snd_ack to 0
            bloc->th_snd_ack = 0;
            bloc->save_snd_ack = 0;
        }

        sendAck(bloc);

    } // if(bloc->snd_ack == seq)


    // duplicate segment
    // ignore, not sendAck
    //
    else if(bloc->snd_ack > seq)
    {
        ev << " ****** error ********* " << endl;
        ev << " duplicate segment received " << endl;
    }


    // Some segment(s) lost
    //
    else if(bloc->snd_ack < seq)
    {
        //rec_seq_no.record(seq);

        // end byte + 1 byte of lost segment
        if(bloc->th_snd_ack == 0)
            bloc->th_snd_ack = seq;

        // end byte + 1 byte of totally received segment up to now
        // After receiving all lost segment, bloc->snd_ack = bloc->save_snd_ack
        bloc->save_snd_ack = seq + msg->length()/8 - TCP_HEAD;

        sendAck(bloc);
    }

    delete msg;

}
