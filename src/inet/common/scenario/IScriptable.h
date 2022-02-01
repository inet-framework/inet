//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISCRIPTABLE_H
#define __INET_ISCRIPTABLE_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Modules that need to be scriptable by ScenarioManager should "implement"
 * (subclass from) this class.
 *
 * @see ScenarioManager
 */
class INET_API IScriptable
{
  public:
    virtual ~IScriptable() {}

    /**
     * Called by ScenarioManager whenever a script command needs to be
     * carried out by the module.
     *
     * The command is represented by the XML element or element tree.
     * The command name can be obtained as:
     *
     * <pre>
     * const char *command = node->getTagName()
     * </pre>
     *
     * Parameters are XML attributes, e.g. a "neighbour" parameter can be
     * retrieved as:
     *
     * <pre>
     * const char *attr = node->getAttribute("neighbour")
     * </pre>
     *
     * More complex input can be passed in child elements.
     *
     * @see cXMLElement
     */
    virtual void processCommand(const cXMLElement& node) = 0;
};

} // namespace inet

#endif

