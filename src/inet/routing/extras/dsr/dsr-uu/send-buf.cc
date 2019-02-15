/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

namespace inet {

namespace inetmanet {


void NSCLASS send_buf_set_max_len(unsigned int max_len)
{
    buffMaxlen = max_len;
}

void NSCLASS send_buf_timeout(void *data)
{

    if (packetBuffer.empty())
        return;

    simtime_t now = simTime();
    simtime_t minTimeout = SimTime::getMaxTime();

    double timeout = ((double)ConfValToUsecs(SendBufferTimeout)/1000000.0);
    int pkts = 0;


    for (auto it = packetBuffer.begin();it != packetBuffer.end();)
    {
        double packetTimeout = SIMTIME_DBL(now - it->second.time)+0.01;// this value avoid problems of rounds, if the time to discard this packet is less than 0.01 seconds discard this packet also
        if (packetTimeout >= timeout)
        {
            dsr_pkt_free(it->second.packet);
            packetBuffer.erase(it++);
            pkts++;
        }
        else
        {
            if (it->second.time < minTimeout)
                minTimeout = it->second.time;
            ++it;
        }
    }

    if (packetBuffer.empty())
        return;

    DEBUG("%d packets garbage collected\n", pkts);
    struct timeval expires;
    double nextPacketTimeout = timeout - SIMTIME_DBL(now - minTimeout); // next timeout relative time
    minTimeout = now + nextPacketTimeout; // next timeout absolute time

    timevalFromSimTime(&expires,minTimeout);
    set_timer(&send_buf_timer, &expires);

}

int NSCLASS send_buf_enqueue_packet(struct dsr_pkt *dp)
{
    PacketStoreage entry;

    entry.time = simTime();
    entry.packet = dp;
    L3Address destination = dp->dst.s_addr;
    if (packetBuffer.empty())
    {
        struct timeval expires;
        double timeout = ((double)ConfValToUsecs(SendBufferTimeout)/1000000.0);
        timevalFromSimTime(&expires,entry.time+timeout);
        set_timer(&send_buf_timer, &expires);
    }
    else if (packetBuffer.size() >= buffMaxlen)
    {
        // delete oldest
        auto itAux = packetBuffer.begin();
        for (auto it = packetBuffer.begin();it != packetBuffer.end();++it)
        {
            if (itAux->second.time > it->second.time)
                itAux = it;
        }
        packetBuffer.erase(itAux);
    }
    packetBuffer.insert(std::make_pair(destination,entry));
    return 0;
}

int NSCLASS send_buf_set_verdict(int verdict, struct in_addr dst)
{
    if (packetBuffer.empty())
        return 0;  // nothing to-to
    L3Address destination = dst.s_addr;
    std::pair <PacketBuffer::iterator, PacketBuffer::iterator> ret;
    ret = packetBuffer.equal_range(destination);

    int pkts = 0;
    if(ret.first == ret.second)
        return pkts; // no packets

    for (auto it=ret.first; it!=ret.second; ++it)
    {
        struct dsr_pkt *dp = it->second.packet;
        pkts++;
        switch (verdict)
        {
            case SEND_BUF_DROP:
                dsr_pkt_free(dp);
                break;
            case SEND_BUF_SEND:
                dp->srt = dsr_rtc_find(my_addr(), dp->dst);
                if (dp->srt)
                {
                    if (dsr_srt_add(dp) < 0)
                    {
                        DEBUG("Could not add source route\n");
                        dsr_pkt_free(dp);
                    }
                    else
                        AddCost(dp,dp->srt);
                    /* Send packet */
                    omnet_xmit(dp);
                }
                else
                {
                    DEBUG("No source route found for %s!\n",print_ip(dst));
                    dsr_pkt_free(dp);
                }
                break;
        }
    }
    packetBuffer.erase(ret.first, ret.second);
    return pkts;
 }


int NSCLASS send_buf_init(void)
{
    buffMaxlen = SEND_BUF_MAX_LEN;
    init_timer(&send_buf_timer);
    send_buf_timer.function = &NSCLASS send_buf_timeout;
    return 1;
}

void NSCLASS send_buf_cleanup(void)
{
    int pkts = 0;
    if (timer_pending(&send_buf_timer))
        del_timer_sync(&send_buf_timer);

    while (!packetBuffer.empty())
    {
        dsr_pkt_free(packetBuffer.begin()->second.packet);
        pkts++;
        packetBuffer.erase(packetBuffer.begin());
    }
    DEBUG("Flushed %d packets\n", pkts);
}


} // namespace inetmanet

} // namespace inet

