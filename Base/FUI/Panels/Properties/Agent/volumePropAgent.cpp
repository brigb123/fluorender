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

#include "volumePropAgent.hpp"

#include <Panels/Properties/propertiesPanel.hpp>
/*
#include <Fui/VolumePropPanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>
*/

VolumePropAgent::VolumePropAgent(PropertiesPanel &panel) :
	InterfaceAgent(),
    parentPanel(panel)
{
  double test = 3.22;
  this->setValue("gamma 3d", test);
  double ree = 0.0;

  this->getValue("gamma 3d", ree);
  parentPanel.setPropGammaValue(ree);
  std::cout << ree << std::endl;
}

void VolumePropAgent::setObject(fluo::VolumeData* obj)
{
  InterfaceAgent::setObject(obj);
}

fluo::VolumeData* VolumePropAgent::getObject()
{
  return dynamic_cast<fluo::VolumeData*>(InterfaceAgent::getObject());
}

void VolumePropAgent::UpdateAllSettings()
{
  double dval = 0.0;
  int ival = 0;
  bool bval = false;
  long lval = 0;

	//maximum value
  double max_val;
  getValue("max int", max_val);
  max_val = std::max(255.0, max_val);
  parentPanel.setPropOptionsMaxVal(max_val);

    //set range
    //wxFloatingPointValidator<double>* vald_fp;
    //wxIntegerValidator<unsigned int>* vald_i;

    // volume properties
    // transfer function
    // gamma
//	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_gamma_text->GetValidator()))
    //	vald_fp->SetRange(0.0, 10.0);
    //panel_.m_gamma_sldr->SetValue(int(dval*100.0 + 0.5));
    //str = QString::Format("%.2f", dval);
    //panel_.m_gamma_text->ChangeValue(str);

    // gamma
  getValue("gamma 3d", dval);
  parentPanel.setPropGammaValue(dval);

    //boundary
    //if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_boundary_text->GetValidator()))
//		vald_fp->SetRange(0.0, 1.0);

    //panel.m_boundary_sldr->SetValue(int(dval*2000.0 + 0.5));
    //str = QString::Format("%.4f", dval);
    //panel_.m_boundary_text->ChangeValue(str);

  getValue("extract boundary", dval);
  parentPanel.setPropExtBoundValue(dval);

    /*
    //contrast
	if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_saturation_text->GetValidator()))
		vald_i->SetMin(0);
	getValue("saturation", dval);
	ival = int(dval*panel_.m_max_val + 0.5);
	panel_.m_saturation_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
	panel_.m_saturation_sldr->SetValue(ival);
    panel_.m_saturation_text->ChangeValue(str);

    */

  getValue("saturation", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropSatValue(ival);

    /*
	//left threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_left_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	getValue("low threshold", dval);
	ival = int(dval*panel_.m_max_val + 0.5);
	panel_.m_left_thresh_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
	panel_.m_left_thresh_sldr->SetValue(ival);
    panel_.m_left_thresh_text->ChangeValue(str);
    */

  getValue("low threshold", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropLowThreshValue(ival);


    /*

    //right threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_right_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	getValue("high threshold", dval);
	ival = int(dval*panel_.m_max_val + 0.5);
	panel_.m_right_thresh_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
	panel_.m_right_thresh_sldr->SetValue(ival);
    panel_.m_right_thresh_text->ChangeValue(str);

    */

  getValue("high threshold", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropHighThreshValue(ival);


    /*
	//luminance
	if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_luminance_text->GetValidator()))
		vald_i->SetMin(0);
	getValue("luminance", dval);
	ival = int(dval*panel_.m_max_val + 0.5);
	panel_.m_luminance_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
	panel_.m_luminance_sldr->SetValue(ival);
    panel_.m_luminance_text->ChangeValue(str);
    */

  getValue("luminance", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropLuminValue(ival);

    /*

    //shadow
    if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_shadow_text->GetValidator()))
        vald_fp->SetRange(0.0, 1.0);
    bool shadow_enable;
    getValue("shadow enable", shadow_enable);
    panel_.m_shadow_tool->ToggleTool(VolumePropPanel::ID_ShadowChk, shadow_enable);
    getValue("shadow int", dval);
    panel_.m_shadow_sldr->SetValue(int(dval*100.0 + 0.5));
    str = QString::Format("%.2f", dval);
    panel_.m_shadow_text->ChangeValue(str);

    */

  bool isShadowEnabled;
  getValue("shadow enable", isShadowEnabled);
  getValue("shadow int", dval);

  parentPanel.setPropShadowEnabled(isShadowEnabled);
  parentPanel.setPropShadowValue(dval);

    /*

    //alpha
    if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_alpha_text->GetValidator()))
        vald_i->SetMin(0);
    getValue("alpha", dval);
    ival = int(dval*panel_.m_max_val + 0.5);
    panel_.m_alpha_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
    panel_.m_alpha_sldr->SetValue(ival);
    panel_.m_alpha_text->ChangeValue(str);
    bool alpha_enable;
    getValue("alpha enable", alpha_enable);
    panel_.m_alpha_tool->ToggleTool(VolumePropPanel::ID_AlphaChk, alpha_enable);

    */

  bool isAlphaEnabled;
  getValue("alpha enable", isAlphaEnabled);
  getValue("alpha", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);

  parentPanel.setPropAlphaEnabled(isAlphaEnabled);
  parentPanel.setPropAlphaValue(ival);

    /*

    //smaple rate
    if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_sample_text->GetValidator()))
        vald_fp->SetRange(0.0, 100.0);
    getValue("sample rate", dval);
    panel_.m_sample_sldr->SetValue(dval*10.0);
    str = QString::Format("%.1f", dval);
    panel_.m_sample_text->ChangeValue(str);

    */

  getValue("sample rate", dval);
  parentPanel.setPropSampleValue(dval);

    /*

	//shadings
	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_low_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 10.0);
	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_hi_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 100.0);
	double amb, diff, spec, shine;
	//vd->GetMaterial(amb, diff, spec, shine);
	getValue("mat amb", amb);
	getValue("mat diff", diff);
	getValue("mat spec", spec);
	getValue("mat shine", shine);
	panel_.m_low_shading_sldr->SetValue(amb*100.0);
    str = QString::Format("%.2f", amb);
	panel_.m_low_shading_text->ChangeValue(str);
	panel_.m_hi_shading_sldr->SetValue(shine*10.0);
    str = QString::Format("%.2f", shine);
	panel_.m_hi_shading_text->ChangeValue(str);
	bool shading_enable;
	getValue("shading enable", shading_enable);
	panel_.m_shade_tool->ToggleTool(VolumePropPanel::ID_ShadingEnableChk, shading_enable);

    */
  double amb = 0.0;
  double diff = 0.0;
  double spec = 0.0;
  double shine = 0.0;

  bool isShadeEnabled;

  getValue("mat amb", amb);
  getValue("mat diff", diff);
  getValue("mat spec", spec);
  getValue("mat shine", shine);
  getValue("shading enable", isShadeEnabled);

  parentPanel.setPropLowShaderValue(amb);
  parentPanel.setPropHighShaderValue(shine);
  parentPanel.setPropShaderEnabled(isShadeEnabled);
    /*


    //colormap
    //double low, high;
    //vd->GetColormapValues(low, high);
    //low
    if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_colormap_low_value_text->GetValidator()))
        vald_i->SetMin(0);
    getValue("colormap low", dval);
    ival = int(dval*panel_.m_max_val + 0.5);
    panel_.m_colormap_low_value_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
    panel_.m_colormap_low_value_sldr->SetValue(ival);
    panel_.m_colormap_low_value_text->ChangeValue(str);
    //high
    if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_colormap_high_value_text->GetValidator()))
        vald_i->SetMin(0);
    getValue("colormap high", dval);
    ival = int(dval*panel_.m_max_val + 0.5);
    panel_.m_colormap_high_value_sldr->SetRange(0, int(panel_.m_max_val));
    str = QString::Format("%d", ival);
    panel_.m_colormap_high_value_sldr->SetValue(ival);
    panel_.m_colormap_high_value_text->ChangeValue(str);
    //colormap
    getValue("colormap type", lval);
    panel_.m_colormap_combo->SetSelection(lval);
    getValue("colormap proj", lval);
    panel_.m_colormap_combo2->SetSelection(lval);
    //mode
    getValue("colormap mode", lval);
    bool colormap_enable = lval == 1;
    panel_.m_colormap_tool->ToggleTool(VolumePropPanel::ID_ColormapEnableChk, colormap_enable);
    */

  getValue("colormap low", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropLowColorModeValue(ival);

  getValue("colormap high", dval);
  ival = static_cast<int>(dval*parentPanel.getPropOptionsMaxVal() + 0.5);
  parentPanel.setPropHighColorModeValue(ival);

    /*

    //color
    fluoTYPE::Color c;
    getValue("color", c);
    wxColor wxc((unsigned char)(c.r() * 255 + 0.5),
        (unsigned char)(c.g() * 255 + 0.5),
        (unsigned char)(c.b() * 255 + 0.5));
    panel_.m_color_text->ChangeValue(QString::Format("%d , %d , %d",
        wxc.Red(), wxc.Green(), wxc.Blue()));
    panel_.m_color_btn->SetColour(wxc);
    getValue("sec color", c);
    wxc = wxColor((unsigned char)(c.r() * 255 + 0.5),
        (unsigned char)(c.g() * 255 + 0.5),
        (unsigned char)(c.b() * 255 + 0.5));
    panel_.m_color2_text->ChangeValue(QString::Format("%d , %d , %d",
        wxc.Red(), wxc.Green(), wxc.Blue()));
    panel_.m_color2_btn->SetColour(wxc);

    */
  FLTYPE::Color c;
  getValue("color", c);
  QColor qColor(static_cast<int>(c.r() * 255 + 0.5),
                    static_cast<int>(c.g() * 255 + 0.5),
                    static_cast<int>(c.b() * 255 + 0.5));

  getValue("sec color", c);
  qColor = QColor(static_cast<int>(c.r() * 255 + 0.5),
                    static_cast<int>(c.g() * 255 + 0.5),
                    static_cast<int>(c.b() * 255 + 0.5));

    // TODO: Find out where these are all changed.

    /*

	//spacings
	double spcx, spcy, spcz;
	//vd->GetBaseSpacings(spcx, spcy, spcz);
	getValue("spc x", spcx);
	getValue("spc y", spcy);
	getValue("spc z", spcz);
	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_x_text->GetValidator()))
		vald_fp->SetMin(0.0);
    str = QString::Format("%.3f", spcx);
	panel_.m_space_x_text->ChangeValue(str);
	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_y_text->GetValidator()))
		vald_fp->SetMin(0.0);
    str = QString::Format("%.3f", spcy);
	panel_.m_space_y_text->ChangeValue(str);
	if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_z_text->GetValidator()))
		vald_fp->SetMin(0.0);
    str = QString::Format("%.3f", spcz);
	panel_.m_space_z_text->ChangeValue(str);

	//legend
	getValue("legend", bval);
	panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_LegendChk, bval);

	//interpolate
	getValue("interpolate", bval);
	panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_InterpolateChk, bval);
	if (bval)
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InterpolateChk,
			wxGetBitmapFromMemory(interpolate));
	else
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InterpolateChk,
			wxGetBitmapFromMemory(interpolate_off));

	//sync group
	//if (m_group)
	//	m_sync_group = m_group->GetVolumeSyncProp();
    //bool sync = testSyncParentValue("gamma 3d");
    //panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_SyncGroupChk, sync);


	//inversion
	getValue("invert", bval);
	panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_InvChk, bval);
	if (bval)
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InvChk,
			wxGetBitmapFromMemory(invert));
	else
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InvChk,
			wxGetBitmapFromMemory(invert_off));

	//MIP
	getValue("mip mode", lval);
	bool mip_enable = lval == 1;
	panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_MipChk, colormap_enable);
	if (mip_enable)
		panel_.m_threh_st->SetLabel("Shade Threshold : ");
	else
		panel_.m_threh_st->SetLabel("Threshold : ");

	//noise reduction
	getValue("noise redct", bval);
	panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_NRChk, bval);
	if (bval)
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_NRChk,
			wxGetBitmapFromMemory(smooth));
	else
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_NRChk,
			wxGetBitmapFromMemory(smooth_off));

	//blend mode
	getValue("blend mode", lval);
	if (lval == 2)
	{
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_DepthChk, true);
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_DepthChk, wxGetBitmapFromMemory(depth));
	}
	else
	{
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_DepthChk, false);
		panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_DepthChk, wxGetBitmapFromMemory(depth_off));
	}

	if (alpha_enable)
		panel_.EnableAlpha();
	else
		panel_.DisableAlpha();
	if (shading_enable)
		panel_.EnableShading();
	else
		panel_.DisableShading();
	if (shadow_enable)
		panel_.EnableShadow();
	else
		panel_.DisableShadow();
	if (colormap_enable)
		panel_.EnableColormap();
	else
		panel_.DisableColormap();
	if (mip_enable)
		panel_.EnableMip();
	else
		panel_.DisableMip();

    //panel_.Layout();
    */
}

void VolumePropAgent::OnLuminanceChanged(fluo::Event& event)
{
    /*
	double luminance;
	getValue("luminance", luminance);
	int ival = int(luminance*panel_.m_max_val + 0.5);
	panel_.m_luminance_sldr->SetRange(0, int(panel_.m_max_val));
    QString str = QString::Format("%d", ival);
	panel_.m_luminance_sldr->SetValue(ival);
    panel_.m_luminance_text->ChangeValue(str);
    */
}

void VolumePropAgent::OnColorChanged(fluo::Event& event)
{
    /*
    fluoTYPE::Color color;
	getValue("color", color);
	wxColor wxc((unsigned char)(color.r() * 255 + 0.5),
		(unsigned char)(color.g() * 255 + 0.5),
		(unsigned char)(color.b() * 255 + 0.5));
    panel_.m_color_text->ChangeValue(QString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
    panel_.m_color_btn->SetColour(wxc);
    */
}