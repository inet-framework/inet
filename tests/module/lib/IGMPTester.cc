
#include <fstream>

#include "INETDefs.h"
#include "IScriptable.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"

class INET_API IGMPTester : public cSimpleModule, public IScriptable
{
  private:
     std::ofstream out;
  protected:
    virtual void initialize();
    virtual void processCommand(const cXMLElement &node);
  public:
    typedef IPv4InterfaceData::IPv4AddressVector IPv4AddressVector;
    IGMPTester() {}
  private:
    void checkMulticastGroups(const char* command, const char* node, const char *ifname, const char *expected, const IPv4AddressVector &groups);
};

Define_Module(IGMPTester);

void IGMPTester::initialize()
{
    const char *filename = par("outputFile");
    if (!filename || !(*filename))
        filename = "output.txt";

    out.open(filename);
    if (out.fail())
        throw cRuntimeError("Failed to open output file: %s", filename);
}

void IGMPTester::processCommand(const cXMLElement &node)
{
  const char *tag = node.getTagName();
  const char *nodeName = node.getAttribute("node");
  const char *ifname = node.getAttribute("ifname");
  cModule *module = simulation.getModuleByPath(nodeName);
  IInterfaceTable *ift = InterfaceTableAccess().get(module);
  InterfaceEntry *ie = ift->getInterfaceByName(ifname);

  if (!strcmp(tag, "join"))
  {
    const char *group = node.getAttribute("group");
    ie->ipv4Data()->joinMulticastGroup(IPv4Address(group));
  }
  else if (!strcmp(tag, "leave"))
  {
    const char *group = node.getAttribute("group");
    ie->ipv4Data()->leaveMulticastGroup(IPv4Address(group));
  }
  else if (!strcmp(tag, "check"))
  {
    const char *groups = node.getAttribute("groups");
    const char *listeners = node.getAttribute("listeners");
    if (groups)
    {
        const IPv4AddressVector &joinedGroups = ie->ipv4Data()->getJoinedMulticastGroups();
        checkMulticastGroups("multicast groups", nodeName, ifname, groups, joinedGroups);
    }
    else if (listeners)
    {
        const IPv4AddressVector &reportedGroups = ie->ipv4Data()->getReportedMulticastGroups();
        checkMulticastGroups("multicast listeners", nodeName, ifname, listeners, reportedGroups);
    }
  }
}

void IGMPTester::checkMulticastGroups(const char* desc, const char *module, const char *ifname, const char *expected, const IPv4AddressVector &groups)
{
  out << "At t=" << simTime() << " " << module << "/" << ifname << " " << desc << ": ";

  std::set<IPv4Address> expectedGroups;
  cStringTokenizer tokenizer(expected);
  const char *token;
  while ((token = tokenizer.nextToken()) != NULL)
    expectedGroups.insert(IPv4Address(token));

  std::set<IPv4Address> foundGroups(groups.begin(), groups.end());

  if (foundGroups == expectedGroups)
      out << "PASS\n";
  else
  {
    out << "FAIL ";
    out << "expected:";
    for (std::set<IPv4Address>::iterator it = expectedGroups.begin(); it != expectedGroups.end(); ++it)
        out << " " << *it;
    out << " found:";
    for (std::set<IPv4Address>::iterator it = foundGroups.begin(); it != foundGroups.end(); ++it)
        out << " " << *it;
    out << "\n";
  }
}
