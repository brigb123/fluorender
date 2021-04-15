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
#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <Group.hpp>
#include "ProcessorNode.hpp"
#include "ProcessorVisitor.hpp"

namespace fluo
{
	typedef std::function<ProcessorBranchType()> conditionFunctionType;
	class Processor : public Group
	{
	public:

		Processor();

		Processor(const Processor& prc, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Processor* clone(const CopyOp& copyop) const { return new Processor(*this, copyop); };

		virtual bool isSameKindAs(const Processor*) const {return true;}

		virtual const char* className() const { return "Processor"; }

		//manage conditions to implement a decision tree
		virtual void setConditionFunction(conditionFunctionType func)
		{
			condition_func_ = func;
		}
		virtual ProcessorBranchType condition()
		{
			if (condition_func_)
				return condition_func_();
			else
				return PBT_DONTCARE;
		};

	protected:
		~Processor();

	protected:
		conditionFunctionType condition_func_;
	};
}
#endif//PROCESSOR_HPP
