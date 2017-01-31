/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "exmax.h"
#include <algorithm>
#include <fstream>

using namespace FL;

ClusterExmax::ClusterExmax() :
	m_clnum(2),
	m_weak_result(false),
	m_eps(1e-3),
	m_max_iter(200)
{

}

ClusterExmax::~ClusterExmax()
{

}

bool ClusterExmax::Execute()
{
	if (m_data.empty())
		return false;

	Initialize();

	size_t counter = 0;
	do {
		m_params_prv = m_params;
		m_likelihood_prv = m_likelihood;
		Expectation();
		Maximization();
		//histograom test
		GenUncertainty(0.05);
		counter++;
	} while (!Converge() &&
		counter < m_max_iter);

	if (counter == m_max_iter)
	{
		if (m_weak_result)
		{
			GenResult();
			if (GetProb() > 0.8)
				return true;
		}
		return false;
	}
	else
	{
		GenResult();
		if (GetProb() > 0.95)
			return true;
		else
			return false;
	}
}

float ClusterExmax::GetProb()
{
	if (m_mem_prob.empty())
		return 0.0f;

	double sum = 0;
	double max;
	size_t size = m_mem_prob[0].size();
	for (size_t i = 0; i < size; ++i)
	{
		max = 0;
		for (size_t j = 0; j < m_clnum; ++j)
		{
			if (m_mem_prob[j][i] > max)
				max = m_mem_prob[j][i];
		}
		sum += max;
	}
	if (size)
		return float(sum / size);
	else
		return 0.0f;
}

void ClusterExmax::Initialize()
{
	//use same tau and covar
	FLIVR::Vector trace;
	FLIVR::Vector mean;
	FLIVR::Vector vec;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
		mean += FLIVR::Vector((*iter)->center);
	mean /= m_data.size();
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		vec = FLIVR::Vector((*iter)->center) - mean;
		trace += vec * vec;
	}
	trace /= m_data.size() - 1;
	FLIVR::Mat3 covar(trace);
	double tau = 1.0 / m_clnum;

	//use similar method as k-means for means
	//search for maximum
	pClusterPoint p = nullptr;
	Cluster cluster;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		if (iter == m_data.begin())
			p = *iter;
		else
		{
			if ((*iter)->intensity > p->intensity)
				p = *iter;
		}
	}
	if (p != nullptr)
	{
		Params params;
		params.tau = tau;
		params.mean = p->center;
		params.covar = covar;
		m_params.push_back(params);
		cluster.push_back(p);
	}
	//search for the rest
	for (int i = 1; i < m_clnum; ++i)
	{
		p = nullptr;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			if (cluster.find(*iter))
				continue;
			if (p == nullptr)
				p = *iter;
			else
			{
				FLIVR::Point pi_1 = m_params[i - 1].mean;
				double d1 = (p->center - pi_1).length();
				double d2 = ((*iter)->center - pi_1).length();
				if (d2 > d1)
					p = *iter;
			}
		}
		if (p != nullptr)
		{
			Params params;
			params.tau = tau;
			params.mean = p->center;
			params.covar = covar;
			m_params.push_back(params);
			cluster.push_back(p);
		}
	}

	//allocate membership probabilities
	m_mem_prob.resize(m_clnum);
	for (size_t i = 0; i < m_clnum; ++i)
		m_mem_prob[i].resize(m_data.size(), 0);
	m_mem_prob_prv.clear();
	m_count.resize(m_data.size(), 0);
}

void ClusterExmax::Expectation()
{
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double sum = 0;
		for (unsigned int j = 0; j < m_clnum; ++j)
			sum += m_params_prv[j].tau *
			Gaussian((*iter)->center,
				m_params_prv[j].mean,
				m_params_prv[j].covar);
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			if (sum > 0.0)
				m_mem_prob[j][i] = m_params_prv[j].tau *
				Gaussian((*iter)->center, m_params_prv[j].mean,
					m_params_prv[j].covar) / sum;
			else
				m_mem_prob[j][i] = 1.0;
		}
		i++;
	}
}

double ClusterExmax::Gaussian(FLIVR::Point &p, FLIVR::Point &m, FLIVR::Mat3 &s)
{
	double pi2_3 = 248.050213442399; //(2pi)^3
	double det = s.det();
	if (det == 0.0)
	{
		//perturb
		s.mat[0][0] = s.mat[0][0] < 0.5 ? 0.5 : s.mat[0][0];
		s.mat[1][1] = s.mat[1][1] < 0.5 ? 0.5 : s.mat[1][1];
		s.mat[2][2] = s.mat[2][2] < 0.5 ? 0.5 : s.mat[2][2];
		det = s.det();
	}
	FLIVR::Vector d = p - m;
	return exp(-0.5 * FLIVR::Dot(d, s.inv() * d)) / sqrt(pi2_3 * det);
}

void ClusterExmax::Maximization()
{
	//update params
	for (unsigned int j = 0; j < m_clnum; ++j)
	{
		unsigned int i;
		double sum_t = 0;
		for (i = 0; i < m_data.size(); ++i)
			sum_t += m_mem_prob[j][i];

		//tau
		m_params[j].tau = sum_t / m_data.size();

		//mean
		i = 0;
		FLIVR::Point sum_p;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			sum_p += (*iter)->center * m_mem_prob[j][i];
			i++;
		}
		m_params[j].mean = sum_p / sum_t;

		//covar/sigma
		i = 0;
		FLIVR::Mat3 sum_s(0);
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			FLIVR::Vector d = (*iter)->center - m_params[j].mean;
			sum_s += form(d, d) * m_mem_prob[j][i];
			i++;
		}
		m_params[j].covar = sum_s / sum_t;
	}
}

bool ClusterExmax::Converge()
{
	//compute likelihood
	double c = 2.75681559961402;
	m_likelihood = 0;
	double l;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			l = log(m_params_prv[j].tau);
			l -= 0.5 * log(m_params_prv[j].covar.det());
			FLIVR::Vector d = (*iter)->center - m_params_prv[j].mean;
			l -= 0.5 * FLIVR::Dot(d, m_params_prv[j].covar.inv() * d);
			l -= c;
			m_likelihood += l;
		}
	}

	if (fabs(m_likelihood - m_likelihood_prv) > m_eps)
		return false;
	else
		return true;
}

void ClusterExmax::GenResult()
{
	m_result.clear();
	m_result.resize(m_clnum);
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		int index = -1;
		double max_mem_prob;
		for (int j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
			{
				index = j;
				max_mem_prob = m_mem_prob[j][i];
			}
			else
			{
				if (m_mem_prob[j][i] > max_mem_prob)
				{
					index = j;
					max_mem_prob = m_mem_prob[j][i];
				}
			}
		}
		if (index > -1)
			m_result[index].push_back(*iter);
		i++;
	}
}

void ClusterExmax::GenHistogram(size_t bins)
{
	if (!m_clnum)
		return;

	m_bins = bins;
	//allocate histogram space
	m_histogram.clear();
	double value = 1.0 / m_clnum;
	double inc = (1.0 - value) / m_bins;
	for (size_t i = 0; i < m_bins; ++i)
	{
		EmBin bin;
		bin.value = value;
		value += inc;
		bin.count = 0;
		m_histogram.push_back(bin);
	}
	//fill in histogram
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double max_mem_prob = 0;
		for (int j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
				max_mem_prob = m_mem_prob[j][i];
			else if (m_mem_prob[j][i] > max_mem_prob)
				max_mem_prob = m_mem_prob[j][i];
		}
		i++;
		//
		size_t index = size_t((max_mem_prob - 
			1.0 / m_clnum) / inc);
		if (index < m_bins)
			m_histogram[index].count++;
	}
	//output histogram
	std::ofstream outfile;
	outfile.open("hist.txt", std::ofstream::out |
		std::ofstream::app);
	for (size_t i = 0; i < m_histogram.size(); ++i)
		if (i < m_histogram.size() - 1)
			outfile << m_histogram[i].count << "\t";
		else
			outfile << m_histogram[i].count << "\n";
	outfile.close();
}

void ClusterExmax::GenUncertainty(double delta, bool output)
{
	if (!m_clnum)
		return;
	if (!m_mem_prob_prv.size())
	{
		m_mem_prob_prv = m_mem_prob;
		return;
	}

	//compute count
	for (size_t i = 0; i < m_mem_prob[0].size(); ++i)
	{
		double var = 0;
		for (size_t j = 0; j < m_mem_prob.size(); ++j)
			var += fabs(m_mem_prob[j][i] -
				m_mem_prob_prv[j][i]);
		//size_t count = size_t(var / delta);
		//m_count[i] += count;
		m_count[i] += var;
	}
	//allocate histogram space
	m_histogram.clear();
	//fill in histogram
	for (size_t i = 0; i < m_count.size(); ++i)
	{
		//if (m_histogram.size() <= m_count[i])
		//	m_histogram.resize(m_count[i] + 1);
		//m_histogram[m_count[i]].count++;
		size_t index = m_count[i] / delta;
		if (m_histogram.size() <= index)
		{
			m_histogram.resize(index + 1);
			m_histogram[index].value = index * delta;
		}
		m_histogram[index].count++;
	}
	m_mem_prob_prv = m_mem_prob;

	if (output)
	{
		//output histogram
		std::ofstream outfile;
		outfile.open("hist.txt", std::ofstream::out |
			std::ofstream::app);
		for (size_t i = 0; i < m_histogram.size(); ++i)
			if (i < m_histogram.size() - 1)
				outfile << m_histogram[i].count << "\t";
			else
				outfile << m_histogram[i].count << "\n";
		outfile.close();
	}
}

void ClusterExmax::GenerateNewColors(void* label,
	size_t nx, size_t ny, size_t nz)
{
	//m_id_list.clear();

	unsigned int id;
	unsigned long long index;
	int i, j, k;

	double scale = m_histogram.back().value;
	scale = 350 / scale;

	unsigned int ii = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		//id = m_count[ii] * 10 + 1;
		id = m_count[ii] * scale + 1;
		ii++;
		i = int((*iter)->center.x() + 0.5);
		if (i < 0 || i >= nx)
			continue;
		j = int((*iter)->center.y() + 0.5);
		if (j < 0 || j >= ny)
			continue;
		k = int((*iter)->center.z() + 0.5);
		if (k < 0 || k >= nz)
			continue;
		index = nx*ny*k + nx*j + i;
		((unsigned int*)label)[index] = id;
	}

}

void ClusterExmax::GenerateNewColors2(void* label,
	size_t nx, size_t ny, size_t nz)
{
	//m_id_list.clear();

	unsigned int id;
	int i, j, k;

	unsigned int ii = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		int index = -1;
		double max_mem_prob;
		id = 0;
		for (int j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
			{
				index = j;
				max_mem_prob = m_mem_prob[j][ii];
			}
			else
			{
				if (m_mem_prob[j][ii] > max_mem_prob)
				{
					index = j;
					max_mem_prob = m_mem_prob[j][ii];
				}
			}
		}
		if (index > -1)
			id = (1.0 - max_mem_prob) * 700 + 1;
		ii++;
		i = int((*iter)->center.x() + 0.5);
		if (i < 0 || i >= nx)
			continue;
		j = int((*iter)->center.y() + 0.5);
		if (j < 0 || j >= ny)
			continue;
		k = int((*iter)->center.z() + 0.5);
		if (k < 0 || k >= nz)
			continue;
		index = nx*ny*k + nx*j + i;
		((unsigned int*)label)[index] = id;
	}

}
