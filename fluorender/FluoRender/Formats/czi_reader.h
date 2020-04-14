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
#ifndef _CZI_READER_H_
#define _CZI_READER_H_

#include <base_reader.h>
#include <vector>
#include <string>

#define HDRSIZE	32//header size
#define FIXSIZE	256//fixed part size

using namespace std;

class CZIReader : public BaseReader
{
public:
	CZIReader();
	~CZIReader();

	int GetType() { return READER_CZI_TYPE; }

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetChannSeq(bool cs);
	bool GetChannSeq();
	void SetDigitOrder(int order);
	int GetDigitOrder();
	void SetTimeId(wstring &id);
	wstring GetTimeId();
	int Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurDataName(int t, int c);
	wstring GetCurMaskName(int t, int c);
	wstring GetCurLabelName(int t, int c);

	wstring GetPathName() { return m_path_name; }
	wstring GetDataName() { return m_data_name; }
	int GetTimeNum() { return m_time_num; }
	int GetCurTime() { return m_cur_time; }
	int GetChanNum() { return m_chan_num; }
	double GetExcitationWavelength(int chan);
	int GetSliceNum() { return m_slice_num; }
	int GetXSize() { return m_x_size; }
	int GetYSize() { return m_y_size; }
	bool IsSpcInfoValid() { return m_valid_spc; }
	double GetXSpc() { return m_xspc; }
	double GetYSpc() { return m_yspc; }
	double GetZSpc() { return m_zspc; }
	double GetMaxValue() { return m_max_value; }
	double GetScalarScale() { return m_scalar_scale; }
	bool GetBatch() { return m_batch; }
	int GetBatchNum() { return (int)m_batch_list.size(); }
	int GetCurBatch() { return m_cur_batch; }

private:
	wstring m_data_name;

/*	struct SliceInfo
	{
		unsigned int offset;	//offset value in lsm file
		unsigned int offset_high;//if it is larger than 4GB, this is the high 32 bits of the 64-bit address
		unsigned int size;		//size in lsm file
	};
	typedef vector<SliceInfo> ChannelInfo;		//all slices form a channel
	typedef vector<ChannelInfo> DatasetInfo;	//channels form a dataset
	vector<DatasetInfo> m_lsm_info;				//datasets of different time points form an lsm file
*/

	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	int m_slice_num;
	int m_x_size;
	int m_y_size;
	bool m_valid_spc;
	double m_xspc;
	double m_yspc;
	double m_zspc;
	double m_max_value;
	double m_scalar_scale;

	//lsm properties
/*	int m_compression;		//1:no compression; 5:lzw compression
	int m_predictor;		//shoud be 2 if above is 5
	unsigned int m_version;	//lsm version
	int m_datatype;			//0: varying; 1: 8-bit; 2: 12-bit; 5: 32-bit
	bool m_l4gb;			//true: this is a larger-than-4-GB file
*/
	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	vector<WavelengthInfo> m_excitation_wavelength_list;

	typedef enum
	{
		SegFile = 0,
		SegDirectory,
		SegSubBlock,
		SegMetadata,
		SegAttach,
		SegAttDir,
		SegDeleted,
		SegAll
	} SegType;

	static std::vector<std::string> m_types;
	bool m_header_read;
	bool m_multi_file;
	unsigned int m_file_part;
	unsigned long long m_dir_pos;//directory segment position
	unsigned long long m_meta_pos;//metadata segment position
	unsigned long long m_att_dir;//attachment directory position

private:
	//segment reader
	bool ReadSegment(FILE* pfile, unsigned long long &ioffset, SegType type = SegAll);
	bool ReadFile(FILE* pfile);
	bool ReadDirectory(FILE* pfile);
	bool ReadSubBlock(FILE* pfile);
	bool ReadMetadata(FILE* pfile, unsigned long long &ioffset);
	bool ReadAttach(FILE* pfile);
	bool ReadAttDir(FILE* pfile);
	bool ReadDeleted(FILE* pfile);
};

#endif//_CZI_READER_H_