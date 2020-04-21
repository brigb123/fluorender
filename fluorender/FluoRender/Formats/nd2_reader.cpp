/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#include "nd2_reader.h"
#include "../compatibility.h"
#include <stdio.h>

ND2Reader::ND2Reader()
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 0.0;
	m_yspc = 0.0;
	m_zspc = 0.0;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;

/*	m_compression = 0;
	m_predictor = 0;
	m_version = 0;
	m_datatype = 0;
	m_l4gb = false;*/
}

ND2Reader::~ND2Reader()
{
}

void ND2Reader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
	m_id_string = m_path_name;
}

void ND2Reader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int ND2Reader::Preprocess()
{
/*	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
		return READER_OPEN_FAIL;

	m_lsm_info.clear();
	m_l4gb = false;

	int i, j;

	unsigned int ioffset = 0;
	fread(&ioffset, sizeof(unsigned int), 1, pfile);
	if (ioffset != 0x002A4949)
	{
		fclose(pfile);
		return READER_FORMAT_ERROR;
	}

	int cnt_image = 0;
	bool full_image = false;

	//first offset
	if (fread(&ioffset, sizeof(unsigned int), 1, pfile) < 1)
	{
		fclose(pfile);
		return READER_FORMAT_ERROR;
	}

	unsigned int prev_offset = 0;
	unsigned int offset_high = 0;

	//images
	while (FSEEK64(pfile, ioffset, SEEK_SET) == 0)
	{
		unsigned short entry_num;
		//entry number
		if (fread(&entry_num, sizeof(unsigned short), 1, pfile) < 1)
		{
			fclose(pfile);
			return READER_FORMAT_ERROR;
		}

		vector<unsigned int> offsets;
		vector<unsigned int> offset_highs;
		vector<unsigned int> sizes;

		for (i = 0; i < entry_num; i++)
		{
			unsigned short tag;
			if (fread(&tag, sizeof(unsigned short), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned short type;
			if (fread(&type, sizeof(unsigned short), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned int length;
			if (fread(&length, sizeof(unsigned int), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned int value;
			if (fread(&value, sizeof(unsigned int), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}

			//remember current position
			long cur_pos = ftell(pfile);

			switch (tag)
			{
			case 0x00FE://254, new subfile type
				if (value == 0)
				{
					full_image = true;
					cnt_image++;
				}
				else
					full_image = false;
				break;
			case 0x0100://256, image width
				if (full_image && cnt_image == 1)
					m_x_size = value;
				break;
			case 0x0101://257, image length
				if (full_image && cnt_image == 1)
					m_y_size = value;
				break;
			case 0x0102://258, bits per sample
				break;
			case 0x0103://259, compression
				if (full_image && cnt_image == 1)
					m_compression = value << 16 >> 16;
				break;
			case 0x0106://262, photometric interpretation
				break;
			case 0x0111://273, strip offsets
				if (full_image)
				{
					if (length == 1)
					{
						offsets.push_back(value);
						if (value < prev_offset)
						{
							m_l4gb = true;
							offset_high++;
						}
						prev_offset = value;
						offset_highs.push_back(offset_high);
					}
					else
					{
						if (FSEEK64(pfile, value, SEEK_SET) == 0)
						{
							unsigned int vtemp;
							for (j = 0; j < (int)length; j++)
							{
								if (fread(&vtemp, sizeof(unsigned int), 1, pfile) == 1)
								{
									offsets.push_back(vtemp);
									if (vtemp < prev_offset)
									{
										m_l4gb = true;
										offset_high++;
									}
									prev_offset = vtemp;
									offset_highs.push_back(offset_high);
								}
							}
						}
					}
				}
				break;
			case 0x0115://277, samples per pixel
				if (full_image && cnt_image == 1)
					m_chan_num = value << 16 >> 16;
				break;
			case 0x0117://279, strip byte counts
				if (full_image)
				{
					if (length == 1)
					{
						sizes.push_back(value);
					}
					else
					{
						if (FSEEK64(pfile, value, SEEK_SET) == 0)
						{
							unsigned int vtemp;
							for (j = 0; j < (int)length; j++)
							{
								if (fread(&vtemp, sizeof(unsigned int), 1, pfile) == 1)
									sizes.push_back(vtemp);
							}
						}
					}
				}
				break;
			case 0x011C://284, planar configuration
				break;
			case 0x013D://317, predictor
				if (full_image && cnt_image == 1)
					m_predictor = value << 16 >> 16;
				break;
			case 0x0140://320, colormap
				break;
			case 0x866C://34412, zeiss lsm info
				if (type == 1)
				{
					if (FSEEK64(pfile, value, SEEK_SET) != 0)
					{
						fclose(pfile);
						return READER_FORMAT_ERROR;
					}
					unsigned char* pdata = new unsigned char[length];
					if (fread(pdata, sizeof(unsigned char), length, pfile) != length)
					{
						fclose(pfile);
						return READER_FORMAT_ERROR;
					}
					//read lsm info
					ReadLsmInfo(pfile, pdata, length);
					//delete
					delete[]pdata;
				}
				break;
			}

			//reset position
			if (FSEEK64(pfile, cur_pos, SEEK_SET) != 0)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
		}

		//build lsm info, which contains all offset values and sizes
		if (full_image)
		{
			int time = (cnt_image - 1) / m_slice_num;
			if (time + 1 > (int)m_lsm_info.size())
			{
				DatasetInfo dinfo;
				for (i = 0; i < m_chan_num; i++)
				{
					ChannelInfo cinfo;
					dinfo.push_back(cinfo);
				}
				m_lsm_info.push_back(dinfo);
			}
			//int slice = (cnt_image-1) % m_slice_num;
			for (i = 0; i < m_chan_num; i++)
			{
				SliceInfo sinfo;
				sinfo.offset = offsets[i];
				sinfo.offset_high = offset_highs[i];
				sinfo.size = sizes[i];
				//add slice info to lsm info
				m_lsm_info[time][i].push_back(sinfo);
			}
		}

		//next image
		if (fread(&ioffset, sizeof(unsigned int), 1, pfile) < 1)
		{
			fclose(pfile);
			return READER_FORMAT_ERROR;
		}
		if (!ioffset)
			break;
	}

	fclose(pfile);

	m_cur_time = 0;
	m_data_name = GET_NAME(m_path_name);

	return READER_OK;*/
}

void ND2Reader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool ND2Reader::GetSliceSeq()
{
	return false;
}

void ND2Reader::SetChannSeq(bool cs)
{
	//do nothing
}

bool ND2Reader::GetChannSeq()
{
	return false;
}

void ND2Reader::SetDigitOrder(int order)
{
	//do nothing
}

int ND2Reader::GetDigitOrder()
{
	return 0;
}

void ND2Reader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring ND2Reader::GetTimeId()
{
	return wstring(L"");
}

void ND2Reader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES(m_path_name, L"*.lsm", m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int ND2Reader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
	{
		m_path_name = m_batch_list[index];
		Preprocess();
		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
}

double ND2Reader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* ND2Reader::Convert(int t, int c, bool get_max)
{
/*	Nrrd *data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
		return 0;

	int i, j;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num > 0 &&
		m_x_size > 0 &&
		m_y_size > 0 &&
		t < (int)m_lsm_info.size() &&
		c < (int)m_lsm_info[t].size())
	{
		//allocate memory for nrrd
		switch (m_datatype)
		{
		case 1://8-bit
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned char *val = new (std::nothrow) unsigned char[mem_size];
			ChannelInfo *cinfo = &m_lsm_info[t][c];
			for (i = 0; i < (int)cinfo->size(); i++)
			{
				if (m_l4gb ?
					FSEEK64(pfile, ((uint64_t((*cinfo)[i].offset_high)) << 32) + (*cinfo)[i].offset, SEEK_SET) == 0 :
					FSEEK64(pfile, (*cinfo)[i].offset, SEEK_SET) == 0)
				{
					unsigned int val_pos = m_x_size*m_y_size*i;
					if (m_compression == 1)
						fread(val + val_pos, sizeof(unsigned char), (*cinfo)[i].size, pfile);
					else if (m_compression == 5)
					{
						unsigned char* tif = new (std::nothrow) unsigned char[(*cinfo)[i].size];
						fread(tif, sizeof(unsigned char), (*cinfo)[i].size, pfile);
						LZWDecode(tif, val + val_pos, (*cinfo)[i].size);
						for (j = 0; j < m_y_size; j++)
							DecodeAcc8(val + val_pos + j*m_x_size, m_x_size, 1);
						delete[]tif;
					}
				}
			}
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		case 2://16-bit
		case 3:
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned short *val = new (std::nothrow) unsigned short[mem_size];
			ChannelInfo *cinfo = &m_lsm_info[t][c];
			for (i = 0; i < (int)cinfo->size(); i++)
			{
				if (m_l4gb ?
					FSEEK64(pfile, ((uint64_t((*cinfo)[i].offset_high)) << 32) + (*cinfo)[i].offset, SEEK_SET) == 0 :
					FSEEK64(pfile, (*cinfo)[i].offset, SEEK_SET) == 0)
				{
					unsigned int val_pos = m_x_size*m_y_size*i;
					if (m_compression == 1)
						fread(val + val_pos, sizeof(unsigned char), (*cinfo)[i].size, pfile);
					else if (m_compression == 5)
					{
						unsigned char* tif = new (std::nothrow) unsigned char[(*cinfo)[i].size];
						fread(tif, sizeof(unsigned char), (*cinfo)[i].size, pfile);
						LZWDecode(tif, (tidata_t)(val + val_pos), (*cinfo)[i].size);
						for (j = 0; j < m_y_size; j++)
							DecodeAcc16((tidata_t)(val + val_pos + j*m_x_size), m_x_size, 1);
						delete[]tif;
					}
				}
			}
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		}
	}

	fclose(pfile);
	m_cur_time = t;
	return data;*/
}

wstring ND2Reader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring ND2Reader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring ND2Reader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
	return label_name;
}
