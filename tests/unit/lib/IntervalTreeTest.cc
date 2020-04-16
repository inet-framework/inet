#include "IntervalTreeTest.h"

namespace inet {

void IntervalTreeTest::insertNodes(int nodeCount)
{
    for (int i = 0; i < nodeCount; i++) {
        double low = (double) rand() / RAND_MAX;
        double high = low + (double) rand() / RAND_MAX;
        char *c = new char[16];
        sprintf(c, "%d", i);
        IntervalTree::Interval *interval = new IntervalTree::Interval(low, high, c);
        intervals.push_back(interval);
        tree.insert(interval);
    }
}

void IntervalTreeTest::deleteNodes()
{
    for (auto interval : intervals) {
        if ((double) rand() / RAND_MAX < 0.5) {
            tree.deleteNode(interval);
        }
    }
}

void IntervalTreeTest::checkTree()
{
    checkNil();
    checkNode(tree.root->left);
    checkNode(tree.root->right);
}

void IntervalTreeTest::checkNil()
{
    IntervalTree::Node *nil = tree.nil;
    if (nil->left != nil || nil->right != nil)
        throw std::runtime_error("Broken: nil left or right");
//    if (nil->parent != nil)
//        throw std::runtime_error("Broken: nil parent");
    if (nil->red || nil->stored_interval)
        throw std::runtime_error("Broken: nil color");
    const auto nts = -SimTime::getMaxTime();
    if (nil->key != nts || nil->high != nts || nil->max_high != nts)
        throw std::runtime_error("Broken: nil key");
}

int IntervalTreeTest::checkNode(IntervalTree::Node *node)
{
    int ld = 0, rd = 0; // black depths on each subtree
    if (node->left != tree.nil) {
        ld = checkNode(node->left);
        if (node->left->parent != node)
            throw std::runtime_error("Broken: node left parent");
        if (node->red && node->left->red)
            throw std::runtime_error("Broken: node left color");
        if (node->left->key > node->key)
            throw std::runtime_error("Broken: node left key");
    }
    if (node->right != tree.nil) {
        rd = checkNode(node->right);
        if (node->right->parent != node)
            throw std::runtime_error("Broken: node right parent");
        if (node->red && node->right->red)
            throw std::runtime_error("Broken: node right color");
        if (node->key > node->right->key)
            throw std::runtime_error("Broken: node right key");
    }
    if (ld != rd)
        throw std::runtime_error("Broken: node depth");
    if (!node->red)
        ++ld;
    return ld;
}

void IntervalTreeTest::run(int nodeCount)
{
    std::cout << "Testing IntervalTree with " << nodeCount << " nodes\n";
    insertNodes(nodeCount);
    deleteNodes();
    checkTree();
}

} // namespace inet

