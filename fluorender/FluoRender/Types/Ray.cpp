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

#include <Types/Ray.h>
#include <Types/Utils.h>

namespace fluo
{
	Ray::Ray(const Point& o, const Vector& d)
		: o_(o), d_(d)
	{
	}

	Ray::Ray(const Ray& copy)
		: o_(copy.o_), d_(copy.d_)
	{
	}

	Ray::~Ray()
	{
	}

	Ray& Ray::operator=(const Ray& copy)
	{
		o_ = copy.o_;
		d_ = copy.d_;
		return *this;
	}

	bool Ray::operator==(const Ray& r) const
	{
		return (o_ == r.o_) && (d_ == r.d_);
	}

	bool Ray::operator!=(const Ray& r) const
	{
		return (o_ != r.o_) || (d_ != r.d_);
	}

	Point Ray::parameter(double t)
	{
		return o_ + d_*t;
	}

	bool
		Ray::planeIntersectParameter(Vector& N, Point& P, double& t)
	{
		//! Computes the ray parameter t at which the ray R will

		//! point P

		/*  Dot(N, ((O + t0*V) - P)) = 0   solve for t0 */

		Point O(o_);
		Vector V(d_);
		double D = -(N.x()*P.x() + N.y()*P.y() + N.z()*P.z());
		double NO = (N.x()*O.x() + N.y()*O.y() + N.z()*O.z());

		double NV = Dot(N, V);

		if (fabs(NV) <= Epsilon()) // ray is parallel to plane
			return false;
		else {
			t = -(D + NO) / NV;
			return true;
		}
	}

	void Ray::normalize()
	{
		d_.normalize();
	}

} // End namespace fluo

