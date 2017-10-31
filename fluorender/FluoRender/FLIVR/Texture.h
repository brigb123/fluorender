//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#ifndef SLIVR_Texture_h
#define SLIVR_Texture_h

#include <vector>
#include "Transform.h"
#include "TextureBrick.h"
#include "Utils.h"

namespace FLIVR
{
	using namespace std;

	class Transform;

	class Texture 
	{
	public:
		static size_t mask_undo_num_;
		Texture();
		virtual ~Texture();

		int get_build_max_tex_size() { return build_max_tex_size_; }
		void set_brick_size(int size) { brick_size_ = size; }
		void build(Nrrd* val, Nrrd* grad,
			double vmn, double vmx,
			double gmn, double gmx);

		inline Vector res() { return Vector(nx_, ny_, nz_); }
		inline int nx() { return nx_; }
		inline int ny() { return ny_; }
		inline int nz() { return nz_; }

		//bricks
		inline int bszx() { return bszx_; }
		inline int bszy() { return bszy_; }
		inline int bszz() { return bszz_; }
		inline int bnx() { return bnx_; }
		inline int bny() { return bny_; }
		inline int bnz() { return bnz_; }
		//get neighbor id
		inline unsigned int negxid(unsigned int id);
		inline unsigned int negyid(unsigned int id);
		inline unsigned int negzid(unsigned int id);
		inline unsigned int posxid(unsigned int id);
		inline unsigned int posyid(unsigned int id);
		inline unsigned int poszid(unsigned int id);
		//get brick id by index
		inline unsigned int get_brick_id(unsigned long long index);
		TextureBrick* get_brick(unsigned int bid);

		inline int nc() { return nc_; }
		inline int nb(int i)
		{
			assert(i >= 0 && i < TEXTURE_MAX_COMPONENTS);
			return nb_[i];
		}
		inline int nmask() { return nmask_; }
		inline int nlabel() { return nlabel_; }

		inline void set_size(int nx, int ny, int nz, int nc, int* nb) 
		{
			nx_ = nx; ny_ = ny; nz_ = nz; nc_ = nc;
			for(int c = 0; c < nc_; c++)
			{
				nb_[c] = nb[c];
			}
			if (nc==1)
			{
				ntype_[0] = TYPE_INT;
			}
			else if (nc>1)
			{
				ntype_[0] = TYPE_INT_GRAD;
				ntype_[1] = TYPE_GM;
			}
		}

		//! Interface that does not expose FLIVR::BBox.
		inline 
			void get_bounds(double &xmin, double &ymin, double &zmin,
			double &xmax, double &ymax, double &zmax) const 
		{
			BBox b;
			get_bounds(b);
			xmin = b.min().x();
			ymin = b.min().y();
			zmin = b.min().z();

			xmax = b.max().x();
			ymax = b.max().y();
			zmax = b.max().z();
		}

		inline 
			void get_bounds(BBox &b) const 
		{
			b.extend(transform_.project(bbox_.min()));
			b.extend(transform_.project(bbox_.max()));
		}

		inline BBox *bbox() { return &bbox_; }
		inline void set_bbox(BBox bbox) { bbox_ = bbox; }
		inline Transform *transform() { return &transform_; }
		inline void set_transform(Transform tform) { transform_ = tform; }

		// get sorted bricks
		vector<TextureBrick*>* get_sorted_bricks(
			Ray& view, bool is_orthographic = false);
		//get closest bricks
		vector<TextureBrick*>* get_closest_bricks(
			Point& center, int quota, bool skip,
			Ray& view, bool is_orthographic = false);
		//set sort bricks
		void set_sort_bricks() {sort_bricks_ = true;}
		void reset_sort_bricks() {sort_bricks_ = false;}
		bool get_sort_bricks() {return sort_bricks_;}
		// load the bricks independent of the view
		vector<TextureBrick*>* get_bricks();
		int get_brick_num() {return int(bricks_.size());}
		//quota bricks
		vector<TextureBrick*>* get_quota_bricks();

		inline int nlevels(){ return int(bricks_.size()); }

		inline double vmin() const { return vmin_; }
		inline double vmax() const { return vmax_; }
		inline double gmin() const { return gmin_; }
		inline double gmax() const { return gmax_; }
		inline void set_minmax(double vmin, double vmax, double gmin, double gmax)
		{vmin_ = vmin; vmax_ = vmax; gmin_ = gmin; gmax_ = gmax;}

		void set_spacings(double x, double y, double z);
		void get_spacings(double &x, double &y, double &z)
		{x = spcx_; y = spcy_; z = spcz_;}

		// Creator of the brick owns the nrrd memory.
		void set_nrrd(Nrrd* data, int index);
		Nrrd* get_nrrd(int index)
		{if (index>=0&&index<TEXTURE_MAX_COMPONENTS) return data_[index]; else return 0;}
		int get_max_tex_comp()
		{return TEXTURE_MAX_COMPONENTS;}
		bool trim_mask_undos_head();
		bool trim_mask_undos_tail();
		bool get_undo();
		bool get_redo();
		void set_mask(void* mask_data);
		void push_mask();
		void mask_undos_forward();
		void mask_undos_backward();
		void clear_undos();

		//add one more texture component as the volume mask
		bool add_empty_mask();
		//add one more texture component as the labeling volume
		bool add_empty_label();

		//get priority brick number
		inline void set_use_priority(bool value) {use_priority_ = value;}
		inline bool get_use_priority() {return use_priority_;}
		inline int get_n_p0()
		{if (use_priority_) return n_p0_; else return int(bricks_.size());}

	protected:
		void build_bricks(vector<TextureBrick*> &bricks,
			int nx, int ny, int nz,
			int nc, int* nb);

		//remember the brick size, as it may change
		int build_max_tex_size_;
		//expected brick size, 0: ignored
		int brick_size_;
		//! data carved up to texture memory sized chunks.
		vector<TextureBrick*>						bricks_;
		//for limited number of bricks during interactions
		vector<TextureBrick*>						quota_bricks_;
		//sort texture brick
		bool sort_bricks_;
		//! data size
		int											nx_;
		int											ny_;
		int											nz_;
		//brick size, planned
		int											bszx_;
		int											bszy_;
		int											bszz_;
		//brick num
		int											bnx_;
		int											bny_;
		int											bnz_;
		//! number of components currently used.
		int											nc_; 
		//type of all the components
		enum CompType
		{
			TYPE_NONE=0, TYPE_INT, TYPE_INT_GRAD, TYPE_GM, TYPE_MASK, TYPE_LABEL
		};
		CompType									ntype_[TEXTURE_MAX_COMPONENTS];
		//the index of current mask
		int											nmask_;
		//the index of current label
		int											nlabel_;
		//! bytes per texel for each component.
		int											nb_[TEXTURE_MAX_COMPONENTS];
		//! data tform
		Transform									transform_; 
		double										vmin_;
		double										vmax_;
		double										gmin_;
		double										gmax_;
		//! data bbox
		BBox										bbox_; 
		//! spacings
		double										spcx_;
		double										spcy_;
		double										spcz_;
		//priority
		bool use_priority_;
		int n_p0_;

		//actual data
		Nrrd* data_[TEXTURE_MAX_COMPONENTS];
		//undos for mask
		vector<void*> mask_undos_;
		int mask_undo_pointer_;
	};

	inline unsigned int Texture::negxid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + y * bnx_ + x - 1;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::negyid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + (y - 1) * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::negzid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = (z - 1) * bnx_ * bny_ + y * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::posxid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + y * bnx_ + x + 1;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::posyid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + (y + 1) * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::poszid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = (z + 1) * bnx_ * bny_ + y * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	inline unsigned int Texture::get_brick_id(unsigned long long index)
	{
		int x, y, z;
		z = index / (nx_ * ny_);
		y = index % (nx_ * ny_);
		x = y % nx_;
		y = y / nx_;
		//get brick indices
		x = x / (bszx_-1);
		y = y / (bszy_-1);
		z = z / (bszz_-1);
		return z * bnx_ * bny_ + y * bnx_ + x;
	}

	inline TextureBrick* Texture::get_brick(unsigned int bid)
	{
		for (size_t i=0; i<bricks_.size(); ++i)
		{
			if (bricks_[i]->get_id() == bid)
				return bricks_[i];
		}
		return 0;
	}

} // namespace FLIVR

#endif // Volume_Texture_h
