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

#include "GaussMarkovMobility.h"

Define_Module(GaussMarkovMobility);

void GaussMarkovMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing GaussMarkovMobility stage " << stage << endl;

    if (stage == 0)
    {
        m_updateInterval = par("updateInterval");
        m_speedMean = par("speed");
        m_angleMean = par("angle");
        m_alpha = par("alpha");
        m_margin = par("margin");
        m_variance = par("variance");
        m_angle = fmod(m_angle,360);
        //constrain m_alpha to [0.0;1.0]
        m_alpha = fmax(0.0, m_alpha);
        m_alpha = fmin(1.0, m_alpha);
        
        m_speed = m_speedMean;
        m_angle = m_angleMean;

        // if the initial speed is lower than 0, the node is stationary
        m_stationary = (m_speed == 0);

        // host moves the first time after some random delay to avoid synchronized movements
        if (!m_stationary)
            scheduleAt(simTime() + uniform(0, m_updateInterval), new cMessage("move"));
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * host is stationary this function is never called.
 */
void GaussMarkovMobility::handleSelfMsg(cMessage * msg)
{
    preventBorderHugging();
    move();
    updatePosition();
    if (!m_stationary)
        scheduleAt(simTime() + m_updateInterval, msg);
}

void GaussMarkovMobility::preventBorderHugging()
{
  bool left = (pos.x < m_margin);
  bool right = (pos.x >= (getPlaygroundSizeX() - m_margin));
  bool top = (pos.y < m_margin);
  bool bottom = (pos.y >= (getPlaygroundSizeY() - m_margin));

  double newMean = -1;

  if (top || bottom)
  {
      newMean = ((bottom) ? 270.0 : 90.0);
      if (right)
          newMean -= 45.0;
      else if (left)
          newMean += 45.0;
  }  
  else if (left)
  {
      newMean = 0.0;      
  }
  else if (right)
  {
      newMean = 180.0;
  }
      
  if (newMean >= 0)
  {
      m_angleMean = newMean;
  }
}

/**
 * Move the host
 */
void GaussMarkovMobility::move()
{
    pos.x += m_speed * cos(PI * m_angle / 180) * m_updateInterval;
    pos.y += m_speed * sin(PI * m_angle / 180) * m_updateInterval;

    // do something if we reach the wall
    Coord dummy;
    handleIfOutside(REFLECT, dummy, dummy, m_angle);
    
    // calculate new speed and direction based on the model
    m_speed = m_alpha * m_speed +
                (1.0 - m_alpha) * m_speedMean +
                sqrt(1.0 - m_alpha * m_alpha)
                  * normal(0.0, 1.0)
                  * m_variance;

    m_angle = m_alpha * m_angle +
                (1.0 - m_alpha) * m_angleMean +
                sqrt(1.0 - m_alpha * m_alpha)
                  * normal(0.0, 1.0)
                  * m_variance;

    EV << " speed= " << m_speed << " angle= " << m_angle << endl;
    EV << " mspeed= " << m_speedMean << " mangle " << m_angleMean << endl;
    EV << " xpos= " << pos.x << " ypos=" << pos.y << endl;
}
