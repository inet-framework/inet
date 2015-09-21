#ifndef __INET_INTERVALTREETEST_H
#define __INET_INTERVALTREETEST_H

#include "inet/common/IntervalTree.h"

namespace inet {

class IntervalTreeTest
{
  protected:
    std::vector<Interval *> intervals;
    IntervalTree tree;

  protected:
    void insertNodes(int nodeCount);
    void deleteNodes();

    void checkTree();
    void checkNil();
    int checkNode(IntervalTreeNode* n);

  public:
    void run(int nodeCount);
};

} // namespace inet

#endif // ifndef __INET_INTERVALTREETEST_H

