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

#ifndef GLFLOAT4_HPP
#define GLFLOAT4_HPP

#include <string>
#include <iosfwd>
#include <iostream>

#include <Glew/GL/glew.h>

namespace FLTYPE
{
	class GLfloat4
	{
		GLfloat v_[4];
	public:
		inline GLfloat4(const GLfloat v[4]);
		inline GLfloat4(const GLfloat v1, const GLfloat v2, const GLfloat v3, const GLfloat v4);
		inline GLfloat4(const GLfloat4&);
		inline GLfloat4();
		inline GLfloat4(GLfloat init);

		inline GLfloat* get();
		inline GLfloat4& operator=(const GLfloat4&);
		inline GLfloat4& operator=(const GLfloat&);
		inline GLfloat4& operator=(const GLint&);
		inline GLfloat& operator[](int idx);
		inline GLfloat operator[](int idx) const;
		inline int operator==(const GLfloat4&) const;
		inline int operator!=(const GLfloat4&) const;

		friend std::ostream& operator<<(std::ostream& os, const GLfloat4& v)
		{
			//avoid using spaces so that it can be read correctly using >>
			os << '[' << v[0] << ',' << v[1] << ',' << v[2] << ',' << v[3] << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, GLfloat4& v)
		{
			GLfloat v1, v2, v3, v4;
			char st;
			is >> st >> v1 >> st >> v2 >> st >> v3 >> st >> v4 >> st;
			v = GLfloat4(v1, v2, v3, v4);
			return is;
		}
	};

	inline GLfloat4::GLfloat4(const GLfloat v[4])
	{
		v_[0] = v[0]; v_[1] = v[1]; v_[2] = v[2]; v_[3] = v[3];
	}

	inline GLfloat4::GLfloat4(const GLfloat v1, const GLfloat v2, const GLfloat v3, const GLfloat v4)
	{
		v_[0] = v1; v_[1] = v2; v_[2] = v3; v_[3] = v4;
	}

	inline GLfloat4::GLfloat4(const GLfloat4& v)
	{
		v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2]; v_[3] = v.v_[3];
	}

	inline GLfloat4::GLfloat4()
	{
		v_[0] = 0.0f; v_[1] = 0.0f; v_[2] = 0.0f; v_[3] = 0.0f;
	}

	inline GLfloat4::GLfloat4(GLfloat init)
	{
		v_[0] = init; v_[1] = init; v_[2] = init; v_[3] = init;
	}

	inline GLfloat* GLfloat4::get()
	{
		return &(v_[0]);
	}

	inline GLfloat4& GLfloat4::operator=(const GLfloat4& v)
	{
		v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2]; v_[3] = v.v_[3];
		return *this;
	}

	inline GLfloat4& GLfloat4::operator=(const GLfloat& v)
	{
		v_[0] = v; v_[1] = v; v_[2] = v; v_[3] = v;
		return *this;
	}

	inline GLfloat4& GLfloat4::operator=(const GLint& v)
	{
		v_[0] = static_cast<GLfloat>(v);
		v_[1] = static_cast<GLfloat>(v);
		v_[2] = static_cast<GLfloat>(v);
		v_[3] = static_cast<GLfloat>(v);
		return *this;
	}

	inline GLfloat& GLfloat4::operator[](int idx)
	{
		return v_[idx];
	}

	inline GLfloat GLfloat4::operator[](int idx) const
	{
		return v_[idx];
	}

	inline int GLfloat4::operator==(const GLfloat4& v) const
	{
		return v_[0] == v.v_[0] &&
			v_[1] == v.v_[1] &&
			v_[2] == v.v_[2] &&
			v_[3] == v.v_[3];
	}

	inline int GLfloat4::operator!=(const GLfloat4& v) const
	{
		return v_[0] != v.v_[0] ||
			v_[1] != v.v_[1] ||
			v_[2] != v.v_[2] ||
			v_[3] != v.v_[3];
	}
}

#endif//_GLFLOAT4_H_
