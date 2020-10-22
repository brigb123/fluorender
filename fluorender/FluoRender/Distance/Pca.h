/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2019 Scientific Computing and Imaging Institute,
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
#ifndef FL_Pca_h
#define FL_Pca_h

#include <FLIVR/Point.h>
#include <FLIVR/Vector.h>
#include <vector>

namespace FL
{
	class Pca
	{
	public:
		Pca(int mode = 0):
			m_mode(mode)
		{
			ClearPoints();
		}
		~Pca()
		{}

		void AddPoint(FLIVR::Point &point)
		{
			if (m_mode == 0)
			{
				m_mean = FLIVR::Point(m_mean + point);
				m_cov[0][0] += point(0) * point(0);
				m_cov[0][1] += point(0) * point(1);
				m_cov[0][2] += point(0) * point(2);
				m_cov[1][1] += point(1) * point(1);
				m_cov[1][2] += point(1) * point(2);
				m_cov[2][2] += point(2) * point(2);
				m_num++;
			}
			else if (m_mode == 2)
				m_points.push_back(point);
		}
		void SetPoints(std::vector<FLIVR::Point> &points)
		{
			m_points.assign(points.begin(), points.end());
			m_mode = 2;
		}
		void SetCovMat(std::vector<double> &cov)
		{
			int size = cov.size();
			if (size < 6)
				return;
			if (size > 9)
				size = 9;
			std::memcpy(m_cov, &cov[0], size * sizeof(double));
			if (size < 9)
			{
				m_cov[2][2] = m_cov[1][2];
				m_cov[2][1] = m_cov[1][1];
				m_cov[2][0] = m_cov[0][2];
				m_cov[1][2] = m_cov[1][1];
				m_cov[1][1] = m_cov[1][0];
				m_cov[1][0] = m_cov[0][1];
			}
			m_mode = 1;
		}

		void ClearPoints()
		{
			m_num = 0;
			m_mean = FLIVR::Point();
			memset(m_cov, 0, sizeof(double) * 9);
			m_points.clear();
		}
		void Compute();

		FLIVR::Vector GetAxis(int index)
		{
			if (index >= 0 && index <= 2)
				return m_axis[index];
			else
				return FLIVR::Vector();
		}

	private:
		int m_mode;//0-incremental cov; 1-external cov; 2-store points
		std::vector<FLIVR::Point> m_points;
		FLIVR::Vector m_axis[3];
		double m_values[3];
		int m_num;
		FLIVR::Point m_mean;
		double m_cov[3][3];//covariance matrix

	private:

	};
}
#endif//FL_Pca_h