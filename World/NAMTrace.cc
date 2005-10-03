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

#include "NAMTraceWriter.h"

Define_Module(NAMTraceWriter);


void NAMTraceWriter::initialize()
{
    lastnamid = 0;
    namfb = NULL;
    nams = NULL;
    const char *namlog = par("logfile");
    if (namelog && namlog[0])
    {
        ev << "nam tracing enabled (file " << namlog << ")" << endl;
        namfb = new std::filebuf();
        namfb->open(namlog, std::ios::out | std::ios::app);
        nams = new std::ostream(namfb);

        const char *prolog = par("prolog");
        if (strlen(prolog))
        {
            cStringTokenizer tokenizer(prolog, ";");
            const char *token;
            while((token = tokenizer.nextToken())!=NULL)
                    *nams << token << endl;
            *nams << std::flush;
        }
    }
}

void NAMTraceWriter::handleMessage(cMessage *msg)
{
    error("This module doesn't process messages");
}

void NAMTraceWriter::finish()
{
    if (fnamfb)
        namfb->close();
}

int NAMTraceWriter::setNamId(cModule *node, int namid)
{
    modid2namid[node->id()] = namid==-1 ? ++lastnamid : namid;
}

int NAMTraceWriter::getNamId(cModule *node)
{
    int modid = node->id();
    std::map<int,int>::iterator it = modid2namid.find(modid);
    if (it == modid2namid.end())
        error("getNamId(): setNamId() on module '%s' not yet called", node->fullPath().c_str());
    return it->second;
}

std::ostream NAMTraceWriter::log()
{
    if (!nams)
        error("Cannot write to (%s)%s log: log not open", className(), fullPath().c_str());
    return *nams;
}
