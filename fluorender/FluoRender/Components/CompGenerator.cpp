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
#include "DataManager.h"
#include "CompGenerator.h"
#include "cl_code.h"
#include <algorithm>
#ifdef _DEBUG
#include <fstream>
#endif

using namespace flrd;

ComponentGenerator::ComponentGenerator(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false)
{
}

ComponentGenerator::~ComponentGenerator()
{
}

bool ComponentGenerator::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void ComponentGenerator::ShuffleID()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_shuffle_id_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_mask)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	//clipping planes
	cl_float4 p[6];
	if (m_vd && m_vd->GetVR())
	{
		vector<fluo::Plane*> *planes = m_vd->GetVR()->get_planes();
		double abcd[4];
		for (size_t i = 0; i < 6; ++i)
		{
			(*planes)[i]->get(abcd);
			p[i] = { float(abcd[0]),
				float(abcd[1]),
				float(abcd[2]),
				float(abcd[3]) };
		}
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index, 6,
			sizeof(unsigned int), (void*)(&lenz));
		kernel_prog->setKernelArgConst(kernel_index, 7,
			sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(kernel_index, 8,
			sizeof(cl_float4), (void*)(p+1));
		kernel_prog->setKernelArgConst(kernel_index, 9,
			sizeof(cl_float4), (void*)(p+2));
		kernel_prog->setKernelArgConst(kernel_index, 10,
			sizeof(cl_float4), (void*)(p+3));
		kernel_prog->setKernelArgConst(kernel_index, 11,
			sizeof(cl_float4), (void*)(p+4));
		kernel_prog->setKernelArgConst(kernel_index, 12,
			sizeof(cl_float4), (void*)(p+5));
		//brick matrix
		fluo::BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.Max().x() - bbx.Min().x()),
			float(bbx.Max().y() - bbx.Min().y()),
			float(bbx.Max().z() - bbx.Min().z()) };
		cl_float3 trl ={
			float(bbx.Min().x()),
			float(bbx.Min().y()),
			float(bbx.Min().z())};
		kernel_prog->setKernelArgConst(kernel_index, 13,
			sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(kernel_index, 14,
			sizeof(cl_float3), (void*)(&trl));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index, 15,
				CL_MEM_READ_ONLY, mid);
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		//kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		//ReleaseLabel(val32, brick_num, b);

		m_sig_progress();
	}
}

void ComponentGenerator::SetIDBit(int psize)
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_set_bit_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	//int kernel_index1;
	int kernel_index2;
	int kernel_index3;
	if (m_use_mask)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_4");
		//kernel_index1 = kernel_prog->createKernel("kernel_5");
		kernel_index2 = kernel_prog->createKernel("kernel_6");
		kernel_index3 = kernel_prog->createKernel("kernel_7");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		//kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
		kernel_index3 = kernel_prog->createKernel("kernel_3");
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//size buffer
		unsigned int* size_buffer = 0;

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;
		size_buffer = new unsigned int[data_size];
		memset(size_buffer, 0, label_size);

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		flvr::Argument arg_szbuf = kernel_prog->setKernelArgBuf(kernel_index0, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			label_size, size_buffer);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index0, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index0, 6,
			sizeof(unsigned int), (void*)(&lenz));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index0, 7,
				CL_MEM_READ_ONLY, mid);
		//kernel 1 ger max size
		/*arg_szbuf.kernel_index = kernel_index1;
		arg_szbuf.index = 0;
		kernel_prog->setKernelArgument(arg_szbuf);
		kernel_prog->setKernelArgConst(kernel_index1, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&nz));
		unsigned int maxv = 0;
		Argument arg_maxv = kernel_prog->setKernelArgBuf(
			kernel_index1, 4,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&maxv));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index1, 5,
				CL_MEM_READ_ONLY, mid);*/
		//kernel 2
		arg_szbuf.kernel_index = kernel_index2;
		arg_szbuf.index = 0;
		kernel_prog->setKernelArgument(arg_szbuf);
		arg_label.kernel_index = kernel_index2;
		arg_label.index = 1;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_index2, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index2, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index2, 6,
			sizeof(unsigned int), (void*)(&lenz));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index2, 7,
				CL_MEM_READ_ONLY, mid);
		//kernel 3
		arg_szbuf.kernel_index = kernel_index3;
		arg_szbuf.index = 0;
		kernel_prog->setKernelArgument(arg_szbuf);
		arg_label.kernel_index = kernel_index3;
		arg_label.index = 1;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_index3, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index3, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index3, 4,
			sizeof(unsigned int), (void*)(&nz));
		//unsigned int limit = 1;
		//limit *= std::min(iter, nx);
		////limit *= std::min(iter, ny);
		//limit *= std::min(iter, nz);
		kernel_prog->setKernelArgConst(kernel_index3, 5,
			sizeof(unsigned int), (void*)(&psize));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index3, 6,
				CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		//debug
		//kernel_prog->readBuffer(arg_szbuf, size_buffer);
		//ofs.open("E:/DATA/Test/density_field/size.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)size_buffer, nx*ny*nz*sizeof(unsigned int));
		//ofs.close();
		//kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
		//kernel_prog->readBuffer(arg_maxv, &maxv);
		//maxv = (unsigned int)(psize * maxv + 0.5);
		//kernel_prog->setKernelArgConst(kernel_index3, 5,
		//	sizeof(unsigned int), (void*)(&maxv));
		kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);
		//debug
		//val = new unsigned int[nx*ny*nz];
		//kernel_prog->readBuffer(arg_szbuf, val);
		//ofs.open("E:/DATA/Test/density_field/df.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz*sizeof(unsigned int));
		//delete[] val;
		//ofs.close();

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		delete[] size_buffer;

		m_sig_progress();
	}
}

void ComponentGenerator::Grow(bool diffuse, int iter, float tran, float falloff, float sscale)
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_brainbow_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_mask)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//auto iter
		int biter = iter;
		if (biter < 0)
			biter = std::max(std::max(nx, ny), nz);

		unsigned int rcnt = 0;
		unsigned int seed = biter > 10 ? biter : 11;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgTex3D(kernel_index0, 0,
			CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(kernel_index0, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index0, 6,
			sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(kernel_index0, 7,
			sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(kernel_index0, 8,
			sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 9,
			sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 10,
			sizeof(float), (void*)(&sscale));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index0, 11,
				CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < biter; ++j)
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		m_sig_progress();
	}
}

void ComponentGenerator::DensityField(int dsize, int wsize,
	bool diffuse, int iter, float tran, float falloff,
	float density, float sscale)
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_density_field_3d);
	if (!kernel_prog)
		return;

	int kernel_index0 = kernel_prog->createKernel("kernel_0");
	int kernel_index1 = kernel_prog->createKernel("kernel_1");
	int kernel_index2 = kernel_prog->createKernel("kernel_2");

	flvr::KernelProgram* kernel2_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_density_grow_3d);
	if (!kernel2_prog)
		return;
	int kernel2_index0;
	if (m_use_mask)
		kernel2_index0 = kernel2_prog->createKernel("kernel_1");
	else
		kernel2_index0 = kernel2_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		gsx = wsize >= nx ? nx : wsize;
		gsy = wsize >= ny ? ny : wsize;
		gsz = wsize >= nz ? nz : wsize;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		//set
		//kernel 0
		flvr::Argument arg_img = kernel_prog->setKernelArgTex3D(kernel_index0, 0,
			CL_MEM_READ_ONLY, did);
		flvr::Argument arg_df = kernel_prog->setKernelArgBuf(
			kernel_index0, 1, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		int dnxy = dnx * dny;
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&dnx));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(int), (void*)(&dsize));
		kernel_prog->setKernelArgConst(kernel_index0, 5,
			sizeof(float), (void*)(&sscale));
		//kernel 1
		arg_df.kernel_index = kernel_index1;
		arg_df.index = 0;
		kernel_prog->setKernelArgument(arg_df);
		flvr::Argument arg_gavg = kernel_prog->setKernelArgBuf(
			kernel_index1, 1, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		flvr::Argument arg_gvar = kernel_prog->setKernelArgBuf(
			kernel_index1, 2, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		kernel_prog->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&gsx));
		kernel_prog->setKernelArgConst(kernel_index1, 4,
			sizeof(unsigned int), (void*)(&gsy));
		kernel_prog->setKernelArgConst(kernel_index1, 5,
			sizeof(unsigned int), (void*)(&gsz));
		int ngxy = ngy * ngx;
		kernel_prog->setKernelArgConst(kernel_index1, 6,
			sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog->setKernelArgConst(kernel_index1, 7,
			sizeof(unsigned int), (void*)(&ngx));
		kernel_prog->setKernelArgConst(kernel_index1, 8,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog->setKernelArgConst(kernel_index1, 9,
			sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog->setKernelArgConst(kernel_index2, 2,
			sizeof(unsigned int), (void*)(&gsx));
		kernel_prog->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&gsy));
		kernel_prog->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&gsz));
		kernel_prog->setKernelArgConst(kernel_index2, 5,
			sizeof(unsigned int), (void*)(&ngx));
		kernel_prog->setKernelArgConst(kernel_index2, 6,
			sizeof(unsigned int), (void*)(&ngy));
		kernel_prog->setKernelArgConst(kernel_index2, 7,
			sizeof(unsigned int), (void*)(&ngz));
		kernel_prog->setKernelArgConst(kernel_index2, 8,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog->setKernelArgConst(kernel_index2, 9,
			sizeof(unsigned int), (void*)(&dnx));

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//init
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		//debug
		//val = new unsigned char[dnx*dny*dnz];
		//kernel_prog->readBuffer(arg_df, val);
		//ofs.open("E:/DATA/Test/density_field/df.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//delete[] val;
		//ofs.close();
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
		//debug
		//val = new unsigned char[ngx*ngy*ngz];
		//kernel_prog->readBuffer(arg_gavg, val);
		//ofs.open("E:/DATA/Test/density_field/arg_gavg.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, ngx*ngy*ngz);
		//ofs.close();
		//kernel_prog->readBuffer(arg_gvar, val);
		//ofs.open("E:/DATA/Test/density_field/arg_gvar.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, ngx*ngy*ngz);
		//ofs.close();
		//delete[] val;
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		flvr::Argument arg_avg = kernel_prog->setKernelArgBuf(
			kernel_index2, 0, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		arg_gavg.kernel_index = kernel_index2;
		arg_gavg.index = 1;
		kernel_prog->setKernelArgument(arg_gavg);
		kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		//compute var
		flvr::Argument arg_var = kernel_prog->setKernelArgBuf(
			kernel_index2, 0, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		arg_gvar.kernel_index = kernel_index2;
		arg_gvar.index = 1;
		kernel_prog->setKernelArgument(arg_gvar);
		kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);

		//debug
		//val = new unsigned char[dnx*dny*dnz];
		//kernel_prog->readBuffer(arg_avg, val);
		//ofs.open("E:/DATA/Test/density_field/avg.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//ofs.close();
		//kernel_prog->readBuffer(arg_var, val);
		//ofs.open("E:/DATA/Test/density_field/var.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//ofs.close();
		//delete[] val;

		//release buffer
		kernel_prog->releaseMemObject(arg_gavg);
		kernel_prog->releaseMemObject(arg_gvar);

		//density grow
		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		arg_img.kernel_index = kernel2_index0;
		arg_img.index = 0;
		kernel2_prog->setKernelArgument(arg_img);
		flvr::Argument arg_label = kernel2_prog->setKernelArgTex3DBuf(kernel2_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		arg_df.kernel_index = kernel2_index0;
		arg_df.index = 2;
		kernel2_prog->setKernelArgument(arg_df);
		arg_avg.kernel_index = kernel2_index0;
		arg_avg.index = 3;
		kernel2_prog->setKernelArgument(arg_avg);
		arg_var.kernel_index = kernel2_index0;
		arg_var.index = 4;
		kernel2_prog->setKernelArgument(arg_var);
		kernel2_prog->setKernelArgBuf(kernel2_index0, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel2_prog->setKernelArgConst(kernel2_index0, 6,
			sizeof(unsigned int), (void*)(&seed));
		kernel2_prog->setKernelArgConst(kernel2_index0, 7,
			sizeof(unsigned int), (void*)(&nx));
		kernel2_prog->setKernelArgConst(kernel2_index0, 8,
			sizeof(unsigned int), (void*)(&ny));
		kernel2_prog->setKernelArgConst(kernel2_index0, 9,
			sizeof(unsigned int), (void*)(&nz));
		kernel2_prog->setKernelArgConst(kernel2_index0, 10,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel2_prog->setKernelArgConst(kernel2_index0, 11,
			sizeof(unsigned int), (void*)(&dnx));
		kernel2_prog->setKernelArgConst(kernel2_index0, 12,
			sizeof(float), (void*)(&tran));
		kernel2_prog->setKernelArgConst(kernel2_index0, 13,
			sizeof(float), (void*)(&scl_ff));
		kernel2_prog->setKernelArgConst(kernel2_index0, 14,
			sizeof(float), (void*)(&grad_ff));
		kernel2_prog->setKernelArgConst(kernel2_index0, 15,
			sizeof(float), (void*)(&density));
		kernel2_prog->setKernelArgConst(kernel2_index0, 16,
			sizeof(float), (void*)(&sscale));
		if (m_use_mask)
			kernel2_prog->setKernelArgTex3D(kernel2_index0, 17,
				CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < iter; ++j)
			kernel2_prog->executeKernel(kernel2_index0, 3, global_size, local_size);

		//read back
		kernel2_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel2_prog->releaseAll();
		kernel_prog->releaseAll(false);

		m_sig_progress();
	}
}

void ComponentGenerator::DistGrow(bool diffuse, int iter,
	float tran, float falloff, int dsize, int max_dist,
	float dist_thresh, float sscale, float dist_strength)
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_dist_field_2d);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_mask)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}

	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_dist_grow_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_mask)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 0
		flvr::Argument arg_img = kernel_prog_dist->setKernelArgTex3D(
			kernel_dist_index0, 0, CL_MEM_READ_ONLY, did);
		flvr::Argument arg_distf = kernel_prog_dist->setKernelArgBuf(
			kernel_dist_index0, 1, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*nx*ny*nz, NULL);
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 5,
			sizeof(int), (void*)(&dsize));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 6,
			sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 7,
			sizeof(float), (void*)(&sscale));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 8,
			sizeof(unsigned char), (void*)(&ini));
		if (m_use_mask)
			kernel_prog_dist->setKernelArgTex3D(kernel_dist_index0, 9,
				CL_MEM_READ_ONLY, mid);
		//kernel 1
		arg_distf.kernel_index = kernel_dist_index1;
		arg_distf.index = 0;
		kernel_prog_dist->setKernelArgument(arg_distf);
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 3,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 4,
			sizeof(unsigned char), (void*)(&ini));
		if (m_use_mask)
			kernel_prog_dist->setKernelArgTex3D(kernel_dist_index1, 7,
				CL_MEM_READ_ONLY, mid);
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		unsigned char nn, re;
		for (int j = 0; j < max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 5,
				sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 6,
				sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
		}
		//debug
		//val = new unsigned char[nx*ny*nz];
		//kernel_prog_dist->readBuffer(arg_distf, val);
		//ofs.open("E:/DATA/Test/density_field/df.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz);
		//delete[] val;
		//ofs.close();

		//grow
		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		arg_img.kernel_index = kernel_index0;
		arg_img.index = 0;
		kernel_prog->setKernelArgument(arg_img);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		arg_distf.kernel_index = kernel_index0;
		arg_distf.index = 2;
		kernel_prog->setKernelArgument(arg_distf);
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 5,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(kernel_index0, 6,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index0, 7,
			sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(kernel_index0, 8,
			sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(kernel_index0, 9,
			sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 10,
			sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 11,
			sizeof(float), (void*)(&sscale));
		kernel_prog->setKernelArgConst(kernel_index0, 12,
			sizeof(float), (void*)(&dist_strength));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index0, 13,
				CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < iter; ++j)
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		kernel_prog_dist->releaseAll(false);

		m_sig_progress();
	}
}

void ComponentGenerator::DistDensityField(
	bool diffuse, int iter, float tran, float falloff,
	int dsize1, int max_dist, float dist_thresh, float dist_strength,
	int dsize2, int wsize, float density, float sscale)
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_dist_field_2d);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_mask)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}
	//prog density
	flvr::KernelProgram* kernel_prog_dens = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_distdens_field_3d);
	if (!kernel_prog_dens)
		return;
	int kernel_dens_index0 = kernel_prog_dens->createKernel("kernel_0");
	int kernel_dens_index1 = kernel_prog_dens->createKernel("kernel_1");
	int kernel_dens_index2 = kernel_prog_dens->createKernel("kernel_2");

	//prog grow
	flvr::KernelProgram* kernel_prog_grow = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_density_grow_3d);
	if (!kernel_prog_grow)
		return;
	int kernel_grow_index0;
	if (m_use_mask)
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_1");
	else
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_0");

	//processing by brick
	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 0
		flvr::Argument arg_img = kernel_prog_dist->setKernelArgTex3D(
			kernel_dist_index0, 0, CL_MEM_READ_ONLY, did);
		flvr::Argument arg_distf = kernel_prog_dist->setKernelArgBuf(
			kernel_dist_index0, 1, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*nx*ny*nz, NULL);
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 5,
			sizeof(int), (void*)(&dsize1));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 6,
			sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 7,
			sizeof(float), (void*)(&sscale));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index0, 8,
			sizeof(unsigned char), (void*)(&ini));
		if (m_use_mask)
			kernel_prog_dist->setKernelArgTex3D(kernel_dist_index0, 9,
				CL_MEM_READ_ONLY, mid);
		//kernel 1
		arg_distf.kernel_index = kernel_dist_index1;
		arg_distf.index = 0;
		kernel_prog_dist->setKernelArgument(arg_distf);
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 3,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 4,
			sizeof(unsigned char), (void*)(&ini));
		if (m_use_mask)
			kernel_prog_dist->setKernelArgTex3D(kernel_dist_index1, 7,
				CL_MEM_READ_ONLY, mid);
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		unsigned char nn, re;
		for (int j = 0; j < max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 5,
				sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setKernelArgConst(kernel_dist_index1, 6,
				sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
		}
		//debug
		//val = new unsigned char[nx*ny*nz];
		//kernel_prog_dist->readBuffer(arg_distf, val);
		//ofs.open("E:/DATA/Test/density_field/df.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz);
		//delete[] val;
		//ofs.close();

		//generate density field arg_densf
		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		gsx = wsize >= nx ? nx : wsize;
		gsy = wsize >= ny ? ny : wsize;
		gsz = wsize >= nz ? nz : wsize;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		//set
		//kernel 0
		arg_img.kernel_index = kernel_dens_index0;
		arg_img.index = 0;
		kernel_prog_dens->setKernelArgument(arg_img);
		arg_distf.kernel_index = kernel_dens_index0;
		arg_distf.index = 1;
		kernel_prog_dens->setKernelArgument(arg_distf);
		flvr::Argument arg_densf = kernel_prog_dens->setKernelArgBuf(
			kernel_dens_index0, 2, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		int nxy = nx * ny;
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 3,
			sizeof(unsigned int), (void*)(&nxy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 4,
			sizeof(unsigned int), (void*)(&nx));
		int dnxy = dnx * dny;
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 5,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 6,
			sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 7,
			sizeof(int), (void*)(&dsize2));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 8,
			sizeof(float), (void*)(&sscale));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index0, 9,
			sizeof(float), (void*)(&dist_strength));
		//kernel 1
		arg_densf.kernel_index = kernel_dens_index1;
		arg_densf.index = 0;
		kernel_prog_dens->setKernelArgument(arg_densf);
		flvr::Argument arg_gavg = kernel_prog_dens->setKernelArgBuf(
			kernel_dens_index1, 1, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		flvr::Argument arg_gvar = kernel_prog_dens->setKernelArgBuf(
			kernel_dens_index1, 2, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 3,
			sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 4,
			sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 5,
			sizeof(unsigned int), (void*)(&gsz));
		int ngxy = ngy * ngx;
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 6,
			sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 7,
			sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 8,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index1, 9,
			sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 2,
			sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 3,
			sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 4,
			sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 5,
			sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 6,
			sizeof(unsigned int), (void*)(&ngy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 7,
			sizeof(unsigned int), (void*)(&ngz));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 8,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(kernel_dens_index2, 9,
			sizeof(unsigned int), (void*)(&dnx));

		//init
		kernel_prog_dens->executeKernel(kernel_dens_index0, 3, global_size, local_size);
		//debug
		//val = new unsigned char[dnx*dny*dnz];
		//kernel_prog_dens->readBuffer(arg_densf, val);
		//ofs.open("E:/DATA/Test/density_field/df.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//delete[] val;
		//ofs.close();
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog_dens->executeKernel(kernel_dens_index1, 3, global_size, local_size);
		//debug
		//val = new unsigned char[ngx*ngy*ngz];
		//kernel_prog->readBuffer(arg_gavg, val);
		//ofs.open("E:/DATA/Test/density_field/arg_gavg.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, ngx*ngy*ngz);
		//ofs.close();
		//kernel_prog->readBuffer(arg_gvar, val);
		//ofs.open("E:/DATA/Test/density_field/arg_gvar.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, ngx*ngy*ngz);
		//ofs.close();
		//delete[] val;
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		flvr::Argument arg_avg = kernel_prog_dens->setKernelArgBuf(
			kernel_dens_index2, 0, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		arg_gavg.kernel_index = kernel_dens_index2;
		arg_gavg.index = 1;
		kernel_prog_dens->setKernelArgument(arg_gavg);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		//compute var
		flvr::Argument arg_var = kernel_prog_dens->setKernelArgBuf(
			kernel_dens_index2, 0, CL_MEM_READ_WRITE |
			CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		arg_gvar.kernel_index = kernel_dens_index2;
		arg_gvar.index = 1;
		kernel_prog_dens->setKernelArgument(arg_gvar);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);

		//debug
		//val = new unsigned char[dnx*dny*dnz];
		//kernel_prog->readBuffer(arg_avg, val);
		//ofs.open("E:/DATA/Test/density_field/avg.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//ofs.close();
		//kernel_prog->readBuffer(arg_var, val);
		//ofs.open("E:/DATA/Test/density_field/var.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, dnx*dny*dnz);
		//ofs.close();
		//delete[] val;

		//release buffer
		kernel_prog_dens->releaseMemObject(arg_gavg);
		kernel_prog_dens->releaseMemObject(arg_gvar);
		kernel_prog_dens->releaseMemObject(arg_distf);

		//distance + density grow
		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		arg_img.kernel_index = kernel_grow_index0;
		arg_img.index = 0;
		kernel_prog_grow->setKernelArgument(arg_img);
		flvr::Argument arg_label = kernel_prog_grow->setKernelArgTex3DBuf(kernel_grow_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		arg_densf.kernel_index = kernel_grow_index0;
		arg_densf.index = 2;
		kernel_prog_grow->setKernelArgument(arg_densf);
		arg_avg.kernel_index = kernel_grow_index0;
		arg_avg.index = 3;
		kernel_prog_grow->setKernelArgument(arg_avg);
		arg_var.kernel_index = kernel_grow_index0;
		arg_var.index = 4;
		kernel_prog_grow->setKernelArgument(arg_var);
		kernel_prog_grow->setKernelArgBuf(kernel_grow_index0, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 6,
			sizeof(unsigned int), (void*)(&seed));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 7,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 8,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 9,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 10,
			sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 11,
			sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 12,
			sizeof(float), (void*)(&tran));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 13,
			sizeof(float), (void*)(&scl_ff));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 14,
			sizeof(float), (void*)(&grad_ff));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 15,
			sizeof(float), (void*)(&density));
		kernel_prog_grow->setKernelArgConst(kernel_grow_index0, 16,
			sizeof(float), (void*)(&sscale));
		if (m_use_mask)
			kernel_prog_grow->setKernelArgTex3D(kernel_grow_index0, 17,
				CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < iter; ++j)
			kernel_prog_grow->executeKernel(kernel_grow_index0, 3, global_size, local_size);

		//read back
		kernel_prog_grow->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog_grow->releaseAll();
		kernel_prog_dist->releaseAll(false);
		kernel_prog_dens->releaseAll(false);

		m_sig_progress();
	}
}

void ComponentGenerator::Cleanup(int iter, unsigned int size_lm)
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_cleanup_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	int kernel_index1;
	int kernel_index2;
	if (m_use_mask)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_3");
		kernel_index1 = kernel_prog->createKernel("kernel_4");
		kernel_index2 = kernel_prog->createKernel("kernel_5");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		unsigned int* size_buffer = 0;

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;
		size_buffer = new unsigned int[data_size];
		memset(size_buffer, 0, label_size);

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBuf(kernel_index0, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			label_size, size_buffer);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index0, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index0, 6,
			sizeof(unsigned int), (void*)(&lenz));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index0, 7,
				CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog->setKernelArgBuf(kernel_index1, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			label_size, size_buffer);
		arg_label.kernel_index = kernel_index1;
		arg_label.index = 1;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index1, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index1, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index1, 6,
			sizeof(unsigned int), (void*)(&lenz));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index1, 7,
				CL_MEM_READ_ONLY, mid);
		//kernel 2
		kernel_prog->setKernelArgTex3D(kernel_index2, 0,
			CL_MEM_READ_ONLY, did);
		kernel_prog->setKernelArgBuf(kernel_index2, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			label_size, size_buffer);
		arg_label.kernel_index = kernel_index2;
		arg_label.index = 2;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index2, 5,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index2, 6,
			sizeof(unsigned int), (void*)(&size_lm));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index2, 7,
				CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < iter; ++j)
		{
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		}

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		delete[] size_buffer;

		m_sig_progress();
	}
}

void ComponentGenerator::ClearBorders()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_clear_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_mask)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index, 0,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&nz));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index, 4,
				CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		m_sig_progress();
	}
}

void ComponentGenerator::FillBorders(float tol)
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_fill_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_mask)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks_id();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_mask && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label = kernel_prog->setKernelArgTex3DBuf(kernel_index, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(float), (void*)(&tol));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(kernel_index, 6,
				CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		m_sig_progress();
	}
}
