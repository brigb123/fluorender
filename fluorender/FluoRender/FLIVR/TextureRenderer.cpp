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

#include <GL/glew.h>
#include <FLIVR/texturebrick.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/Color.h>
#include <FLIVR/Utils.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/VolShader.h>
#include <FLIVR/SegShader.h>
#include <FLIVR/VolCalShader.h>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

namespace FLIVR
{
	bool TextureRenderer::mem_swap_ = true;
	bool TextureRenderer::use_mem_limit_ = false;
	double TextureRenderer::mem_limit_ = 0.0;
	double TextureRenderer::available_mem_ = 0.0;
	double TextureRenderer::large_data_size_ = 0.0;
	int TextureRenderer::force_brick_size_ = 0;
	vector<TexParam> TextureRenderer::tex_pool_;
	bool TextureRenderer::start_update_loop_ = false;
	bool TextureRenderer::done_update_loop_ = true;
	bool TextureRenderer::done_current_chan_ = true;
	int TextureRenderer::total_brick_num_ = 0;
	int TextureRenderer::cur_brick_num_ = 0;
	int TextureRenderer::cur_chan_brick_num_ = 0;
	bool TextureRenderer::clear_chan_buffer_ = true;
	bool TextureRenderer::save_final_buffer_ = true;
	unsigned long TextureRenderer::st_time_ = 0;
	unsigned long TextureRenderer::up_time_ = 100;
	unsigned long TextureRenderer::cor_up_time_ = 100;
	unsigned long TextureRenderer::consumed_time_ = 0;
	bool TextureRenderer::interactive_ = false;
	int TextureRenderer::finished_bricks_ = 0;
	BrickQueue TextureRenderer::brick_queue_(15);
	int TextureRenderer::quota_bricks_ = 0;
	Point TextureRenderer::quota_center_;
	int TextureRenderer::update_order_ = 0;

	TextureRenderer::TextureRenderer(Texture* tex)
		:
		tex_(tex),
		mode_(MODE_NONE),
		sampling_rate_(1.0),
		num_slices_(0),
		irate_(0.5),
		imode_(false),
		blend_framebuffer_resize_(false),
		blend_framebuffer_(0),
		blend_tex_id_(0),
		filter_buffer_resize_(false),
		filter_buffer_(0),
		filter_tex_id_(0),
		fbo_mask_(0),
		fbo_label_(0),
		tex_2d_mask_(0),
		tex_2d_weight1_(0),
		tex_2d_weight2_(0),
		tex_2d_dmap_(0),
		blend_num_bits_(32),
		clear_pool_(false)
	{
		glGenBuffers(1, &m_slices_vbo);
		glGenBuffers(1, &m_slices_ibo);
		glGenVertexArrays(1, &m_slices_vao);
		glGenBuffers(1, &m_quad_vbo);
		glGenVertexArrays(1, &m_quad_vao);
		//glGenBuffers(1, &m_quad_ibo);
	}

	TextureRenderer::TextureRenderer(const TextureRenderer& copy)
		:
		tex_(copy.tex_),
		mode_(copy.mode_),
		sampling_rate_(copy.sampling_rate_),
		num_slices_(0),
		irate_(copy.irate_),
		imode_(copy.imode_),
		blend_framebuffer_resize_(false),
		blend_framebuffer_(0),
		blend_tex_id_(0),
		filter_buffer_resize_(false),
		filter_buffer_(0),
		filter_tex_id_(0),
		fbo_mask_(0),
		fbo_label_(0),
		tex_2d_mask_(0),
		tex_2d_weight1_(0),
		tex_2d_weight2_(0),
		tex_2d_dmap_(0),
		blend_num_bits_(copy.blend_num_bits_),
		clear_pool_(copy.clear_pool_)
	{
		//buffers
		glGenBuffers(1, &m_slices_vbo);
		glGenBuffers(1, &m_slices_ibo);
		glGenVertexArrays(1, &m_slices_vao);
		glGenBuffers(1, &m_quad_vbo);
		glGenVertexArrays(1, &m_quad_vao);
		//glGenBuffers(1, &m_quad_ibo);
	}

	TextureRenderer::~TextureRenderer()
	{
		clear_tex_pool();
		if (glIsFramebuffer(blend_framebuffer_))
			glDeleteFramebuffers(1, &blend_framebuffer_);
		if (glIsTexture(blend_tex_id_))
			glDeleteTextures(1, &blend_tex_id_);
		if (glIsFramebuffer(filter_buffer_))
			glDeleteFramebuffers(1, &filter_buffer_);
		if (glIsTexture(filter_tex_id_))
			glDeleteTextures(1, &filter_tex_id_);

		//buffers
		if (glIsBuffer(m_slices_vbo))
			glDeleteBuffers(1, &m_slices_vbo);
		if (glIsBuffer(m_slices_ibo))
			glDeleteBuffers(1, &m_slices_ibo);
		if (glIsVertexArray(m_slices_vao))
			glDeleteVertexArrays(1, &m_slices_vao);
		if (glIsBuffer(m_quad_vbo))
			glDeleteBuffers(1, &m_quad_vbo);
		if (glIsVertexArray(m_quad_vao))
			glDeleteVertexArrays(1, &m_quad_vao);
	}

	//set the texture for rendering
	void TextureRenderer::set_texture(Texture* tex)
	{
		if (tex_ != tex) 
		{
			tex_ = tex;
			// new texture, flag existing tex id's for deletion.
			clear_pool_ = true;
		}
	}

	//set blending bits. b>8 means 32bit blending
	void TextureRenderer::set_blend_num_bits(int b)
	{
		blend_num_bits_ = b;
	}

	// Pool is static, however it is cleared each time
	// when a texture is deleted
	void TextureRenderer::clear_tex_pool() 
	{
		for(unsigned int i = 0; i < tex_pool_.size(); i++)
		{
			// delete tex object.
			if(glIsTexture(tex_pool_[i].id))
			{
				glDeleteTextures(1, (GLuint*)&tex_pool_[i].id);
				tex_pool_[i].id = 0;
			}
		}
		tex_pool_.clear();
		clear_pool_ = false;
	}

	//resize the fbo texture
	void TextureRenderer::resize()
	{
		blend_framebuffer_resize_ = true;
		filter_buffer_resize_ = true;
	}

	//set the 2d texture mask for segmentation
	void TextureRenderer::set_2d_mask(GLuint id)
	{
		tex_2d_mask_ = id;
	}

	//set 2d weight map for segmentation
	void TextureRenderer::set_2d_weight(GLuint weight1, GLuint weight2)
	{
		tex_2d_weight1_ = weight1;
		tex_2d_weight2_ = weight2;
	}

	//set the 2d texture depth map for rendering shadows
	void TextureRenderer::set_2d_dmap(GLuint id)
	{
		tex_2d_dmap_ = id;
	}

	//timer
	unsigned long TextureRenderer::get_up_time()
	{
		if (interactive_)
			return cor_up_time_;
		else
			return up_time_;
	}

	//set corrected up time according to mouse speed
	void TextureRenderer::set_cor_up_time(int speed)
	{
		//cor_up_time_ = speed;
		if (speed < 10) speed = 10;
		cor_up_time_ = (unsigned long)(log10(1000.0/speed)*up_time_);
	}

	//number of bricks rendered before time is up
	void TextureRenderer::reset_finished_bricks()
	{
		if (finished_bricks_>0 && consumed_time_>0)
		{
			brick_queue_.Push(int(double(finished_bricks_)*double(up_time_)/double(consumed_time_)));
		}
		finished_bricks_ = 0;
	}

	//get the maximum finished bricks in queue
	int TextureRenderer::get_finished_bricks_max()
	{
		int max = 0;
		int temp;
		for (int i=0; i<brick_queue_.GetLimit(); i++)
		{
			temp = brick_queue_.Get(i);
			max = temp>max?temp:max;
		}
		return max;
	}

	//estimate next brick number
	int TextureRenderer::get_est_bricks(int mode)
	{
		double result = 0.0;
		if (mode == 0)
		{
			//mean
			double sum = 0.0;
			for (int i=0; i<brick_queue_.GetLimit(); i++)
				sum += brick_queue_.Get(i);
			result = sum / brick_queue_.GetLimit();
		}
		else if (mode == 1)
		{
			//trend (weighted average)
			double sum = 0.0;
			double weights = 0.0;
			double w;
			for (int i=0; i<brick_queue_.GetLimit(); i++)
			{
				w = (i+1) * (i+1);
				sum += brick_queue_.Get(i) * w;
				weights += w;
			}
			result = sum / weights;
		}
		else if (mode == 2)
		{
			//linear regression
			double sum_xy = 0.0;
			double sum_x = 0.0;
			double sum_y = 0.0;
			double sum_x2 = 0.0;
			double x, y;
			double n = brick_queue_.GetLimit();
			for (int i=0; i<brick_queue_.GetLimit(); i++)
			{
				x = i;
				y = brick_queue_.Get(i);
				sum_xy += x * y;
				sum_x += x;
				sum_y += y;
				sum_x2 += x * x;
			}
			double beta = (sum_xy/n - sum_x*sum_y/n/n)/(sum_x2/n - sum_x*sum_x/n/n);
			result = Max(sum_y/n - beta*sum_x/n + beta*n, 1.0);
		}
		else if (mode == 3)
		{
			//most recently
			result = brick_queue_.GetLast();
		}
		else if (mode ==4)
		{
			//median
			int n0 = 0;
			double sum = 0.0;
			int n = brick_queue_.GetLimit();
			int *sorted_queue = new int[n];
			memset(sorted_queue, 0, n*sizeof(int));
			for (int i=0; i<n; i++)
			{
				sorted_queue[i] = brick_queue_.Get(i);
				if (sorted_queue[i] == 0)
					n0++;
				else
					sum += sorted_queue[i];
				for (int j=i; j>0; j--)
				{
					if (sorted_queue[j] < sorted_queue[j-1])
					{
						sorted_queue[j] = sorted_queue[j]+sorted_queue[j-1];
						sorted_queue[j-1] = sorted_queue[j]-sorted_queue[j-1];
						sorted_queue[j] = sorted_queue[j]-sorted_queue[j-1];
					}
					else
						break;
				}
			}
			if (n0 == 0)
				result = sorted_queue[n/2];
			else if (n0 < n)
				result = sum / (n-n0);
			else
				result = 0.0;
			delete []sorted_queue;
		}

		if (interactive_)
			return Max(1, int(cor_up_time_*result/up_time_));
		else
			return int(result);
	}

	Ray TextureRenderer::compute_view()
	{
		Transform *field_trans = tex_->transform();
		double mvmat[16] =
			{m_mv_mat[0][0], m_mv_mat[0][1], m_mv_mat[0][2], m_mv_mat[0][3],
			 m_mv_mat[1][0], m_mv_mat[1][1], m_mv_mat[1][2], m_mv_mat[1][3],
			 m_mv_mat[2][0], m_mv_mat[2][1], m_mv_mat[2][2], m_mv_mat[2][3],
			 m_mv_mat[3][0], m_mv_mat[3][1], m_mv_mat[3][2], m_mv_mat[3][3]};

		// index space view direction
		Vector v = field_trans->project(Vector(-mvmat[2], -mvmat[6], -mvmat[10]));
		v.safe_normalize();
		Transform mv;
		mv.set_trans(mvmat);
		Point p = field_trans->unproject(mv.unproject(Point(0,0,0)));
		return Ray(p, v);
	}

	Ray TextureRenderer::compute_snapview(double snap)
	{
		Transform *field_trans = tex_->transform();
		double mvmat[16] =
			{m_mv_mat[0][0], m_mv_mat[0][1], m_mv_mat[0][2], m_mv_mat[0][3],
			 m_mv_mat[1][0], m_mv_mat[1][1], m_mv_mat[1][2], m_mv_mat[1][3],
			 m_mv_mat[2][0], m_mv_mat[2][1], m_mv_mat[2][2], m_mv_mat[2][3],
			 m_mv_mat[3][0], m_mv_mat[3][1], m_mv_mat[3][2], m_mv_mat[3][3]};
		
		//snap
		Vector vd;
		if (snap>0.0 && snap<0.5)
		{
			double vdx = -mvmat[2];
			double vdy = -mvmat[6];
			double vdz = -mvmat[10];
			double vdx_abs = fabs(vdx);
			double vdy_abs = fabs(vdy);
			double vdz_abs = fabs(vdz);
			//if (vdx_abs < snap) vdx = 0.0;
			//if (vdy_abs < snap) vdy = 0.0;
			//if (vdz_abs < snap) vdz = 0.0;
			if (vdx_abs<snap-0.1) vdx = 0.0;
			else if (vdx_abs<snap) vdx = (vdx_abs-snap+0.1)*snap*10.0*vdx/vdx_abs;
			if (vdy_abs<snap-0.1) vdy = 0.0;
			else if (vdy_abs<snap) vdy = (vdy_abs-snap+0.1)*snap*10.0*vdy/vdy_abs;
			if (vdz_abs<snap-0.1) vdz = 0.0;
			else if (vdz_abs<snap) vdz = (vdz_abs-snap+0.1)*snap*10.0*vdz/vdz_abs;
			vd = Vector(vdx, vdy, vdz);
			vd.safe_normalize();
		}
		else
			vd = Vector(-mvmat[2], -mvmat[6], -mvmat[10]);

		// index space view direction
		Vector v = field_trans->project(vd);
		v.safe_normalize();
		Transform mv;
		mv.set_trans(mvmat);
		Point p = field_trans->unproject(mv.unproject(Point(0,0,0)));
		return Ray(p, v);
	}

	double TextureRenderer::compute_rate_scale(Vector v)
	{
		//Transform *field_trans = tex_->transform();
		double spcx, spcy, spcz;
		tex_->get_spacings(spcx, spcy, spcz);
		double z_factor = spcz/Max(spcx, spcy);
		Vector n(double(tex_->nx())/double(tex_->nz()), 
			double(tex_->ny())/double(tex_->nz()), 
			z_factor>1.0&&z_factor<100.0?sqrt(z_factor):1.0);

		double e = 0.0001;
		double a, b, c;
		double result;
		double v_len2 = v.length2();
		if (fabs(v.x())>=e && fabs(v.y())>=e && fabs(v.z())>=e)
		{
			a = v_len2 / v.x();
			b = v_len2 / v.y();
			c = v_len2 / v.z();
			result = n.x()*n.y()*n.z()*sqrt(a*a*b*b+b*b*c*c+c*c*a*a)/
				sqrt(n.x()*n.x()*a*a*n.y()*n.y()*b*b+
					 n.x()*n.x()*a*a*n.z()*n.z()*c*c+
					 n.y()*n.y()*b*b*n.z()*n.z()*c*c);
		}
		else if (fabs(v.x())<e && fabs(v.y())>=e && fabs(v.z())>=e)
		{
			b = v_len2 / v.y();
			c = v_len2 / v.z();
			result = n.y()*n.z()*sqrt(b*b+c*c)/
				sqrt(n.y()*n.y()*b*b+n.z()*n.z()*c*c);
		}
		else if (fabs(v.y())<e && fabs(v.x())>=e && fabs(v.z())>=e)
		{
			a = v_len2 / v.x();
			c = v_len2 / v.z();
			result = n.x()*n.z()*sqrt(a*a+c*c)/
				sqrt(n.x()*n.x()*a*a+n.z()*n.z()*c*c);
		}
		else if (fabs(v.z())<e && fabs(v.x())>=e && fabs(v.y())>=e)
		{
			a = v_len2 / v.x();
			b = v_len2 / v.y();
			result = n.x()*n.y()*sqrt(a*a+b*b)/
				sqrt(n.x()*n.x()*a*a+n.y()*n.y()*b*b);
		}
		else if (fabs(v.x())>=e && fabs(v.y())<e && fabs(v.z())<e)
		{
			result = n.x();
		}
		else if (fabs(v.y())>=e && fabs(v.x())<e && fabs(v.z())<e)
		{
			result = n.y();
		}
		else if (fabs(v.z())>=e && fabs(v.x())<e && fabs(v.y())<e)
		{
			result = n.z();
		}
		else
			result = 1.0;

		return result;

	}

	bool TextureRenderer::test_against_view(const BBox &bbox)
	{
		memcpy(mvmat_, glm::value_ptr(m_mv_mat2), 16*sizeof(float));
		memcpy(prmat_, glm::value_ptr(m_proj_mat), 16*sizeof(float));

		Transform mv;
		Transform pr;
		mv.set_trans(mvmat_);
		pr.set_trans(prmat_);

		bool overx = true;
		bool overy = true;
		bool overz = true;
		bool underx = true;
		bool undery = true;
		bool underz = true;
		for (int i = 0; i < 8; i++)
		{
			const Point pold((i&1)?bbox.min().x():bbox.max().x(),
				(i&2)?bbox.min().y():bbox.max().y(),
				(i&4)?bbox.min().z():bbox.max().z());
			const Point p = pr.project(mv.project(pold));
			overx = overx && (p.x() > 1.0);
			overy = overy && (p.y() > 1.0);
			overz = overz && (p.z() > 1.0);
			underx = underx && (p.x() < -1.0);
			undery = undery && (p.y() < -1.0);
			underz = underz && (p.z() < -1.0);
		}

		return !(overx || overy || overz || underx || undery || underz);
	}

	//exposed load brick
	GLint TextureRenderer::load_brick_cl(int c, vector<TextureBrick*> *bricks, int bindex)
	{
		GLint result = -1;
		if (clear_pool_) clear_tex_pool();
		TextureBrick* brick = (*bricks)[bindex];
		int nb = brick->nb(c);
		int nx = brick->nx();
		int ny = brick->ny();
		int nz = brick->nz();
		GLenum textype = brick->tex_type(c);
		int idx = -1;
		for(unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if(tex_pool_[i].brick == brick
				&& glIsTexture(tex_pool_[i].id))
			{
				//found!
				idx = i;
			}
		}
		if(idx != -1) 
		{
			result = tex_pool_[idx].id;
		}
		else //idx == -1
		{
			// allocate new object
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);

			// create new entry
			tex_pool_.push_back(TexParam(c, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size())-1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			result = tex_pool_[idx].id;

			// download texture data
#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			if (ShaderProgram::shaders_supported())
			{
				GLenum format;
				GLint internal_format;
				if (nb < 3)
				{
					internal_format = (brick->tex_type(c)==GL_SHORT||
						brick->tex_type(c)==GL_UNSIGNED_SHORT)?
						GL_R16:GL_R8;
					format = GL_RED;
				}
				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
						brick->tex_type(c), 0);
#ifdef _WIN32
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
						brick->tex_type(c), brick->tex_data(c));
#else
					if (bricks->size() > 1)
					{
						unsigned long long mem_size = (unsigned long long)nx*
							(unsigned long long)ny*(unsigned long long)nz*nb;
						unsigned char* temp = new unsigned char[mem_size];
						unsigned char* tp = (unsigned char*)(brick->tex_data(c));
						for (unsigned int k=0; k<nz; ++k)
						{
							for (unsigned int j=0; j<ny; ++j)
							{
								memcpy(temp, tp, nx*nb);
								tp += brick->sx()*nb;
							}
							tp += brick->sx()*brick->sy()*nb;
						}
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), (GLvoid*)temp);
						delete []temp;
					}
					else
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), brick->tex_data(c));
#endif
				}
			}

			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		return result;
	}

	GLint TextureRenderer::load_brick(int unit, int c,
		vector<TextureBrick*> *bricks, int bindex,
		GLint filter, bool compression, int mode)
	{
		GLint result = -1;

		if (clear_pool_) clear_tex_pool();
		TextureBrick* brick = (*bricks)[bindex];
		int idx;

		if (c < 0 || c >= TEXTURE_MAX_COMPONENTS)
			return 0;
		if (brick->ntype(c) != TextureBrick::TYPE_INT)
			return 0;

		glActiveTexture(GL_TEXTURE0+unit);

		int nb = brick->nb(c);
		int nx = brick->nx();
		int ny = brick->ny();
		int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		idx = -1;
		for(unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if(tex_pool_[i].id != 0
				&& tex_pool_[i].brick == brick
				&& tex_pool_[i].comp == c
				&& nx == tex_pool_[i].nx
				&& ny == tex_pool_[i].ny
				&& nz == tex_pool_[i].nz
				&& nb == tex_pool_[i].nb
				&& textype == tex_pool_[i].textype
				&& glIsTexture(tex_pool_[i].id))
			{
				//found!
				idx = i;
			}
		}

		if(idx != -1) 
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
		} 
		else //idx == -1
		{
			//see if it needs to free some memory
			if (mem_swap_)
				check_swap_memory(brick, c);

			// allocate new object
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);

			// create new entry
			tex_pool_.push_back(TexParam(c, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size())-1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

			// download texture data
#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			if (ShaderProgram::shaders_supported())
			{
				GLenum format;
				GLint internal_format;
				if (nb < 3)
				{
					if (compression && GLEW_ARB_texture_compression_rgtc &&
						brick->ntype(c)==TextureBrick::TYPE_INT)
						internal_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
					else
						internal_format = (brick->tex_type(c)==GL_SHORT||
							brick->tex_type(c)==GL_UNSIGNED_SHORT)?
							GL_R16:GL_R8;
					format = GL_RED;
				}
				else
				{
					if (compression && GLEW_ARB_texture_compression_rgtc &&
						brick->ntype(c)==TextureBrick::TYPE_INT)
						internal_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
					else
						internal_format = (brick->tex_type(c)==GL_SHORT||
							brick->tex_type(c)==GL_UNSIGNED_SHORT)?
							GL_RGBA16UI:GL_RGBA8UI;
					format = GL_RGBA;
				}

				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
						brick->tex_type(c), 0);
#ifdef _WIN32
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
						brick->tex_type(c), brick->tex_data(c));
#else
					if (bricks->size() > 1)
					{
						unsigned long long mem_size = (unsigned long long)nx*
							(unsigned long long)ny*(unsigned long long)nz*nb;
						unsigned char* temp = new unsigned char[mem_size];
						unsigned char* tp = (unsigned char*)(brick->tex_data(c));
						for (unsigned int k=0; k<nz; ++k)
						{
							for (unsigned int j=0; j<ny; ++j)
							{
								memcpy(temp, tp, nx*nb);
								tp += brick->sx()*nb;
							}
							tp += brick->sx()*brick->sy()*nb;
						}
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), (GLvoid*)temp);
						delete []temp;
					}
					else
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), brick->tex_data(c));
#endif
					if (mem_swap_)
					{
						double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c)/1.04e6;
						available_mem_ -= new_mem;
					}
				}
			}

#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		if (mem_swap_ &&
			start_update_loop_ &&
			!done_update_loop_)
		{
			if (!brick->drawn(mode))
			{
				brick->set_drawn(mode, true);
				cur_brick_num_++;
				cur_chan_brick_num_++;
			}
		}

		glActiveTexture(GL_TEXTURE0);

		return result;
	}

	//search for or create the mask texture in the texture pool
	GLint TextureRenderer::load_brick_mask(vector<TextureBrick*> *bricks, int bindex, GLint filter, bool compression, int unit)
	{
		GLint result = -1;

		TextureBrick* brick = (*bricks)[bindex];
		int c = brick->nmask();

		glActiveTexture(GL_TEXTURE0+(unit>0?unit:c));

		int nb = brick->nb(c);
		int nx = brick->nx();
		int ny = brick->ny();
		int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		int idx = -1;
		for(unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if(tex_pool_[i].id != 0
				&& tex_pool_[i].brick == brick
				&& tex_pool_[i].comp == c
				&& nx == tex_pool_[i].nx
				&& ny == tex_pool_[i].ny
				&& nz == tex_pool_[i].nz
				&& nb == tex_pool_[i].nb
				&& textype == tex_pool_[i].textype
				&& glIsTexture(tex_pool_[i].id))
			{
				//found!
				idx = i;
			}
		}

		if(idx != -1) 
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
		} 
		else //idx == -1
		{
			////! try again to find the matching texture object
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);

			tex_pool_.push_back(TexParam(c, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size())-1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;

			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

			// download texture data
#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			if (ShaderProgram::shaders_supported())
			{
				GLint internal_format = GL_R8;
				GLenum format = (nb == 1 ? GL_RED : GL_RGBA);
				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
					brick->tex_type(c), 0);
#ifdef _WIN32
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
						brick->tex_type(c), brick->tex_data(c));
#else
					if (bricks->size() > 1)
					{
						unsigned long long mem_size = (unsigned long long)nx*
							(unsigned long long)ny*(unsigned long long)nz*nb;
						unsigned char* temp = new unsigned char[mem_size];
						unsigned char* tp = (unsigned char*)(brick->tex_data(c));
						for (unsigned int k=0; k<nz; ++k)
						{
							for (unsigned int j=0; j<ny; ++j)
							{
								memcpy(temp, tp, nx*nb);
								tp += brick->sx()*nb;
							}
							tp += brick->sx()*brick->sy()*nb;
						}
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), (GLvoid*)temp);
						delete []temp;
					}
					else
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), brick->tex_data(c));
#endif
				}
			}

#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		
		glActiveTexture(GL_TEXTURE0);
		
		return result;
	}

	//search for or create the label texture in the texture pool
	GLint TextureRenderer::load_brick_label(vector<TextureBrick*> *bricks, int bindex)
	{
		GLint result = -1;

		TextureBrick* brick = (*bricks)[bindex];
		int c = brick->nlabel();

		glActiveTexture(GL_TEXTURE0+c);

		int nb = brick->nb(c);
		int nx = brick->nx();
		int ny = brick->ny();
		int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		int idx = -1;
		for(unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if(tex_pool_[i].id != 0
				&& tex_pool_[i].brick == brick
				&& tex_pool_[i].comp == c
				&& nx == tex_pool_[i].nx
				&& ny == tex_pool_[i].ny
				&& nz == tex_pool_[i].nz
				&& nb == tex_pool_[i].nb
				&& textype == tex_pool_[i].textype
				&& glIsTexture(tex_pool_[i].id))
			{
				//found!
				idx = i;
			}
		}

		if(idx != -1) 
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} 
		else //idx == -1
		{
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);
			tex_pool_.push_back(TexParam(c, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size())-1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;

			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// download texture data
#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

			if (ShaderProgram::shaders_supported())
			{
				GLenum format = GL_RED_INTEGER;
				GLint internal_format = GL_R32UI;
				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
					brick->tex_type(c), NULL);
#ifdef _WIN32
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
						brick->tex_type(c), brick->tex_data(c));
#else
					if (bricks->size() > 1)
					{
						unsigned long long mem_size = (unsigned long long)nx*
							(unsigned long long)ny*(unsigned long long)nz*nb;
						unsigned char* temp = new unsigned char[mem_size];
						unsigned char* tp = (unsigned char*)(brick->tex_data(c));
						for (unsigned int k=0; k<nz; ++k)
						{
							for (unsigned int j=0; j<ny; ++j)
							{
								memcpy(temp, tp, nx*nb);
								tp += brick->sx()*nb;
							}
							tp += brick->sx()*brick->sy()*nb;
						}
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), (GLvoid*)temp);
						delete []temp;
					}
					else
						glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
							brick->tex_type(c), brick->tex_data(c));
#endif
				}
			}

#ifdef _WIN32
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
#endif
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		glActiveTexture(GL_TEXTURE0);

		return result;
	}

	bool TextureRenderer::brick_sort(const BrickDist& bd1, const BrickDist& bd2)
	{
		return bd1.dist > bd2.dist;
	}

	void TextureRenderer::check_swap_memory(TextureBrick* brick, int c)
	{
		unsigned int i;
		double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c)/1.04e6;

		if (use_mem_limit_)
		{
			if (available_mem_ >= new_mem)
				return;
		}
		else
		{
			GLenum error = glGetError();
			GLint mem_info[4] = {0, 0, 0, 0};
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem_info);
			error = glGetError();
			if (error == GL_INVALID_ENUM)
			{
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, mem_info);
				error = glGetError();
				if (error == GL_INVALID_ENUM)
					return;
			}

			//available memory size in MB
			available_mem_ = mem_info[0]/1024.0;
			if (available_mem_ >= new_mem)
				return;
		}

		vector<BrickDist> bd_list;
		BrickDist bd;
		//generate a list of bricks and their distances to the new brick
		for (i=0; i<tex_pool_.size(); i++)
		{
			bd.index = i;
			bd.brick = tex_pool_[i].brick;
			//calculate the distance
			bd.dist = brick->bbox().distance(bd.brick->bbox());
			bd_list.push_back(bd);
		}

		//release bricks far away
		double est_avlb_mem = available_mem_;
		if (bd_list.size() > 0)
		{
			//sort from farthest to closest
			std::sort(bd_list.begin(), bd_list.end(), TextureRenderer::brick_sort);

			//remove
			for (i=0; i<bd_list.size(); i++)
			{
				TextureBrick* btemp = bd_list[i].brick;
				tex_pool_[bd_list[i].index].delayed_del = true;
				double released_mem = btemp->nx()*btemp->ny()*btemp->nz()*btemp->nb(c)/1.04e6;
				est_avlb_mem += released_mem;
				if (est_avlb_mem >= new_mem)
					break;
			}

			//delete from pool
			for (int j=int(tex_pool_.size()-1); j>=0; j--)
			{
				if (tex_pool_[j].delayed_del &&
					glIsTexture(tex_pool_[j].id))
				{
					glDeleteTextures(1, (GLuint*)&tex_pool_[j].id);
					tex_pool_.erase(tex_pool_.begin()+j);
				}
			}

			if (use_mem_limit_)
				available_mem_ = est_avlb_mem;
		}
	}

	void TextureRenderer::draw_view_quad(double d)
	{
		float points[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, float(d),
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, float(d),
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, float(d),
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, float(d)};

		glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, points, GL_STREAM_DRAW);

		glBindVertexArray(m_quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)12);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	void TextureRenderer::draw_polygons(vector<float>& vertex,
		vector<uint32_t>& triangle_verts)
	{
		if (vertex.empty() || triangle_verts.empty())
			return;

		//link to the new data
		glBindBuffer(GL_ARRAY_BUFFER, m_slices_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_slices_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*triangle_verts.size(), 
			&triangle_verts[0], GL_STREAM_DRAW);
		
		glBindVertexArray(m_slices_vao);
		//now draw it
		glBindBuffer(GL_ARRAY_BUFFER, m_slices_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)12);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_slices_ibo);
		glDrawElements(GL_TRIANGLES, triangle_verts.size(), GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void TextureRenderer::draw_polygons_wireframe(vector<float>& vertex,
		vector<uint32_t>& index, vector<uint32_t>& size)
	{
		if (vertex.empty() || index.empty())
			return;

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindBuffer(GL_ARRAY_BUFFER, m_slices_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_slices_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*index.size(), 
			&index[0], GL_STREAM_DRAW);

		glBindVertexArray(m_slices_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_slices_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const GLvoid*)12);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_slices_ibo);
		unsigned int idx_num;
		for (unsigned int i=0, k=0; i<size.size(); ++i)
		{
			idx_num = (size[i]-2)*3;
			glDrawElements(GL_TRIANGLES, idx_num, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*> (k));
			k += idx_num*4;
		}
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//bind 2d mask for segmentation
	void TextureRenderer::bind_2d_mask()
	{
		if (ShaderProgram::shaders_supported() && glActiveTexture)
		{
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, tex_2d_mask_);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	//bind 2d weight map for segmentation
	void TextureRenderer::bind_2d_weight()
	{
		if (ShaderProgram::shaders_supported() && glActiveTexture)
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, tex_2d_weight1_);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, tex_2d_weight2_);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	//bind 2d depth map for rendering shadows
	void TextureRenderer::bind_2d_dmap()
	{
		if (ShaderProgram::shaders_supported() && glActiveTexture)
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, tex_2d_dmap_);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	//release texture
	void TextureRenderer::release_texture(int unit, GLenum taget)
	{
		if (ShaderProgram::shaders_supported() && glActiveTexture)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(taget, 0);
			glActiveTexture(GL_TEXTURE0);
		}
	}

} // namespace FLIVR




