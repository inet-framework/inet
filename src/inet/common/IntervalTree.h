/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef __INET_INTERVALTREE_H
#define __INET_INTERVALTREE_H

#include <deque>
#include <limits>
#include "inet/common/INETDefs.h"

namespace inet {

namespace physicallayer {

/// @brief Interval trees implemented using red-black-trees as described in
/// the book Introduction_To_Algorithms_ by Cormen, Leisserson, and Rivest.
/// Can be replaced in part by boost::icl::interval_set, which is only supported after boost 1.46 and does not support delete node routine.
struct Interval
{
  public:
    Interval(simtime_t low, simtime_t high, void *value) :
        low(low), high(high), value(value) {}

    virtual ~Interval()
    {
    }

    virtual void print() const
    {
    }

    /// @brief interval is defined as [low, high]
    simtime_t low, high;
    void *value;
};

/// @brief The node for interval tree
class IntervalTreeNode
{
    friend class IntervalTree;

  public:
    /// @brief Print the interval node information: set left = nil and right = root
    void print(IntervalTreeNode* left, IntervalTreeNode* right) const;

    /// @brief Create an empty node
    IntervalTreeNode();

    /// @brief Create an node storing the interval
    IntervalTreeNode(const Interval* new_interval);

    ~IntervalTreeNode();

  protected:
    /// @brief interval stored in the node
    const Interval* stored_interval = nullptr;

    simtime_t key;

    simtime_t high;

    simtime_t max_high;

    /// @brief red or black node: if red = false then the node is black
    bool red = false;

    IntervalTreeNode* left = nullptr;

    IntervalTreeNode* right = nullptr;

    IntervalTreeNode* parent = nullptr;
};

/// @brief Class describes the information needed when we take the
/// right branch in searching for intervals but possibly come back
/// and check the left branch as well.
struct it_recursion_node
{
  public:
    IntervalTreeNode* start_node = nullptr;

    unsigned int parent_index = 0;

    bool try_right_branch = false;
};

/// @brief Interval tree
class IntervalTree
{
  public:
    IntervalTree();
    ~IntervalTree();

    /// @brief Print the whole interval tree
    void print() const;

    /// @brief Delete one node of the interval tree
    const Interval* deleteNode(IntervalTreeNode* node);

    /// @brief delete node stored a given interval
    void deleteNode(const Interval* ivl);

    /// @brief Insert one node of the interval tree
    IntervalTreeNode* insert(const Interval* new_interval);

    /// @brief get the predecessor of a given node
    IntervalTreeNode* getPredecessor(IntervalTreeNode* node) const;

    /// @brief Get the successor of a given node
    IntervalTreeNode* getSuccessor(IntervalTreeNode* node) const;

    /// @brief Return result for a given query
    std::deque<const Interval*> query(simtime_t low, simtime_t high);

  protected:

    IntervalTreeNode* root = nullptr;

    IntervalTreeNode* nil = nullptr;

    /// @brief left rotation of tree node
    void leftRotate(IntervalTreeNode* node);

    /// @brief right rotation of tree node
    void rightRotate(IntervalTreeNode* node);

    /// @brief recursively insert a node
    void recursiveInsert(IntervalTreeNode* node);

    /// @brief recursively print a subtree
    void recursivePrint(IntervalTreeNode* node) const;

    /// @brief recursively find the node corresponding to the interval
    IntervalTreeNode* recursiveSearch(IntervalTreeNode* node, const Interval* ivl) const;

    /// @brief Travels up to the root fixing the max_high fields after an insertion or deletion
    void fixupMaxHigh(IntervalTreeNode* node);

    void deleteFixup(IntervalTreeNode* node);

  private:
    unsigned int recursion_node_stack_size = 0;
    it_recursion_node* recursion_node_stack = nullptr;
    unsigned int current_parent = 0;
    unsigned int recursion_node_stack_top = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_INTERVAL_TREE_H

