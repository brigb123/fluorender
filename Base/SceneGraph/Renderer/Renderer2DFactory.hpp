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
#ifndef RENDERER2D_FACTORY_HPP
#define RENDERER2D_FACTORY_HPP

#include <ProcessorFactory.hpp>
#include "Renderer2D.hpp"

namespace fluo
{
	class DrawView;
	class Renderer2DFactory : public ProcessorFactory
	{
	public:
		Renderer2DFactory();

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const Renderer2DFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "Renderer2DFactory"; }

		virtual void createDefault() {}

        virtual Renderer2D* getDefault() { return nullptr; }

        virtual Renderer2D* build(Renderer2D* renderer = nullptr) { return nullptr; }

        virtual Renderer2D* clone(Renderer2D*) { return nullptr; }

        virtual Renderer2D* clone(const unsigned int) { return nullptr; }

		inline virtual Renderer2D* get(size_t i)
		{
			return dynamic_cast<Renderer2D*>(ObjectFactory::get(i));
		}

		inline virtual const Renderer2D* get(size_t i) const
		{
			return dynamic_cast<Renderer2D*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual Renderer2D* find(const unsigned int id)
		{
			return dynamic_cast<Renderer2D*>(ObjectFactory::find(id));
		}

		inline virtual Renderer2D* findFirst(const std::string &name)
		{
			return dynamic_cast<Renderer2D*>(ObjectFactory::findFirst(name));
		}

		inline virtual Renderer2D* findLast(const std::string &name)
		{
			return dynamic_cast<Renderer2D*>(ObjectFactory::findLast(name));
		}

		DrawView* getDrawView(const std::string &name);

	protected:
		virtual ~Renderer2DFactory();

	};

}

#endif//RENDERER2D_FACTORY_HPP
