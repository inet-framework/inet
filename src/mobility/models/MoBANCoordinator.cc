/* -*- mode:c++ -*- ********************************************************
 * file:        MoBANCoordinator.cc
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *              http://www.es.ele.tue.nl/nes
 *
 * copyright:   (C) 2010 Electronic Systems group(ES),
 *              Eindhoven University of Technology (TU/e), the Netherlands.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:    MoBAN (Mobility Model for wireless Body Area Networks)
 * description:     Implementation of the coordinator module of the MoBAN mobility model
 ***************************************************************************
 * Citation of the following publication is appreciated if you use MoBAN for
 * a publication of your own.
 *
 * M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
 * In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
 *
 * BibTeX:
 *        @inproceedings{MoBAN,
 *         author = "M. Nabi and M. Geilen and T. Basten.",
 *          title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
 *        booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
 *        series = {SIMUTools '11},
 *        isbn = {978-963-9799-41-7},
 *        year = {2011},
 *        location = {Barcelona, Spain},
 *        publisher = {ICST} }
 *
 **************************************************************************/

#include <string>
#include <stdio.h>
#include <assert.h>
#include <FWMath.h>

#include "MobilityAccess.h"
#include "MoBANCoordinator.h"
#include "MoBANLocal.h"

Define_Module(MoBANCoordinator);

void MoBANCoordinator::initialize(int stage) {
    LineSegmentsMobilityBase::initialize(stage);
    EV << "initializing MoBANCoordinator stage " << stage << endl;
    if (stage == 0) {
        //FIXME calls some functions on other modules, but initialization stage of other modules are not guaranteed.
        // The initialize(0) not called yet on some other modules.
        // Should be redesigned stages in MoBANCoordinator/MoBANLocal modules.
        useMobilityPattern = par("useMobilityPattern").boolValue();
        collectLocalModules(getParentModule());

        // preparing output mobility pattern log file
        char log_file_name[70];
        sprintf(log_file_name, "MoBAN_Pattern_out%d.txt", getIndex());
        logfile = fopen(log_file_name, "w");

        if (!readPostureSpecificationFile())
            error("MoBAN Coordinator: error in reading the posture specification file");

        if (!readConfigurationFile())
            error("MoBAN Coordinator: error in reading the input configuration file");

        if (useMobilityPattern)
            if (!readMobilityPatternFile())
                error("MoBAN Coordinator: error in reading the input mobility pattern file");

        publishToNodes();
    }
}

void MoBANCoordinator::initializePosition() {
    lastPosition = selectDestination();
}

/**
 * The main process of the MoBAN mobility model. To be called whenever a destination is reached or the duration of the previous
 * posture expires. It select the behavior of the next movement (the posture and the destination), and prepare the required
 * variables to make the movement.
 * In the case of using a logged mobility pattern, the new posture and other parameters are obtained from the pattern.
 */
void MoBANCoordinator::setTargetPosition() {
    // select the new posture and set the variable currentPosture as well as the reference points of all nodes within the WBAN.
    if (useMobilityPattern) {
        currentPattern = (currentPattern + 1) % patternLength;
        int postureID = mobilityPattern[currentPattern].postureID;
        currentPosture = postureList[postureID];
    } else
        selectPosture();

    EV << "New posture is selected: " << currentPosture->getPostureName() << endl;

    simtime_t duration;
    if (currentPosture->isMobile())
    {
        double distance;

        if (useMobilityPattern) {
            targetPosition = mobilityPattern[currentPattern].targetPos;
            speed = mobilityPattern[currentPattern].speed;
        }
        else {
            targetPosition = selectDestination();
            speed = selectSpeed();
        }

        if (speed==0)
            error("The velocity in a mobile posture should not be zero!");

        distance = lastPosition.distance(targetPosition);
        duration = distance / speed;
    }
    else
    {
        targetPosition = lastPosition;
        if (useMobilityPattern)
            duration = mobilityPattern[currentPattern].duration;
        else
            duration = selectDuration();
    }
    nextChange = simTime() + duration;

    //show posture name in the graphical interface
    if (ev.isGUI()){
        char dis_str[100];
        sprintf(dis_str,"%s until %f", currentPosture->getPostureName(), nextChange.dbl());
        getDisplayString().setTagArg("t", 0, dis_str);
    }

    // write the move step into the output log file
    if (currentPosture->isMobile())
        fprintf(logfile,"%s %d %f %f %f %f \n", currentPosture->getPostureName(), currentPosture->getPostureID(), targetPosition.x, targetPosition.y, targetPosition.z, speed);
    else
        fprintf(logfile,"%s %d %f \n", currentPosture->getPostureName(), currentPosture->getPostureID(), duration.dbl());

    publishToNodes();

    EV << "New posture: " << currentPosture->getPostureName() << endl;
    EV << "Destination: " << targetPosition.info() << " Total Time = " << duration << endl;
}

/**
 * Select a new posture randomly or based on the given Markov model.
 * If the requested strategy is not uniform random, A Markov chain will be used. If the strategy is INDIVIDUAL_MARKOV,
 * we should retrieve the transition matrix for the current part of the area.  If it is INDIVIDUAL_MARKOV,
 * we have the base transition matrix and a steady state vector for the current part of the area. So the
 * closest transition matrix to the base matrix is calculated which satisfies the required steady state vector.
 * In any case, the next posture is selected considering the current posture and according to the final
 * Markov transition matrix.
 */
void MoBANCoordinator::selectPosture() {
    int postureID;

    if (postureSelStrategy == UNIFORM_RANDOM) {
        postureID = floor(uniform(0, numPostures)); // uniformly random posture selection
        currentPosture = postureList[postureID];
        return;
    }

    /* Here we check the area and the time to determine the corresponding posture transition matrix */
    markovMatrix = transitions->getMatrix(simTime(), lastPosition);

    /* Using transition matrix to select the next posture */
    double randomValue = uniform(0, 1);
    double comp = 0;
    int currentP = currentPosture->getPostureID(); // it determines the column in the matrix

    for (int i = 0; i < static_cast<int>(numPostures); ++i)
    {
        comp += markovMatrix[i][currentP];
        if (randomValue < comp)
        {
            postureID = i;
            break;
        }
    }

    currentPosture = postureList[postureID];
}

/**
 * Select a stay time duration in the specified duration range for the new posture.
 * It is called whenever a new stable posture is selected.
 */
simtime_t MoBANCoordinator::selectDuration() {
    return uniform(minDuration, maxDuration);
}

/**
 * Select a position inside the simulation area as the destination for the new mobile posture.
 * It is called whenever a new mobile posture is selected.
 * It is taken into account that all nodes should be inside the area.
 */
Coord MoBANCoordinator::selectDestination() {
    Coord res;
    res = getRandomPosition();
    res.z = lastPosition.z; // the z value remain the same

    // check if it is okay using CoverRadius
    while (!isInsideWorld(res)) {
        res = getRandomPosition();
        res.z = lastPosition.z;
    }

    return res;
}

/**
 * Select a velocity value within the given velocity range.
 * It is called whenever a new stable posture is selected.
 * In the case of using a logged mobility pattern, the speed value is retrieved from the pattern.
 */
double MoBANCoordinator::selectSpeed() {
    return (uniform(currentPosture->getMinSpeed(), currentPosture->getMaxSpeed()));
}

/**
 * Checks if all nodes of the WBAN are inside the simulation environment with the current position.
 */
bool MoBANCoordinator::isInsideWorld(Coord tPos) {
    Coord absolutePosition;

    for (unsigned int i = 0; i < localModules.size(); ++i) {
        absolutePosition = tPos + currentPosture->getPs(i);
        if (!absolutePosition.isInBoundary(this->constraintAreaMin, this->constraintAreaMax))
            return false;
    }

    return true;
}

/**
 * Publishes the reference point and other information of the posture to the blackboard of the belonging nodes.
*/
void MoBANCoordinator::publishToNodes() {
    for (unsigned int i = 0; i < localModules.size(); ++i) {
        MoBANLocal *localModule = localModules[i];
        EV << "Publish data for: " << localModule->getParentModule()->getFullName() << endl;
        localModule->setMoBANParameters(currentPosture->getPs(i), currentPosture->getRadius(i), currentPosture->getSpeed(i));
    }
}

void MoBANCoordinator::finish() {
    fclose(logfile);
}

/**
 * This function reads the input mobility pattern file and make a list of the mobility patterns.
 * It will be called in the initialization phase if the useMobilityPattern parameter is true.
 */
bool MoBANCoordinator::readMobilityPatternFile() {
    patternLength = 0;
    double x, y, z, s;
    int id;
    char file_name[70];
    char posture_name[50];

    sprintf(file_name, "%s", par("mobilityPatternFile").stringValue());
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
        return false;

    // count number of patterns (lines in the input file)
    int c;
    while ((c = fgetc(fp)) != EOF)
        if (c == '\n')
            patternLength++;
    fclose(fp);

    EV << "Mobility Pattern Length: " << patternLength << endl;

    mobilityPattern = new Pattern[patternLength];

    fp = fopen(file_name,"r");

    int i=0;
    while (fscanf(fp,"%s %d",posture_name,&id)!= -1) {
        mobilityPattern[i].postureID = id;
        if (postureList[id]->isMobile())
        {
            assert(fscanf(fp,"%le %le %le %le",&x,&y,&z,&s)!=-1);
            mobilityPattern[i].targetPos = Coord(x,y,z);
            mobilityPattern[i].speed = s;
        }
        else
        {
            assert (fscanf(fp,"%le",&x)!=-1);
            mobilityPattern[i].duration = x;
        }
        ++i;
    }

    fclose(fp);

    currentPattern = -1;

    return true;
}

/**
 * Function to read the specified posture specification input file and make the posture data base. The posture specification includes the specification of
 * a set of possible body postures in the target application. The specification of each posture should provide the speed range
 * of the global movement of the whole WBAN and the relative position of the reference point, movement radius around the reference point
 * and movement velocity  of all nodes in the WBAN.
 * The function will be called in the initialization phase.
 */
bool MoBANCoordinator::readPostureSpecificationFile() {

    cXMLElement* xmlPosture = par("postureSpecFile").xmlValue();
    if (xmlPosture == 0)
        return false;

    const char* str;

    // read the specification of every posture from file and make a list of postures
    cXMLElementList postures;

    postures = xmlPosture->getElementsByTagName("posture");

    // find the number of defined postures
    numPostures = postures.size();
    if (numPostures == 0)
        error("No posture is defined in the input posture specification file");

    unsigned int postureID;

    cXMLElementList::const_iterator posture;
    for (posture = postures.begin(); posture != postures.end(); posture++)
    {
        str = (*posture)->getAttribute("postureID");
        postureID = strtol(str, 0, 0);
        if (postureID < 0 || postureID >= numPostures)
            error ("Posture ID in input posture specification file is out of the range");

        postureList.push_back(new Posture(postureID, localModules.size()));

        str = (*posture)->getAttribute("name");
        postureList[postureID]->setPostureName(const_cast<char*> (str));

        str = (*posture)->getAttribute("minSpeed");
        double minS = strtod(str, 0);
        str = (*posture)->getAttribute("maxSpeed");
        double maxS = strtod(str, 0);
        postureList[postureID]->setPostureSpeed(minS, maxS);

        int i = 0;
        double x, y, z, s, r;
        cXMLElementList nodeParameters;

        nodeParameters = (*posture)->getElementsByTagName("nodeParameters");
        if (nodeParameters.size() != localModules.size())
            error ("Some nodes may not have specified parameters in a posture in input posture specification file");

        cXMLElementList::const_iterator param;
        for (param = nodeParameters.begin(); param!= nodeParameters.end(); param++)
        {
            str = (*param)->getAttribute("positionX");
            x = strtod(str, 0);

            str = (*param)->getAttribute("positionY");
            y = strtod(str, 0);

            str = (*param)->getAttribute("positionZ");
            z = strtod(str, 0);

            str = (*param)->getAttribute("radius");
            r = strtod(str, 0);

            str = (*param)->getAttribute("speed");
            s = strtod(str, 0);

            postureList[postureID]->setPs(i, Coord(x, y, z));
            postureList[postureID]->setRadius(i, r);
            postureList[postureID]->setSpeed(i, s);

            i++;
        }
    }

    /* Report the obtained specification of the postures. */
    for (unsigned int i = 0; i < numPostures; ++i) {
        EV << "Information for the posture: " << i << " is" << endl;
        for (unsigned int j = 0; j < localModules.size(); ++j)
        EV << "Node " << j << " position: " << postureList[i]->getPs(j).info() <<
            " and radius: " << postureList[i]->getRadius(j) << " and speed: " << postureList[i]->getSpeed(j) << endl;
    }

    return true;

}

/**
 * Function to read the configuration file which includes the information for configuring and tuning the model for a specific
 * application scenario. The configuration file can provide the Markov model transition matrices, different area types and time domains,
 * and the proper matrix for each time-space combination. However, these are all optional and will be given just in the case that the
 * space-time correlation are required to be simulated.
 * The function will be called in the initialization phase.
 */
bool MoBANCoordinator::readConfigurationFile() {
    cXMLElement* xmlConfig = par("configFile").xmlValue();
    if (xmlConfig == 0)
        return false;

    cXMLElementList tagList;
    cXMLElement* tempTag;
    const char* str;
    std::string sstr; // for easier comparison

    /* Reading the initial posture if it is given*/
    int postureID;
    tagList = xmlConfig->getElementsByTagName("initialPosture");

    if (tagList.empty())
        postureID = 0; // no initial posture has been specified. The first one is selected!
    else {
        tempTag = tagList.front();
        str = tempTag->getAttribute("postureID");
        postureID = strtol(str, 0, 0);
    }
    currentPosture = postureList[postureID];
    EV << "Initial Posture: " << currentPosture->getPostureName() << endl;

    /* Reading the initial position if it is given */
    tagList = xmlConfig->getElementsByTagName("initialLocation");
    if (tagList.empty())
        lastPosition = Coord(10,10,5); // no initial location has been specified .
    else
    {
        double x,y,z;
        tempTag= tagList.front();

        str = tempTag->getAttribute("x"); x = strtod(str, 0);
        str = tempTag->getAttribute("y"); y = strtod(str, 0);
        str = tempTag->getAttribute("z"); z = strtod(str, 0);
        lastPosition = Coord(x,y,z);
    }
    EV << "Initial position of the LC: " << lastPosition.info() << endl;

    /* Reading the given range for duration of stable postures */
    tagList = xmlConfig->getElementsByTagName("durationRange");
    if (tagList.empty())
    {
        // no duration is specified. We assign a value!
        minDuration = 0;
        maxDuration = 100;
    }
    else
    {
        tempTag= tagList.front();

        str = tempTag->getAttribute("min");
        minDuration = strtod(str, 0);
        str = tempTag->getAttribute("max");
        maxDuration = strtod(str, 0);
    }
    EV << "Posture duration range: (" << minDuration << " , " << maxDuration << ")" << endl;

    transitions = new PostureTransition(numPostures);

    /* Reading the Markov transition matrices, if there are any. */
    tagList = xmlConfig->getElementsByTagName("markovMatrices");

    if (tagList.empty())
    {
        postureSelStrategy = UNIFORM_RANDOM; // no posture selection strategy is required. uniform random is applied
        EV << "Posture Selection strategy: UNIFORM_RANDOM " << endl;
        return true;
    }

    tempTag = tagList.front();

    cXMLElementList matrixList;
    matrixList = tempTag->getElementsByTagName("MarkovMatrix");

    if (tagList.empty())
    {
        postureSelStrategy = UNIFORM_RANDOM; // no posture selection strategy is required. uniform random is applied
        EV << "Posture Selection strategy: UNIFORM_RANDOM " << endl;
        return true;
    }

    postureSelStrategy = MARKOV_BASE;

    // make an empty matrix for the Markov Chain
    double** matrix = new double* [numPostures];
    for (unsigned int i=0;i<numPostures;++i)
        matrix[i] = new double [numPostures];

    bool setDefault=false; // variable to remember if the default matrix is defined.
    cXMLElementList::const_iterator matrixTag;
    for (matrixTag = matrixList.begin(); matrixTag != matrixList.end(); matrixTag++)
    {

        cXMLElementList rowList;
        cXMLElementList cellList;
        int i=0,j=0;
        bool thisDefault = false;

        if ((*matrixTag)->getAttribute("type") != NULL)
        {
            sstr = (*matrixTag)->getAttribute("type");
            if (sstr == "Default" || sstr == "default")
            {
                if (setDefault)
                    error ("There are more than one default matrix defined in the configuration file!");
                else
                    {setDefault = true; thisDefault = true;}
            }
        }


        sstr = (*matrixTag)->getAttribute("name");

        rowList = (*matrixTag)->getElementsByTagName("row");
        if (rowList.size()!= numPostures && rowList.size()!= 1)
            error("Number of rows in  the Markov transition matrix should be equal to either the number"
                    " of postures (full Markov matrix) or one (steady state vector)");

        if (rowList.size()!= numPostures && thisDefault)
            error("Dimension of the default Markov matrix should be equal to the number of postures in the configuration file");

        if ((rowList.size()== 1) && (!setDefault))
            error("A default matrix is supposed to be defined before a steady state can be defined in the configuration file");

        for (cXMLElementList::const_iterator row = rowList.begin(); row != rowList.end(); row++)
        {
            cellList = (*row)->getElementsByTagName("cell");
            if (cellList.size()!= numPostures)
                error("Number of columns in  the Markov transition matrix should be equal to the number of postures");

            j=0;
            for (cXMLElementList::const_iterator cell = cellList.begin(); cell != cellList.end(); cell++)
            {
                str = (*cell)->getAttribute("value");
                matrix[i][j] = strtod(str, 0);
                j++;
            }

            ++i;
        }

        if (rowList.size() == 1)
            transitions->addSteadyState(sstr, matrix[0]); // steady state
        else
            transitions->addMatrix(sstr, matrix, thisDefault);         // A full Markovian matrix

        EV << "Markov transition matrix " << sstr << " : " << endl;
        for (int k=0;k < i ; ++k)
        {
            for (unsigned int f=0; f<numPostures ;++f)
                EV << matrix[k][f] << " ";
            EV << endl;
        }

    }

    /* Reading the Area types, if there are any. */
    tagList = xmlConfig->getElementsByTagName("areaTypes");

    if (tagList.empty())
        EV << "No area type is given. So there is no spatial correlation in posture selection." << endl;
    else {

            tempTag = tagList.front();
            cXMLElementList typeList = tempTag->getElementsByTagName("areaType");

            if (typeList.empty())
                error ("No areaType has been defined in areaTypes!");

            for (cXMLElementList::const_iterator aType = typeList.begin(); aType != typeList.end(); aType++)
            {
                sstr = (*aType)->getAttribute("name");

                EV << "Area type " << sstr << " : " << endl;

                int typeID = transitions->addAreaType(sstr);

                cXMLElementList boundList = (*aType)->getElementsByTagName("boundary");
                if (boundList.empty())
                    error ("No boundary is given for a area type!");

                Coord minBound, maxBound;
                for (cXMLElementList::const_iterator aBound = boundList.begin(); aBound != boundList.end(); aBound++)
                {
                    str = (*aBound)->getAttribute("xMin"); minBound.x = strtod(str, 0);
                    str = (*aBound)->getAttribute("yMin"); minBound.y = strtod(str, 0);
                    str = (*aBound)->getAttribute("zMin"); minBound.z = strtod(str, 0);

                    str = (*aBound)->getAttribute("xMax"); maxBound.x = strtod(str, 0);
                    str = (*aBound)->getAttribute("yMax"); maxBound.y = strtod(str, 0);
                    str = (*aBound)->getAttribute("zMax"); maxBound.z = strtod(str, 0);

                    transitions->setAreaBoundry(typeID,minBound,maxBound);
                    EV << "Low bound: " << minBound.info() << endl;
                    EV << "High bound: " << maxBound.info() << endl;
                }
            }
        }

    /* Reading the time domains, if there are any. */
    tagList = xmlConfig->getElementsByTagName("timeDomains");

    if (tagList.empty())
        EV << "No time domain is given. So there is no temporal correlation in posture selection." << endl;
    else {

            tempTag = tagList.front();
            cXMLElementList typeList = tempTag->getElementsByTagName("timeDomain");

            if (typeList.empty())
                error ("No timeDomain has been defined in timeDomains!");

            for (cXMLElementList::const_iterator aType = typeList.begin(); aType != typeList.end(); aType++)
            {
                sstr = (*aType)->getAttribute("name");

                EV << "Time domain " << sstr << " : " << endl;

                int typeID = transitions->addTimeDomain(sstr);

                cXMLElementList boundList = (*aType)->getElementsByTagName("boundary");
                if (boundList.empty())
                    error ("No boundary is given for a time domain!");

                simtime_t minTime,maxTime;
                for (cXMLElementList::const_iterator aBound = boundList.begin(); aBound != boundList.end(); aBound++)
                {
                    str = (*aBound)->getAttribute("tMin"); minTime = strtod(str,0);
                    str = (*aBound)->getAttribute("tMax"); maxTime = strtod(str,0);

                    transitions->setTimeBoundry(typeID,minTime,maxTime);
                    EV << "Low bound: (" << minTime.dbl() << ", " << maxTime << ")" << endl;
                }
            }
    }

    /* Reading the combinations, if there are any. */
    tagList = xmlConfig->getElementsByTagName("combinations");

    if (tagList.empty())
        EV << "No combination is given. The default Markov model is then used for the whole time and space!" << endl;
    else {
            tempTag = tagList.front();
            cXMLElementList combList = tempTag->getElementsByTagName("combination");

            if (combList.empty())
                error ("No combination has been defined in combinations!");

            EV << "Combinations: " << endl;

            for (cXMLElementList::const_iterator aComb = combList.begin(); aComb != combList.end(); aComb++)
            {
                std::string areaName,timeName,matrixName;

                if ((*aComb)->getAttribute("areaType") != NULL)
                    areaName = (*aComb)->getAttribute("areaType");
                else
                    areaName.clear();

                if ((*aComb)->getAttribute("timeDomain") != NULL)
                    timeName = (*aComb)->getAttribute("timeDomain");
                else
                    timeName.clear();

                if ((*aComb)->getAttribute("matrix") != NULL)
                    matrixName = (*aComb)->getAttribute("matrix");
                else
                    error("No transition matrix is specified for a combination");

                transitions->addCombination(areaName, timeName, matrixName);

                EV << "(" << areaName << ", " << timeName << ", " << matrixName << ")" << endl;
            }
        }

    return true;
}

void MoBANCoordinator::collectLocalModules(cModule *module)
{
    for (cModule::SubmoduleIterator it(module); !it.end(); it++)
    {
        cModule *submodule = it();
        collectLocalModules(submodule);
        MoBANLocal *localModule = dynamic_cast<MoBANLocal *>(submodule);
        if (localModule && localModule->par("coordinatorIndex").longValue() == getIndex()) {
            localModule->setCoordinator(this);
            localModules.push_back(localModule);
        }
    }
}
