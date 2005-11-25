//
// Copyright (C) 2005 Andras Varga
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

#include "ChannelInstaller.h"

Define_Module(ChannelInstaller);


void ChannelInstaller::initialize()
{
    int count = 0;
    for (int i=0; i<=simulation.lastModuleId(); i++)
    {
        cModule *mod = simulation.module(i);
        if (!mod) continue;
        int numGates = mod->gates();
        for (int j=0; j<numGates; j++)
        {
            cGate *g = mod->gate(j);
            if (!g) continue;
            cChannel *channel = g->channel();
            if (!channel) continue;
            g->setChannel(createReplacementChannelFor(channel));
            count++;
        }
    }

    EV << "ChannelInstaller replaced " << count << " channel objects.\n";
}

cChannel *ChannelInstaller::createReplacementChannelFor(cChannel *channel)
{
    cBasicChannel *oldchan = dynamic_cast<cBasicChannel *>(channel);
    if (!oldchan)
        return channel;

    // create new channel object of the given class, and take over the original object's attributes
    const char *channelClassName = par("channelClass");
    cBasicChannel *newchan = check_and_cast<cBasicChannel *>(createOne(channelClassName));
    newchan->setName(oldchan->name());
    newchan->setError(oldchan->error());
    newchan->setDelay(oldchan->delay());
    newchan->setDatarate(oldchan->datarate());

    // parse the "attr=value;attr=value;.." string, and set the given attributes on the channel
    const char *attrs = par("channelAttrs");
    cStringTokenizer tok(attrs,";");
    while (tok.hasMoreTokens())
    {
        cStringTokenizer tok2(tok.nextToken(), "=");
        const char *attrname = tok2.nextToken();
        const char *value = tok2.nextToken();
        cPar& p = newchan->addPar(attrname);
        if (!p.setFromText(value))
            p.setStringValue(value);
    }

    return newchan;
}

void ChannelInstaller::handleMessage(cMessage *msg)
{
}


