//
// Author: Marcin Kosiba marcin.kosiba@gmail.com
// Copyright (C) 2009 Marcin Kosiba
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

#include "ChiangMobility.h"

Define_Module(ChiangMobility);

void ChiangMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing ChiangMobility stage " << stage << endl;

    if (stage == 0)
    {
        m_updateInterval = par("updateInterval");
        m_stateTransInterval = par("stateTransitionUpdateInterval");
        m_speed = par("speed");
        m_states[0] = 0;
        m_states[1] = 1;

        // if the initial speed is lower than 0, the node is stationary
        m_stationary = (m_speed == 0);

        // host moves the first time after some random delay to avoid synchronized movements
        if (!m_stationary)
        {
            cMessage* stMsg = new cMessage("stUp");
            stMsg->setKind(StateUpMessageKind);

            cMessage* movMsg = new cMessage("move");
            movMsg->setKind(MoveMessageKind);
            
            scheduleAt(simTime() + uniform(0, m_updateInterval), movMsg);
            scheduleAt(simTime() + uniform(0, m_stateTransInterval), stMsg);
        }
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * host is stationary this function is never called.
 */
void ChiangMobility::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        case MoveMessageKind:
            move();
            updatePosition();
            if (!m_stationary)
                scheduleAt(simTime() + m_updateInterval, msg);
            break;
        case StateUpMessageKind:
            recalculateState();
            if (!m_stationary)
                scheduleAt(simTime() + m_stateTransInterval, msg);            
            break;
        default:
            EV << " got self message of unknown kind, ignoring " << endl;
    }
}


static const double stateMatrix[3][3] = {
    {0.5, 0.0, 0.5},
    {0.7, 0.3, 0.0},
    {0.0, 0.3, 0.7}
    //states are {<prev location>, <current location>, <next location>}
};

//gets the next state based on the current state and a random value in [0,1]
int ChiangMobility::getNextStateIndex(int currentState, double rvalue)
{
    //we assume that the sum in each row is 1
    double sum = 0;
    for (int i = 0; i < 3; ++i)
    {        
        if (0 != stateMatrix[currentState][i])
        {
            sum += stateMatrix[currentState][i];
            if (sum >= rvalue)
                return i;
        }
    }
    EV << " getNextStateIndex error! cstate= " << currentState
       << " value= " << rvalue << endl;
    return currentState;
}

/**
 * Recalculate the state
 */
void ChiangMobility::recalculateState()
{
    for(int i = 0; i < 2; ++i)
    {
        m_states[i] = getNextStateIndex(m_states[i], uniform(0.0, 1.0));
    }
}

/**
 * Move the host
 */
void ChiangMobility::move()
{
    int dx, dy;    

    //calculate movement based on state
    dx = m_states[0] - 1;
    dy = m_states[1] - 1;

    if (0 != dx && 0 != dy)
    {
        //distribute speed evenly
        dx *= (m_speed * m_updateInterval) / sqrt(2);
        dy *= (m_speed * m_updateInterval) / sqrt(2);
    }
    else
    {
        //remember one of these is 0
        dx *= (m_speed * m_updateInterval);
        dy *= (m_speed * m_updateInterval);
    }

    //update position
    pos += Coord(dx, dy);
    
    // do something if we reach the wall
    {       
      Coord step = Coord(m_states[0] - 1, m_states[1] - 1);
      Coord target = pos;
      double angle = 0;
      handleIfOutside(REFLECT, target, step, angle);
      m_states[0] = (int)step.x + 1;
      m_states[1] = (int)step.y + 1;
      //EV << " step.x=" << step.x << " step.y= " << step.y << endl;
      //EV << " m_states[0]= " << m_states[0] << " m_states[1]= " << m_states[1] << endl;
    }

    EV << " xpos= " << pos.x << " ypos=" << pos.y << endl;
}
