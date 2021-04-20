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

#include "DrawSimple.hpp"
#include <Color.hpp>

using namespace fluo;

DrawSimple::DrawSimple():
	Renderer2D()
{
	//setConditionFunction(std::bind(&ViewRenderer::drawType,
	//	this));
}

DrawSimple::DrawSimple(const DrawSimple& renderer, const CopyOp& copyop, bool copy_values):
	Renderer2D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

DrawSimple::~DrawSimple()
{
}

//ProcessorBranchType DrawSimple::drawType()
//{
//}

void DrawSimple::preDraw(Event &event)
{
	clearDraw();
}

void DrawSimple::postDraw(Event &event)
{

}

void DrawSimple::clearDraw()
{
	Color bg_color;
	getValue("bg color", bg_color);
	long nx, ny;
	getValue("nx", nx);
	getValue("ny", ny);
	// clear color and depth buffers
	glClearDepth(1.0);
	glClearColor(bg_color.r(), bg_color.g(), bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);
}