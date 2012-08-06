//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2008 Alfonso Ariza
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


#include <fstream>
#include <sstream>
#include <string>

#include "Ns2MotionMobility.h"
#include "FWMath.h"

#ifndef atoi
#include <cstdlib>
#endif


Define_Module(Ns2MotionMobility);


Ns2MotionMobility::Ns2MotionMobility()
{
    vecpos = 0;
    ns2File = NULL;
    nodeId = 0;
    scrollX = 0;
    scrollY = 0;
}

Ns2MotionMobility::~Ns2MotionMobility()
{
    if (ns2File)
        delete ns2File;
}

void Ns2MotionMobility::parseFile(const char *filename)
{

    std::ifstream in(filename, std::ios::in);

    if (in.fail())
        throw cRuntimeError("Cannot open file '%s'", filename);
    ns2File->initial[0] = ns2File->initial[1] = ns2File->initial[2] = -1;
    std::string line;
    std::string subline;

    while (std::getline(in, line))
    {
        // '#' line
        int num_node;
        std::string::size_type found = line.find('#');
        if (found == 0)
            continue;
        if (found != std::string::npos)
            subline = line;
        else
            subline = line.substr(0, found);
        found = subline.find("$node_");
        if (found == std::string::npos)
            continue;
        // Node Id
        std::string::size_type pos1 = subline.find('(');
        std::string::size_type pos2 = subline.find(')');
        if (pos2-pos1>1)
            num_node = std::atoi(subline.substr(pos1+1, pos2-1).c_str());
        if (num_node!=nodeId)
            continue;
        // Initial position
        found = subline.find("set ");
        if (found!=std::string::npos)
        {
            // Initial position
            found = subline.find("X_");
            if (found!=std::string::npos)
            {
                ns2File->initial[0] = std::atof(subline.substr(found+3, std::string::npos).c_str());
            }
            found = subline.find("Y_");
            if (found!=std::string::npos)
            {
                ns2File->initial[1] = std::atof(subline.substr(found+3, std::string::npos).c_str());
            }

            found = subline.find("Z_");
            if (found!=std::string::npos)
            {
                ns2File->initial[2] = std::atof(subline.substr(found+3, std::string::npos).c_str());
            }
        }
        found = subline.find("setdest");
        if (found!=std::string::npos)
        {
            ns2File->lines.push_back(Ns2MotionFile::Line());
            Ns2MotionFile::Line& vec = ns2File->lines.back();
            // initial time
            found = subline.find("at");
            vec.push_back(std::atof(subline.substr(found+3).c_str()));

            std::string parameters = subline.substr(subline.find("setdest ")+8, std::string::npos);

            std::stringstream linestream(parameters);
            double d;
            while (linestream >> d)
                vec.push_back(d);
        }
    }
    in.close();
    // exist data?
    if (ns2File->initial[0]==-1 || ns2File->initial[1]==-1 || ns2File->initial[2]==-1)
        throw cRuntimeError("node '%d' Error ns2 motion file '%s'", nodeId, filename);

}

void Ns2MotionMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);
    EV << "initializing Ns2MotionMobility stage " << stage << endl;
    if (stage == 0)
    {
        scrollX = par("scrollX");
        scrollY = par("scrollY");
        nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getParentModule()->getIndex();
        const char *fname = par("traceFile");
        ns2File = new Ns2MotionFile;
        parseFile(fname);
        vecpos = 0;
        WATCH(nodeId);
    }
}

void Ns2MotionMobility::initializePosition()
{
    lastPosition.x = ns2File->initial[0]+scrollX;
    lastPosition.y = ns2File->initial[1]+scrollY;
}

void Ns2MotionMobility::setTargetPosition()
{

    if (ns2File->lines.size()==0)
    {
        stationary = true;
        return;
    }

    if (vecpos >= ns2File->lines.size())
    {
        stationary = true;
        return;
    }


    if (ns2File->lines.begin()+vecpos == ns2File->lines.end())
    {
        stationary = true;
        return;
    }

    const Ns2MotionFile::Line& vec = ns2File->lines[vecpos];
    double time = vec[0];
    simtime_t now = simTime();
    // TODO: this code is dubious at best
    if (now < time)
    {
        nextChange = time;
        targetPosition = lastPosition;
    }
    else if (vec[3] == 0) // the node is stopped
    {
        const Ns2MotionFile::Line& vec = ns2File->lines[vecpos+1];
        double time = vec[0];
        nextChange = time;
        targetPosition = lastPosition;
        vecpos++;
    }
    else
    {
        targetPosition.x = vec[1]+scrollX;
        targetPosition.y = vec[2]+scrollY;
        double speed = vec[3];
        double distance = lastPosition.distance(targetPosition);
        double travelTime = distance / speed;
        nextChange = now + travelTime;
        vecpos++;
    }
    EV << "TARGET: t=" << nextChange << " (" << targetPosition.x << "," << targetPosition.y << ")\n";
}

void Ns2MotionMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}
