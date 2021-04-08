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
#ifndef RENDERER_FACTORY_HPP
#define RENDERER_FACTORY_HPP

#include <Processor/ProcessorFactory.hpp>
#include "Renderer.hpp"

namespace FLR
{
	class RendererFactory : public fluo::ProcessorFactory
	{
	public:
		RendererFactory();

		virtual bool isSameKindAs(const fluo::Object* obj) const
        { return dynamic_cast<const RendererFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "RendererFactory"; }

		virtual void createDefault() {}

        virtual Renderer* getDefault() { return nullptr; }

        virtual Renderer* build(Renderer* renderer = nullptr) { return nullptr; }

        virtual Renderer* clone(Renderer*) { return nullptr; }

        virtual Renderer* clone(const unsigned int) { return nullptr; }

		inline virtual Renderer* get(size_t i)
		{
			return dynamic_cast<Renderer*>(fluo::ObjectFactory::get(i));
		}

		inline virtual const Renderer* get(size_t i) const
		{
			return dynamic_cast<Renderer*>(const_cast<fluo::Object*>(fluo::ObjectFactory::get(i)));
		}

		inline virtual Renderer* find(const unsigned int id)
		{
			return dynamic_cast<Renderer*>(fluo::ObjectFactory::find(id));
		}

		inline virtual Renderer* findFirst(const std::string &name)
		{
			return dynamic_cast<Renderer*>(fluo::ObjectFactory::findFirst(name));
		}

		inline virtual Renderer* findLast(const std::string &name)
		{
			return dynamic_cast<Renderer*>(fluo::ObjectFactory::findLast(name));
		}

		//FLR::ClipPlaneRenderer* getOrAddClipPlaneRenderer(const std::string &name);

	protected:
		virtual ~RendererFactory();

	};

}

#endif//RENDERER_FACTORY_HPP
