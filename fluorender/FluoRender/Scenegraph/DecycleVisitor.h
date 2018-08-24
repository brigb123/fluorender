/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef _DECYCLEVISITOR_H_
#define _DECYCLEVISITOR_H_

#include <Scenegraph/NodeVisitor.h>
#include <Scenegraph/Group.h>

namespace FL
{
	class DecycleVisitor : public NodeVisitor
	{
	public:
		DecycleVisitor() :
			found_(false),
			parent_(0),
			child_(0)
		{
			setTraversalMode(FL::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		virtual void apply(FL::Node& node)
		{
			if (!found_)
				decycle();
			traverse(node);
		}

		virtual void apply(FL::Group& group)
		{
			if (!found_)
				decycle();
			traverse(group);
		}

		virtual void reset()
		{
			found_ = false;
			parent_ = 0;
			child_ = 0;
		}

		bool removeCycle()
		{
			if (found_)
			{
				//remove cycle
				Group* group = dynamic_cast<Group*>(parent_);
				if (group && child_)
					group->removeChild(child_);
				found_ = false;
				parent_ = 0;
				child_ = 0;
				return true;
			}
			else
				return false;
		}

	private:
		bool found_;
		Node* parent_;
		Node* child_;

		void decycle()
		{
			//check if node already in path
			NodeSet node_set;
			std::pair<NodeSetIter, bool> result;
			for (auto it = m_node_path.begin();
				it != m_node_path.end(); ++it)
			{
				result = node_set.insert(*it);
				if (!result.second)
				{
					found_ = true;
					auto it2 = std::prev(it);
					if (m_traversal_mode == TRAVERSE_PARENTS)
					{
						//child: --it; parent: it
						parent_ = *it;
						child_ = *it2;
					}
					else if (m_traversal_mode != TRAVERSE_NONE)
					{
						//child: it; parent: --it
						parent_ = *it2;
						child_ = *it;
					}
				}
			}
		}
	};
}
#endif//_DECYCLEVISITOR_H_