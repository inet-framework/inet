//
// AnnotationManager - manages annotations on the OMNeT++ canvas
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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


#ifndef WORLD_ANNOTATION_ANNOTATIONCONTROL_H
#define WORLD_ANNOTATION_ANNOTATIONCONTROL_H

#include <list>

#include "INETDefs.h"

#include "ModuleAccess.h"
#include "Coord.h"

/**
 * manages annotations on the OMNeT++ canvas.
 */
class INET_API AnnotationManager : public cSimpleModule
{
  public:
    class Group;

    class Annotation
    {
      public:
        Annotation() : group(0) {}
        virtual ~Annotation() {}

      protected:
        friend class AnnotationManager;

        Group* group;
        mutable std::list<cModule*> dummyObjects;
    };

    class Line : public Annotation
    {
      public:
        Line(Coord p1, Coord p2, std::string color) : p1(p1), p2(p2), color(color) {}
        virtual ~Line() {}

      protected:
        friend class AnnotationManager;

        Coord p1;
        Coord p2;
        std::string color;
    };

    class Polygon : public Annotation
    {
      public:
        Polygon(std::list<Coord> coords, std::string color) : coords(coords), color(color) {}
        virtual ~Polygon() {}

      protected:
        friend class AnnotationManager;

        std::list<Coord> coords;
        std::string color;
    };

    class Group
    {
      public:
        Group(std::string title) : title(title) {}
        virtual ~Group() {}

      protected:
        friend class AnnotationManager;

        std::string title;
    };

    ~AnnotationManager();
    void initialize();
    void finish();
    void handleMessage(cMessage *msg);
    void handleSelfMsg(cMessage *msg);
    void handleParameterChange(const char *parname);

    void addFromXml(cXMLElement* xml);
    Group* createGroup(std::string title = "untitled");
    Line* drawLine(Coord p1, Coord p2, std::string color, Group* group = 0);
    Polygon* drawPolygon(std::list<Coord> coords, std::string color, Group* group = 0);
    Polygon* drawPolygon(std::vector<Coord> coords, std::string color, Group* group = 0);
    void drawBubble(Coord p1, std::string text);
    void erase(const Annotation* annotation);

    cModule* createDummyModule(std::string displayString);
    cModule* createDummyModuleLine(Coord p1, Coord p2, std::string color);

    void show(const Annotation* annotation);
    void hide(const Annotation* annotation);
    void showAll(Group* group = 0);
    void hideAll(Group* group = 0);

  protected:
    typedef std::list<Annotation*> Annotations;
    typedef std::list<Group*> Groups;

    bool debug; /**< whether to emit debug messages */
    cXMLElement* annotationsXml; /**< annotations to add at startup */

    Annotations annotations;
    Groups groups;
};

class AnnotationManagerAccess
{
  public:
    AnnotationManagerAccess()
    {
    }

    AnnotationManager* getIfExists()
    {
        return dynamic_cast<AnnotationManager*>(simulation.getModuleByPath("annotations"));
    }
};

#endif

