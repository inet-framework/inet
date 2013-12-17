//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "TurtleMobility.h"
#include "FWMath.h"


Define_Module(TurtleMobility);


TurtleMobility::TurtleMobility()
{
    turtleScript = NULL;
    nextStatement = NULL;
    speed = 0;
    angle = 0;
}

void TurtleMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);
    EV << "initializing TurtleMobility stage " << stage << endl;
    if (stage == 0)
    {
        WATCH(speed);
        WATCH(angle);
        WATCH(borderPolicy);
    }
}

void TurtleMobility::initializePosition()
{
    LineSegmentsMobilityBase::initializePosition();

    turtleScript = par("turtleScript");
    nextStatement = turtleScript->getFirstChild();

    speed = 1;
    angle = 0;
    borderPolicy = REFLECT;

    // a dirty trick to extract starting position out of the script
    // (start doing it, but then rewind to the beginning)
    nextChange = simTime();
    resumeScript();
    targetPosition = lastPosition;
    nextChange = simTime();
    nextStatement = turtleScript->getFirstChild();

    while (!loopVars.empty())
        loopVars.pop();
}

void TurtleMobility::setTargetPosition()
{
    resumeScript();
}

void TurtleMobility::move()
{
    LineSegmentsMobilityBase::move();
    Coord dummy;
    handleIfOutside(borderPolicy, targetPosition, dummy, angle);
}

/**
 * Will set a new nextChange and targetPosition.
 */
void TurtleMobility::resumeScript()
{
    simtime_t now = simTime();

    while (nextChange == now)
    {
        if (nextStatement != NULL)
        {
            executeStatement(nextStatement);
            gotoNextStatement();
        }
        else
        {
            nextChange = -1;
            stationary = true;
            targetPosition = lastPosition;
        }
    }
}

void TurtleMobility::executeStatement(cXMLElement *stmt)
{
    ASSERT(nextChange != -1);
    const char *tag = stmt->getTagName();

    EV << "doing <" << tag << ">\n";

    if (!strcmp(tag, "repeat"))
    {
        const char *nAttr = stmt->getAttribute("n");
        long n = -1;  // infinity -- that's the default

        if (nAttr)
        {
            n = (long) getValue(nAttr);

            if (n < 0)
                throw cRuntimeError("<repeat>: negative repeat count at %s", stmt->getSourceLocation());
        }

        loopVars.push(n);
    }
    else if (!strcmp(tag, "set"))
    {
        const char *speedAttr = stmt->getAttribute("speed");
        const char *angleAttr = stmt->getAttribute("angle");
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *bpAttr = stmt->getAttribute("borderPolicy");

        if (speedAttr)
            speed = getValue(speedAttr);

        if (angleAttr)
            angle = getValue(angleAttr);

        if (xAttr)
            targetPosition.x = lastPosition.x = getValue(xAttr);

        if (yAttr)
            targetPosition.y = lastPosition.y = getValue(yAttr);

        if (speed <= 0)
            throw cRuntimeError("<set>: speed is negative or zero at %s", stmt->getSourceLocation());

        if (bpAttr)
        {
            if (!strcmp(bpAttr, "reflect"))
                borderPolicy = REFLECT;
            else if (!strcmp(bpAttr, "wrap"))
                borderPolicy = WRAP;
            else if (!strcmp(bpAttr, "placerandomly"))
                borderPolicy = PLACERANDOMLY;
            else if (!strcmp(bpAttr, "error"))
                borderPolicy = RAISEERROR;
            else
                throw cRuntimeError("<set>: value for attribute borderPolicy is invalid, should be "
                      "'reflect', 'wrap', 'placerandomly' or 'error' at %s",
                      stmt->getSourceLocation());
        }
    }
    else if (!strcmp(tag, "forward"))
    {
        const char *dAttr = stmt->getAttribute("d");
        const char *tAttr = stmt->getAttribute("t");

        if (!dAttr && !tAttr)
            throw cRuntimeError("<forward>: must have at least attribute 't' or 'd' (or both) at %s", stmt->getSourceLocation());

        double d, t;

        if (tAttr && dAttr)
        {
            // cover distance d in time t (current speed is ignored)
            d = getValue(dAttr);
            t = getValue(tAttr);
        }
        else if (dAttr)
        {
            // travel distance d at current speed
            d = getValue(dAttr);
            t = d / speed;
        }
        else // tAttr only
        {
            // travel for time t at current speed
            t = getValue(tAttr);
            d = speed * t;
        }

        if (t < 0)
            throw cRuntimeError("<forward>: time (attribute t) is negative at %s", stmt->getSourceLocation());

        if (d < 0)
            throw cRuntimeError("<forward>: distance (attribute d) is negative at %s", stmt->getSourceLocation());

        // FIXME handle zeros properly...
        targetPosition.x += d * cos(PI * angle / 180);
        targetPosition.y += d * sin(PI * angle / 180);
        nextChange += t;
    }
    else if (!strcmp(tag, "turn"))
    {
        const char *angleAttr = stmt->getAttribute("angle");

        if (!angleAttr)
            throw cRuntimeError("<turn>: required attribute 'angle' missing at %s", stmt->getSourceLocation());

        angle += getValue(angleAttr);
    }
    else if (!strcmp(tag, "wait"))
    {
        const char *tAttr = stmt->getAttribute("t");

        if (!tAttr)
            throw cRuntimeError("<wait>: required attribute 't' missing at %s", stmt->getSourceLocation());

        double t = getValue(tAttr);

        if (t < 0)
            throw cRuntimeError("<wait>: time (attribute t) is negative (%g) at %s", t, stmt->getSourceLocation());

        nextChange += t;  // targetPosition is unchanged
    }
    else if (!strcmp(tag, "moveto"))
    {
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *tAttr = stmt->getAttribute("t");

        if (xAttr)
            targetPosition.x = getValue(xAttr);

        if (yAttr)
            targetPosition.y = getValue(yAttr);

        // travel to targetPosition at current speed, or get there in time t (ignoring current speed then)
        double t = tAttr ? getValue(tAttr) : lastPosition.distance(targetPosition) / speed;

        if (t < 0)
            throw cRuntimeError("<wait>: time (attribute t) is negative at %s",
                    stmt->getSourceLocation());

        nextChange += t;
    }
    else if (!strcmp(tag, "moveby"))
    {
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *tAttr = stmt->getAttribute("t");

        if (xAttr)
            targetPosition.x += getValue(xAttr);

        if (yAttr)
            targetPosition.y += getValue(yAttr);

        // travel to targetPosition at current speed, or get there in time t (ignoring current speed then)
        double t = tAttr ? getValue(tAttr) : lastPosition.distance(targetPosition) / speed;

        if (t < 0)
            throw cRuntimeError("<wait>: time (attribute t) is negative at %s",
                    stmt->getSourceLocation());

        nextChange += t;
    }
}

double TurtleMobility::getValue(const char *s)
{
    // first, textually replace $MAXX and $MAXY with their actual values
    std::string str;
    if (strchr(s, '$'))
    {
        char strMinX[32], strMinY[32];
        char strMaxX[32], strMaxY[32];
        sprintf(strMinX, "%g", constraintAreaMin.x);
        sprintf(strMinY, "%g", constraintAreaMin.y);
        sprintf(strMaxX, "%g", constraintAreaMax.x);
        sprintf(strMaxY, "%g", constraintAreaMax.y);

        str = s;
        std::string::size_type pos;

        while ((pos = str.find("$MINX")) != std::string::npos)
            str.replace(pos, sizeof("$MINX")-1, strMinX);

        while ((pos = str.find("$MINY")) != std::string::npos)
            str.replace(pos, sizeof("$MINY")-1, strMinY);

        while ((pos = str.find("$MAXX")) != std::string::npos)
            str.replace(pos, sizeof("$MAXX")-1, strMaxX);

        while ((pos = str.find("$MAXY")) != std::string::npos)
            str.replace(pos, sizeof("$MAXY")-1, strMaxY);

        s = str.c_str();
    }

    // then use cDynamicExpression to evaluate the string
    try {
        cDynamicExpression expr;
        expr.parse(s);
        return expr.doubleValue(this);
    }
    catch (std::exception& e) {
        throw cRuntimeError("Wrong value '%s' around %s: %s", s,
                nextStatement->getSourceLocation(), e.what());
    }
}

void TurtleMobility::gotoNextStatement()
{
    // "statement either doesn't have a child, or it's a <repeat> and loop count is already pushed on the stack"
    ASSERT(!nextStatement->getFirstChild() || (!strcmp(nextStatement->getTagName(), "repeat")
            && !loopVars.empty()));

    if (nextStatement->getFirstChild() && (loopVars.top() != 0 || (loopVars.pop(), false)))   // !=0: positive or -1
    {
        // statement must be a <repeat> if it has children; repeat count>0 must be
        // on the stack; let's start doing the body.
        nextStatement = nextStatement->getFirstChild();
    }
    else if (!nextStatement->getNextSibling())
    {
        // no sibling -- either end of <repeat> body, or end of script
        ASSERT(nextStatement->getParentNode()==turtleScript ? loopVars.empty() : !loopVars.empty());

        if (!loopVars.empty())
        {
            // decrement and check loop counter
            if (loopVars.top() != -1)  // -1 means infinity
                loopVars.top()--;

            if (loopVars.top() != 0)  // positive or -1
            {
                // go to beginning of <repeat> block again
                nextStatement = nextStatement->getParentNode()->getFirstChild();
            }
            else
            {
                // end of loop -- locate next statement after the <repeat>
                nextStatement = nextStatement->getParentNode();
                gotoNextStatement();
            }
        }
        else
        {
            // end of script
            nextStatement = NULL;
        }
    }
    else
    {
        // go to next statement (must exist -- see "if" above)
        nextStatement = nextStatement->getNextSibling();
    }
}
