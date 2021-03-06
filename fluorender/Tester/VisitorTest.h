#pragma once
#include <Scenegraph/NodeVisitor.h>
#include <Scenegraph/Group.h>
#include <iostream>
#include <string>

class InfoVisitor : public flrd::NodeVisitor
{
public:
	InfoVisitor() : level_(0)
	{
		//setTraversalMode(flrd::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		setTraversalMode(flrd::NodeVisitor::TRAVERSE_PARENTS);
	}

	std::string spaces()
	{
		return std::string(level_ * 2, ' ');
	}

	virtual void apply(flrd::Node& node)
	{
		std::cout << spaces() << node.className() << "\t" << &node << std::endl;
		std::cout << spaces() << "  Values: ";
		double value;
		if (node.getValue("value1", value))
			std::cout << "value1 " << value << ' ';
		if (node.getValue("value2", value))
			std::cout << "value2 " << value << ' ';
		if (node.getValue("value3", value))
			std::cout << "value3 " << value << ' ';
		std::cout << std::endl;
		level_++;
		traverse(node);
		level_--;
	}

	virtual void apply(flrd::Group& group)
	{
		std::cout << spaces() << group.className() << std::endl;
		std::cout << spaces() << "  Values: ";
		double value;
		if (group.getValue("value1", value))
			std::cout << "value1 " << value << ' ';
		if (group.getValue("value2", value))
			std::cout << "value2 " << value << ' ';
		if (group.getValue("value3", value))
			std::cout << "value3 " << value << ' ';
		std::cout << std::endl;

		level_++;
		traverse(group);
		level_--;
	}

protected:
	unsigned int level_;
};