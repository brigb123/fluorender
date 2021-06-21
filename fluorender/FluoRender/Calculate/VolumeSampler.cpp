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
#include "VolumeSampler.h"
#include <stdexcept>

using namespace flrd;

VolumeSampler::VolumeSampler() :
	m_result(0),
	m_input(0),
	m_raw_input(0),
	m_raw_result(0),
	m_nx_in(0),
	m_ny_in(0),
	m_nz_in(0),
	m_nx(0),
	m_ny(0),
	m_nz(0),
	m_crop(false),
	m_filter(0),
	m_fx(0),
	m_fy(0),
	m_fz(0),
	m_border(0)
{
}

VolumeSampler::~VolumeSampler()
{
}

void VolumeSampler::SetInput(VolumeData *data)
{
	m_input = data;
}

VolumeData* VolumeSampler::GetInput()
{
	return m_input;
}

VolumeData* VolumeSampler::GetResult()
{
	return m_result;
}

void VolumeSampler::SetSize(int nx, int ny, int nz)
{
	m_nx = nx;
	m_ny = ny;
	m_nz = nz;
}

void VolumeSampler::SetFilter(int type)
{
	m_filter = type;
}

void VolumeSampler::SetFilterSize(int fx, int fy, int fz)
{
	m_fx = fx;
	m_fy = fy;
	m_fz = fz;
}

void VolumeSampler::SetCrop(bool crop)
{
	m_crop = crop;
}

void VolumeSampler::Resize(SampDataType type)
{
	if (!m_input)
		return;
	if (m_nx <= 0 || m_ny <= 0 || m_nz <= 0)
		return;
	m_raw_input = GetRaw(m_input, type);
	//create m_result
	if (m_result)
		delete m_result;
	m_result = new VolumeData();
	//input size
	m_input->GetResolution(m_nx_in, m_ny_in, m_nz_in);
	//input spacing
	double spcx, spcy, spcz;
	m_input->GetSpacings(spcx, spcy, spcz);
	//bits
	Nrrd* input_nrrd = GetNrrd(m_input, type);
	switch (input_nrrd->type)
	{
	case nrrdTypeChar:
	case nrrdTypeUChar:
		m_bits = 8;
		break;
	case nrrdTypeShort:
	case nrrdTypeUShort:
		m_bits = 16;
		break;
	case nrrdTypeInt:
	case nrrdTypeUInt:
		m_bits = 32;
		break;
	}

	//input raw
	m_raw_input = GetRaw(m_input, type);
	//output raw
	unsigned long long total_size = (unsigned long long)m_nx*
		(unsigned long long)m_ny*(unsigned long long)m_nz;
	m_raw_result = (void*)(new unsigned char[total_size * (m_bits /8)]);
	if (!m_raw_result)
		throw std::runtime_error("Unable to allocate memory.");

	unsigned long long index;
	int i, j, k;
	double x, y, z;
	double value;
	for (k = 0; k < m_nz; ++k)
	for (j = 0; j < m_ny; ++j)
	for (i = 0; i < m_nx; ++i)
	{
		index = (unsigned long long)m_nx*(unsigned long long)m_ny*
			(unsigned long long)k + (unsigned long long)m_nx*
			(unsigned long long)j + (unsigned long long)i;
		x = (double(i) + 0.5) / double(m_nx);
		y = (double(j) + 0.5) / double(m_ny);
		z = (double(k) + 0.5) / double(m_nz);
		if (m_bits == 32)
			((unsigned int*)m_raw_result)[index] = SampleInt(x, y, z);
		else
		{
			value = Sample(x, y, z);
			if (m_bits == 8)
				((unsigned char*)m_raw_result)[index] = (unsigned char)(value * 255);
			else if (m_bits == 16)
				((unsigned short*)m_raw_result)[index] = (unsigned short)(value * 65535);
		}
	}

	//write to nrrd
	Nrrd* nrrd_result = nrrdNew();
	if (m_bits == 8)
		nrrdWrap(nrrd_result, (uint8_t*)m_raw_result, nrrdTypeUChar,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);
	else if (m_bits == 16)
		nrrdWrap(nrrd_result, (uint16_t*)m_raw_result, nrrdTypeUShort,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);
	else if (m_bits == 32)
		nrrdWrap(nrrd_result, (uint32_t*)m_raw_result, nrrdTypeUInt,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);

	spcx *= double(m_nx_in) / double(m_nx);
	spcy *= double(m_ny_in) / double(m_ny);
	spcz *= double(m_nz_in) / double(m_nz);
	nrrdAxisInfoSet(nrrd_result, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(nrrd_result, nrrdAxisInfoMax, spcx*m_nx,
		spcy*m_ny, spcz*m_nz);
	nrrdAxisInfoSet(nrrd_result, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrd_result, nrrdAxisInfoSize, (size_t)m_nx,
		(size_t)m_ny, (size_t)m_nz);
}

Nrrd* VolumeSampler::GetNrrd(VolumeData* vd, SampDataType type)
{
	if (!vd || !vd->GetTexture())
		return 0;
	flvr::Texture* tex = vd->GetTexture();
	int index;
	switch (type)
	{
	case SDT_Data:
		index = 0;
		break;
	case SDT_Mask:
		index = tex->nmask();
		break;
	case SDT_Label:
		index = tex->nlabel();
		break;
	}
	return tex->get_nrrd(index);
}

void* VolumeSampler::GetRaw(VolumeData* vd, SampDataType type)
{
	Nrrd* nrrd = GetNrrd(vd, type);
	if (nrrd)
		return nrrd->data;
	return 0;
}

double VolumeSampler::Sample(double x, double y, double z)
{
	switch (m_filter)
	{
	case 0:
		return SampleNearestNeighbor(x, y, z);
	case 1:
		return SampleLinear(x, y, z);
	case 2:
		return SampleBox(x, y, z);
	}
	return 0.0;
}

unsigned int VolumeSampler::SampleInt(double x, double y, double z)
{
	if (!m_raw_input)
		return 0;
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0;
	int nx, ny, nz;
	m_input->GetResolution(nx, ny, nz);
	unsigned long long index = (unsigned long long)nx*(unsigned long long)ny*
		(unsigned long long)k + (unsigned long long)nx*
		(unsigned long long)j + (unsigned long long)i;
	return ((unsigned int*)m_raw_input)[index];
}

bool VolumeSampler::ijk(int &i, int &j, int &k)
{
	if (i < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = 0;
			break;
		case 2:
			i = -1 - i;
			break;
		}
	}
	if (i >= m_nx_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = m_nx_in - 1;
			break;
		case 2:
			i = m_nx_in * 2 - i - 1;
		}
	}
	if (j < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = 0;
			break;
		case 2:
			j = -1 - j;
			break;
		}
	}
	if (j >= m_ny_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = m_ny_in - 1;
			break;
		case 2:
			j = m_ny_in * 2 - j - 1;
		}
	}
	if (k < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = 0;
			break;
		case 2:
			k = -1 - k;
			break;
		}
	}
	if (k >= m_nz_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = m_nz_in - 1;
			break;
		case 2:
			k = m_nz_in * 2 - k - 1;
		}
	}
	return true;
}

void VolumeSampler::xyz2ijk(double x, double y, double z,
	int &i, int &j, int &k)
{
	i = int(x*m_nx_in);
	j = int(y*m_ny_in);
	k = int(z*m_nz_in);
}

double VolumeSampler::SampleNearestNeighbor(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0.0;
	unsigned long long index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	if (m_bits == 8)
		return double(((unsigned char*)m_raw_input)[index]) / 255.0;
	else if (m_bits == 16)
		return double(((unsigned short*)m_raw_input)[index]) / 65535.0;
	return 0.0;
}

double VolumeSampler::SampleLinear(double x, double y, double z)
{
	return 0;
}

double VolumeSampler::SampleBox(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	double sum = 0.0;
	int count = 0;
	unsigned long long index;
	for (int kk=k-m_fz; kk<=k+m_fz; ++kk)
	for (int jj=j-m_fy; jj<=j+m_fy; ++jj)
	for (int ii=i-m_fx; ii<=i+m_fx; ++ii)
	{
		if (ijk(ii, jj, kk))
		{
			index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
				(unsigned long long)kk + (unsigned long long)m_nx_in*
				(unsigned long long)jj + (unsigned long long)ii;
			if (m_bits == 8)
				sum += double(((unsigned char*)m_raw_input)[index]) / 255.0;
			else if (m_bits == 16)
				sum += double(((unsigned short*)m_raw_input)[index]) / 65535.0;
		}
		count++;
	}
	if (count)
		sum /= count;
	index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	//double test = double(((unsigned char*)(m_vd->data))[index]) / 255.0;
	return sum;
}
