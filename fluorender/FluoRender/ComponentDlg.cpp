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
#include "ComponentDlg.h"
#include "VRenderFrame.h"
#include "Components/CompGenerator.h"
#include "Components/CompSelector.h"
#include "Cluster/dbscan.h"
#include "Cluster/kmeans.h"
#include "Cluster/exmax.h"
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <limits>
#include <string>
#include <cctype>
#include <set>
#include <fstream>

BEGIN_EVENT_TABLE(ComponentDlg, wxPanel)
	//EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, ComponentDlg::OnPaneChange)
	//load save settings
	EVT_BUTTON(ID_LoadSettingsBtn, ComponentDlg::OnLoadSettings)
	EVT_BUTTON(ID_SaveSettingsBtn, ComponentDlg::OnSaveSettings)
	EVT_BUTTON(ID_SaveasSettingsBtn, ComponentDlg::OnSaveasSettings)
	//comp gen page
	EVT_COMMAND_SCROLL(ID_BasicIterSldr, ComponentDlg::OnBasicIterSldr)
	EVT_TEXT(ID_BasicIterText, ComponentDlg::OnBasicIterText)
	EVT_COMMAND_SCROLL(ID_BasicThreshSldr, ComponentDlg::OnBasicThreshSldr)
	EVT_TEXT(ID_BasicThreshText, ComponentDlg::OnBasicThreshText)
	//distance field
	EVT_CHECKBOX(ID_BasicUseDistFieldCheck, ComponentDlg::OnBasicUseDistFieldCheck)
	EVT_COMMAND_SCROLL(ID_BasicDistStrengthSldr, ComponentDlg::OnBasicDistStrengthSldr)
	EVT_TEXT(ID_BasicDistStrengthText, ComponentDlg::OnBasicDistStrengthText)
	EVT_COMMAND_SCROLL(ID_BasicMaxDistSldr, ComponentDlg::OnBasicMaxDistSldr)
	EVT_TEXT(ID_BasicMaxDistText, ComponentDlg::OnBasicMaxDistText)
	EVT_COMMAND_SCROLL(ID_BasicDistThreshSldr, ComponentDlg::OnBasicDistThreshSldr)
	EVT_TEXT(ID_BasicDistThreshText, ComponentDlg::OnBasicDistThreshText)
	//diffusion
	EVT_CHECKBOX(ID_BasicDiffCheck, ComponentDlg::OnBasicDiffCheck)
	EVT_COMMAND_SCROLL(ID_BasicFalloffSldr, ComponentDlg::OnBasicFalloffSldr)
	EVT_TEXT(ID_BasicFalloffText, ComponentDlg::OnBasicFalloffText)
	EVT_CHECKBOX(ID_BasicSizeCheck, ComponentDlg::OnBasicSizeCheck)
	EVT_COMMAND_SCROLL(ID_BasicSizeSldr, ComponentDlg::OnBasicSizeSldr)
	EVT_TEXT(ID_BasicSizeText, ComponentDlg::OnBasicSizeText)
	//density
	EVT_CHECKBOX(ID_BasicDensityCheck, ComponentDlg::OnBasicDensityCheck)
	EVT_COMMAND_SCROLL(ID_BasicDensitySldr, ComponentDlg::OnBasicDensitySldr)
	EVT_TEXT(ID_BasicDensityText, ComponentDlg::OnBasicDensityText)
	EVT_COMMAND_SCROLL(ID_BasicDensityWindowSizeSldr, ComponentDlg::OnBasicDensityWindowSizeSldr)
	EVT_TEXT(ID_BasicDensityWindowsSizeText, ComponentDlg::OnBasicDensityWindowSizeText)
	EVT_COMMAND_SCROLL(ID_BasicDensityStatsSizeSldr, ComponentDlg::OnBasicDensityStatsSizeSldr)
	EVT_TEXT(ID_BasicDensityStatsSizeText, ComponentDlg::OnBasicDensityStatsSizeText)
	//clean
	EVT_CHECKBOX(ID_BasicCleanCheck, ComponentDlg::OnBasicCleanCheck)
	EVT_BUTTON(ID_BasicCleanBtn, ComponentDlg::OnBasicCleanBtn)
	EVT_COMMAND_SCROLL(ID_BasicCleanIterSldr, ComponentDlg::OnBasicCleanIterSldr)
	EVT_TEXT(ID_BasicCleanIterText, ComponentDlg::OnBasicCleanIterText)
	EVT_COMMAND_SCROLL(ID_BasicCleanLimitSldr, ComponentDlg::OnBasicCleanLimitSldr)
	EVT_TEXT(ID_BasicCleanLimitText, ComponentDlg::OnBasicCleanLimitText)

	//clustering page
	EVT_RADIOBUTTON(ID_ClusterMethodExmaxRd, ComponentDlg::OnClusterMethodExmaxCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodDbscanRd, ComponentDlg::OnClusterMethodDbscanCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodKmeansRd, ComponentDlg::OnClusterMethodKmeansCheck)
	//parameters
	EVT_COMMAND_SCROLL(ID_ClusterClnumSldr, ComponentDlg::OnClusterClnumSldr)
	EVT_TEXT(ID_ClusterClnumText, ComponentDlg::OnClusterClnumText)
	EVT_COMMAND_SCROLL(ID_ClusterMaxIterSldr, ComponentDlg::OnClusterMaxiterSldr)
	EVT_TEXT(ID_ClusterMaxIterText, ComponentDlg::OnClusterMaxiterText)
	EVT_COMMAND_SCROLL(ID_ClusterTolSldr, ComponentDlg::OnClusterTolSldr)
	EVT_TEXT(ID_ClusterTolText, ComponentDlg::OnClusterTolText)
	EVT_COMMAND_SCROLL(ID_ClusterSizeSldr, ComponentDlg::OnClusterSizeSldr)
	EVT_TEXT(ID_ClusterSizeText, ComponentDlg::OnClusterSizeText)
	EVT_COMMAND_SCROLL(ID_ClusterEpsSldr, ComponentDlg::OnClusterEpsSldr)
	EVT_TEXT(ID_ClusterEpsText, ComponentDlg::OnClusterepsText)

	//analysis page
	EVT_TEXT(ID_CompIdText, ComponentDlg::OnCompIdText)
	EVT_TEXT_ENTER(ID_CompIdText, ComponentDlg::OnCompFull)
	EVT_BUTTON(ID_CompIdXBtn, ComponentDlg::OnCompIdXBtn)
	EVT_CHECKBOX(ID_AnalysisMinCheck, ComponentDlg::OnAnalysisMinCheck)
	EVT_SPINCTRL(ID_AnalysisMinSpin, ComponentDlg::OnAnalysisMinSpin)
	EVT_CHECKBOX(ID_AnalysisMaxCheck, ComponentDlg::OnAnalysisMaxCheck)
	EVT_SPINCTRL(ID_AnalysisMaxSpin, ComponentDlg::OnAnalysisMaxSpin)
	EVT_BUTTON(ID_CompFullBtn, ComponentDlg::OnCompFull)
	EVT_BUTTON(ID_CompExclusiveBtn, ComponentDlg::OnCompExclusive)
	EVT_BUTTON(ID_CompAppendBtn, ComponentDlg::OnCompAppend)
	EVT_BUTTON(ID_CompAllBtn, ComponentDlg::OnCompAll)
	EVT_BUTTON(ID_CompClearBtn, ComponentDlg::OnCompClear)
	EVT_CHECKBOX(ID_ConsistentCheck, ComponentDlg::OnConsistentCheck)
	EVT_CHECKBOX(ID_ColocalCheck, ComponentDlg::OnColocalCheck)
	//output
	EVT_RADIOBUTTON(ID_OutputMultiRb, ComponentDlg::OnOutputTypeRadio)
	EVT_RADIOBUTTON(ID_OutputRgbRb, ComponentDlg::OnOutputTypeRadio)
	EVT_BUTTON(ID_OutputRandomBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputSizeBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputAnnBtn, ComponentDlg::OnOutputAnn)
	//distance
	EVT_COMMAND_SCROLL(ID_DistNeighborSldr, ComponentDlg::OnDistNeighborSldr)
	EVT_TEXT(ID_DistNeighborText, ComponentDlg::OnDistNeighborText)
	EVT_BUTTON(ID_DistOutputBtn, ComponentDlg::OnDistOutput)

	//execute
	EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, ComponentDlg::OnNotebook)
	EVT_BUTTON(ID_GenerateBtn, ComponentDlg::OnGenerate)
	EVT_TOGGLEBUTTON(ID_AutoUpdateBtn, ComponentDlg::OnAutoUpdate)
	EVT_BUTTON(ID_ClusterBtn, ComponentDlg::OnCluster)
	EVT_BUTTON(ID_AnalyzeBtn, ComponentDlg::OnAnalyze)
	EVT_BUTTON(ID_AnalyzeSelBtn, ComponentDlg::OnAnalyzeSel)
END_EVENT_TABLE()

ComponentDlg::ComponentDlg(wxWindow *frame, wxWindow *parent)
	: wxPanel(parent, wxID_ANY,
		wxDefaultPosition,
		wxSize(500, 720),
		0, "ComponentDlg"),
	m_frame(parent),
	m_view(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//notebook
	m_notebook = new wxNotebook(this, ID_Notebook);
	m_notebook->AddPage(CreateCompGenPage(m_notebook), "Generate Comps");
	m_notebook->AddPage(CreateClusteringPage(m_notebook), "Cluster");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_generate_prg = new wxGauge(this, ID_GeneratePrg, 100,
		wxDefaultPosition, wxSize(-1, 18));
	m_use_sel_chk = new wxCheckBox(this, ID_UseSelChk, "Use Sel.",
		wxDefaultPosition, wxDefaultSize);
	m_generate_btn = new wxButton(this, ID_GenerateBtn, "Generate",
		wxDefaultPosition, wxSize(75, -1));
	m_auto_update_btn = new wxToggleButton(this, ID_AutoUpdateBtn, "Auto Update",
		wxDefaultPosition, wxSize(75, -1));
	m_cluster_btn = new wxButton(this, ID_ClusterBtn, "Cluster",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_btn = new wxButton(this, ID_AnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_sel_btn = new wxButton(this, ID_AnalyzeSelBtn, "Anlyz. Sel.",
		wxDefaultPosition, wxSize(75, -1));
	sizer1->Add(10, 10);
	sizer1->Add(m_generate_prg, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(m_use_sel_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_generate_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_auto_update_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_cluster_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_sel_btn, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);

	//stats text
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	m_stat_text = new wxTextCtrl(this, ID_StatText, "",
		wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE);
	m_stat_text->SetEditable(false);
	m_stat_text->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(ComponentDlg::OnKey), 0L, this);
	sizer2->Add(m_stat_text, 1, wxEXPAND);

	//all controls
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);

	SetSizer(sizerv);
	Layout();

	GetSettings();
}

ComponentDlg::~ComponentDlg()
{
	SaveSettings("");
}

wxWindow* ComponentDlg::CreateCompGenPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_iter_sldr = new wxSlider(page, ID_BasicIterSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_iter_text = new wxTextCtrl(page, ID_BasicIterText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer1->Add(2, 2);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_basic_iter_sldr, 1, wxEXPAND);
	sizer1->Add(m_basic_iter_text, 0, wxALIGN_CENTER);
	sizer1->Add(2, 2);

	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_thresh_sldr = new wxSlider(page, ID_BasicThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_thresh_text = new wxTextCtrl(page, ID_BasicThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer2->Add(2, 2);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_basic_thresh_sldr, 1, wxEXPAND);
	sizer2->Add(m_basic_thresh_text, 0, wxALIGN_CENTER);
	sizer2->Add(2, 2);

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_diff_check = new wxCheckBox(page, ID_BasicDiffCheck, "Enable Diffusion",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer3->Add(2, 2);
	sizer3->Add(m_basic_diff_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_falloff_sldr = new wxSlider(page, ID_BasicFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_falloff_text = new wxTextCtrl(page, ID_BasicFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(2, 2);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_basic_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_basic_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(2, 2);

	//size not used
	//m_basic_size_check = new wxCheckBox(page, ID_BasicSizeCheck, "Enable Size Limiter",
	//	wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	//m_basic_size_check->Hide();
	//m_basic_size_sldr = new wxSlider(page, ID_BasicSizeSldr, 100, 0, 500,
	//	wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	//m_basic_size_text = new wxTextCtrl(page, ID_BasicSizeText, "100",
	//	wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	//m_basic_size_sldr->Hide();
	//m_basic_size_text->Hide();

	//density
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_density_check = new wxCheckBox(page, ID_BasicDensityCheck, "Use Density Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer5->Add(2, 2);
	sizer5->Add(m_basic_density_check, 0, wxALIGN_CENTER);
	//
	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Density Control:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_density_sldr = new wxSlider(page, ID_BasicDensitySldr, 1000, 0, 5000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_density_text = new wxTextCtrl(page, ID_BasicDensityText, "1.0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(2, 2);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_basic_density_sldr, 1, wxEXPAND);
	sizer6->Add(m_basic_density_text, 0, wxALIGN_CENTER);
	sizer6->Add(2, 2);

	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Window Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_density_window_size_sldr = new wxSlider(page, ID_BasicDensityWindowSizeSldr, 5, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_density_window_size_text = new wxTextCtrl(page, ID_BasicDensityWindowsSizeText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer7->Add(2, 2);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_basic_density_window_size_sldr, 1, wxEXPAND);
	sizer7->Add(m_basic_density_window_size_text, 0, wxALIGN_CENTER);
	sizer7->Add(2, 2);

	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stats Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_density_stats_size_sldr = new wxSlider(page, ID_BasicDensityStatsSizeSldr, 15, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_density_stats_size_text = new wxTextCtrl(page, ID_BasicDensityStatsSizeText, "15",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer8->Add(2, 2);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_basic_density_stats_size_sldr, 1, wxEXPAND);
	sizer8->Add(m_basic_density_stats_size_text, 0, wxALIGN_CENTER);
	sizer8->Add(2, 2);

	//distance field
	wxBoxSizer* sizer9 = new wxBoxSizer(wxHORIZONTAL);
	m_use_dist_field_check = new wxCheckBox(page, ID_BasicUseDistFieldCheck, "Use Distance Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer9->Add(2, 2);
	sizer9->Add(m_use_dist_field_check, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Strength",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_dist_strength_sldr = new wxSlider(page, ID_BasicDistStrengthSldr, 500, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_dist_strength_text = new wxTextCtrl(page, ID_BasicDistStrengthText, "0.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer10->Add(2, 2);
	sizer10->Add(st, 0, wxALIGN_CENTER);
	sizer10->Add(m_basic_dist_strength_sldr, 1, wxEXPAND);
	sizer10->Add(m_basic_dist_strength_text, 0, wxALIGN_CENTER);
	sizer10->Add(2, 2);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Max Distance",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_max_dist_sldr = new wxSlider(page, ID_BasicMaxDistSldr, 30, 1, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_max_dist_text = new wxTextCtrl(page, ID_BasicMaxDistText, "30",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer11->Add(2, 2);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_basic_max_dist_sldr, 1, wxEXPAND);
	sizer11->Add(m_basic_max_dist_text, 0, wxALIGN_CENTER);
	sizer11->Add(2, 2);
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_dist_thresh_sldr = new wxSlider(page, ID_BasicDistThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_dist_thresh_text = new wxTextCtrl(page, ID_BasicDistThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer12->Add(2, 2);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(m_basic_dist_thresh_sldr, 1, wxEXPAND);
	sizer12->Add(m_basic_dist_thresh_text, 0, wxALIGN_CENTER);
	sizer12->Add(2, 2);

	//clean
	wxBoxSizer* sizer13 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_clean_check = new wxCheckBox(page, ID_BasicCleanCheck, "Clean Up",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_basic_clean_btn = new wxButton(page, ID_BasicCleanBtn, "Clean More",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	sizer13->Add(2, 2);
	sizer13->Add(m_basic_clean_check, 0, wxALIGN_CENTER);
	sizer13->AddStretchSpacer(1);
	sizer13->Add(m_basic_clean_btn, 0, wxALIGN_CENTER);
	sizer13->Add(2, 2);

	//iterations
	wxBoxSizer* sizer14 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_clean_iter_sldr = new wxSlider(page, ID_BasicCleanIterSldr, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_clean_iter_text = new wxTextCtrl(page, ID_BasicCleanIterText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer14->Add(2, 2);
	sizer14->Add(st, 0, wxALIGN_CENTER);
	sizer14->Add(m_basic_clean_iter_sldr, 1, wxEXPAND);
	sizer14->Add(m_basic_clean_iter_text, 0, wxALIGN_CENTER);
	sizer14->Add(2, 2);
	//iterations
	wxBoxSizer* sizer15 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_clean_limit_sldr = new wxSlider(page, ID_BasicCleanLimitSldr, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_clean_limit_text = new wxTextCtrl(page, ID_BasicCleanLimitText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer15->Add(2, 2);
	sizer15->Add(st, 0, wxALIGN_CENTER);
	sizer15->Add(m_basic_clean_limit_sldr, 1, wxEXPAND);
	sizer15->Add(m_basic_clean_limit_text, 0, wxALIGN_CENTER);
	sizer15->Add(2, 2);

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "ID Growth && Merge"), wxVERTICAL);
	group1->Add(5, 5);
	group1->Add(sizer1, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer2, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer3, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer4, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer5, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer6, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer7, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer8, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer9, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer10, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer11, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer12, 0, wxEXPAND);
	group1->Add(5, 5);

	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Post Cleanup"), wxVERTICAL);
	group2->Add(sizer13, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer14, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer15, 0, wxEXPAND);
	group2->Add(5, 5);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(group1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

	return page;
}

wxWindow* ComponentDlg::CreateClusteringPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp1(1);
	wxFloatingPointValidator<double> vald_fp2(2);

	//clustering methods
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Clustering Method"),
		wxVERTICAL);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Choose Method:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_method_exmax_rd = new wxRadioButton(page, ID_ClusterMethodExmaxRd,
		"EM", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_cluster_method_dbscan_rd = new wxRadioButton(page, ID_ClusterMethodDbscanRd,
		"DBSCAN", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_kmeans_rd = new wxRadioButton(page, ID_ClusterMethodKmeansRd,
		"K-Means", wxDefaultPosition, wxDefaultSize);
	sizer11->Add(5, 5);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_cluster_method_kmeans_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_exmax_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_dbscan_rd, 0, wxALIGN_CENTER);
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//parameters
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Settings"),
		wxVERTICAL);
	//clnum
	wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Cluster Number:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_clnum_sldr = new wxSlider(page, ID_ClusterClnumSldr, 2, 2, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_clnum_text = new wxTextCtrl(page, ID_ClusterClnumText, "2",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer21->Add(5, 5);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_cluster_clnum_sldr, 1, wxEXPAND);
	sizer21->Add(m_cluster_clnum_text, 0, wxALIGN_CENTER);
	sizer21->Add(5, 5);
	//maxiter
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Max Iterations:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_maxiter_sldr = new wxSlider(page, ID_ClusterMaxIterSldr, 200, 1, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_maxiter_text = new wxTextCtrl(page, ID_ClusterMaxIterText, "200",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer22->Add(5, 5);
	sizer22->Add(st, 0, wxALIGN_CENTER);
	sizer22->Add(m_cluster_maxiter_sldr, 1, wxEXPAND);
	sizer22->Add(m_cluster_maxiter_text, 0, wxALIGN_CENTER);
	sizer22->Add(5, 5);
	//tol
	wxBoxSizer *sizer23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tolerance:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_tol_sldr = new wxSlider(page, ID_ClusterTolSldr, 0.90, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_tol_text = new wxTextCtrl(page, ID_ClusterTolText, "0.90",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp2);
	sizer23->Add(5, 5);
	sizer23->Add(st, 0, wxALIGN_CENTER);
	sizer23->Add(m_cluster_tol_sldr, 1, wxEXPAND);
	sizer23->Add(m_cluster_tol_text, 0, wxALIGN_CENTER);
	sizer23->Add(5, 5);
	//size
	wxBoxSizer *sizer24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Min. Size:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_size_sldr = new wxSlider(page, ID_ClusterSizeSldr, 60, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_size_text = new wxTextCtrl(page, ID_ClusterSizeText, "60",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer24->Add(5, 5);
	sizer24->Add(st, 0, wxALIGN_CENTER);
	sizer24->Add(m_cluster_size_sldr, 1, wxEXPAND);
	sizer24->Add(m_cluster_size_text, 0, wxALIGN_CENTER);
	sizer24->Add(5, 5);
	//eps
	wxBoxSizer *sizer25 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighborhood:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_eps_sldr = new wxSlider(page, ID_ClusterEpsSldr, 25, 5, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_eps_text = new wxTextCtrl(page, ID_ClusterEpsText, "2.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp1);
	sizer25->Add(5, 5);
	sizer25->Add(st, 0, wxALIGN_CENTER);
	sizer25->Add(m_cluster_eps_sldr, 1, wxEXPAND);
	sizer25->Add(m_cluster_eps_text, 0, wxALIGN_CENTER);
	sizer25->Add(5, 5);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer23, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer24, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer25, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//note
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "N.B."),
		wxVERTICAL);
	st = new wxStaticText(page, 0,
		"Make selection with paint brush first, and then compute clustering on the selection.");
	sizer3->Add(10, 10);
	sizer3->Add(st, 0);
	sizer3->Add(10, 10);

	//all
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);

	return page;
}

wxWindow* ComponentDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//selection tools
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Selection Tools"),
		wxVERTICAL);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "ID:",
		wxDefaultPosition, wxDefaultSize);
	m_comp_id_text = new wxTextCtrl(page, ID_CompIdText, "",
		wxDefaultPosition, wxSize(80, 23), wxTE_PROCESS_ENTER);
	m_comp_id_x_btn = new wxButton(page, ID_CompIdXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	//size limiters
	m_analysis_min_check = new wxCheckBox(page, ID_AnalysisMinCheck, "Min Size:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_min_spin = new wxSpinCtrl(page, ID_AnalysisMinSpin, "0",
		wxDefaultPosition, wxSize(80, 23), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 0);
	m_analysis_max_check = new wxCheckBox(page, ID_AnalysisMaxCheck, "Max Size:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_max_spin = new wxSpinCtrl(page, ID_AnalysisMaxSpin, "100",
		wxDefaultPosition, wxSize(80, 23), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 100);
	sizer11->Add(5, 5);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer11->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_min_check, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_min_spin, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_max_check, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_max_spin, 0, wxALIGN_CENTER);
	//buttons
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Select",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_all_btn = new wxButton(page, ID_CompAllBtn, "All",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_full_btn = new wxButton(page, ID_CompFullBtn, "FullCompt",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(65, 23));
	sizer12->AddStretchSpacer();
	sizer12->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_all_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_full_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_exclusive_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer12->AddStretchSpacer();
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer12, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//colocalization
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Options"),
		wxVERTICAL);
	wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
	m_consistent_check = new wxCheckBox(page, ID_ConsistentCheck, "Make color consistent for multiple bricks",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer21->Add(5, 5);
	sizer21->Add(m_consistent_check, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	m_colocal_check = new wxCheckBox(page, ID_ColocalCheck, "Compute colocalization with other channels",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer22->Add(5, 5);
	sizer22->Add(m_colocal_check, 0, wxALIGN_CENTER);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//output
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Output as New Channels"),
		wxVERTICAL);
	//radios
	wxBoxSizer *sizer31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Channel Type:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_multi_rb = new wxRadioButton(page, ID_OutputMultiRb, "Each Comp.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_output_rgb_rb = new wxRadioButton(page, ID_OutputRgbRb, "R+G+B",
		wxDefaultPosition, wxDefaultSize);
	sizer31->Add(5, 5);
	sizer31->Add(st, 0, wxALIGN_CENTER);
	sizer31->Add(m_output_multi_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	sizer31->Add(m_output_rgb_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	//buttons
	wxBoxSizer *sizer32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Output:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_random_btn = new wxButton(page, ID_OutputRandomBtn, "Random Colors",
		wxDefaultPosition, wxSize(100, 23));
	m_output_size_btn = new wxButton(page, ID_OutputSizeBtn, "Size-based",
		wxDefaultPosition, wxSize(100, 23));
	m_output_ann_btn = new wxButton(page, ID_OutputAnnBtn, "Annotations",
		wxDefaultPosition, wxSize(100, 23));
	sizer32->Add(5, 5);
	sizer32->Add(st, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_random_btn, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_size_btn, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_ann_btn, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer31, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer32, 0, wxEXPAND);
	sizer3->Add(10, 10);

	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Distances"),
		wxVERTICAL);
	wxBoxSizer *sizer41 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighbors:",
		wxDefaultPosition, wxSize(100, 20));
	m_dist_neighbor_sldr = new wxSlider(page, ID_DistNeighborSldr, 1, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_neighbor_text = new wxTextCtrl(page, ID_DistNeighborText, "1",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_dist_output_btn = new wxButton(page, ID_DistOutputBtn, "Compute",
		wxDefaultPosition, wxSize(60, 23));
	sizer41->Add(5, 5);
	sizer41->Add(st, 0, wxALIGN_CENTER);
	sizer41->Add(m_dist_neighbor_sldr, 1, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	sizer41->Add(m_dist_neighbor_text, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	sizer41->Add(m_dist_output_btn, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	//
	sizer4->Add(10, 10);
	sizer4->Add(sizer41, 0, wxEXPAND);
	sizer4->Add(10, 10);

	//note
	wxBoxSizer *sizer5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "N.B."),
		wxVERTICAL);
	st = new wxStaticText(page, 0,
		"Enable 4D script in the settings to show component colors.");
	sizer5->Add(10, 10);
	sizer5->Add(st, 0);
	sizer5->Add(10, 10);

	//all
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer4, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer5, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);

	m_analysis_min_check->SetValue(false);
	m_analysis_min_spin->Disable();
	m_analysis_max_check->SetValue(false);
	m_analysis_max_spin->Disable();

	return page;
}

void ComponentDlg::Update()
{
	//update ui
	//comp generate page
	m_basic_iter_text->SetValue(wxString::Format("%d", m_basic_iter));
	m_basic_thresh_text->SetValue(wxString::Format("%.3f", m_basic_thresh));
	m_use_dist_field_check->SetValue(m_use_dist_field);
	EnableUseDistField(m_use_dist_field);
	m_basic_dist_strength_text->SetValue(wxString::Format("%.3f", m_basic_dist_strength));
	m_basic_max_dist_text->SetValue(wxString::Format("%d", m_basic_max_dist));
	m_basic_dist_thresh_text->SetValue(wxString::Format("%.3f", m_basic_dist_thresh));
	m_basic_diff_check->SetValue(m_basic_diff);
	EnableBasicDiff(m_basic_diff);
	m_basic_falloff_text->SetValue(wxString::Format("%.3f", m_basic_falloff));
	//m_basic_size_check->SetValue(m_basic_size);
	EnableBasicSize(m_basic_size);
	//m_basic_size_text->SetValue(wxString::Format("%d", m_basic_size_lm));
	EnableBasicDensity(m_basic_density);
	m_basic_density_check->SetValue(m_basic_density);
	m_basic_density_text->SetValue(wxString::Format("%.3f", m_basic_density_thresh));
	m_basic_density_window_size_text->SetValue(wxString::Format("%d", m_basic_density_window_size));
	m_basic_density_stats_size_text->SetValue(wxString::Format("%d", m_basic_density_stats_size));
	EnableBasicClean(m_basic_clean);
	m_basic_clean_check->SetValue(m_basic_clean);
	m_basic_clean_iter_text->SetValue(wxString::Format("%d", m_basic_clean_iter));
	m_basic_clean_limit_text->SetValue(wxString::Format("%d", m_basic_clean_size_vl));

	//cluster page
	m_cluster_method_exmax_rd->SetValue(m_cluster_method_exmax);
	m_cluster_method_dbscan_rd->SetValue(m_cluster_method_dbscan);
	m_cluster_method_kmeans_rd->SetValue(m_cluster_method_kmeans);
	//parameters
	m_cluster_clnum_text->SetValue(wxString::Format("%d", m_cluster_clnum));
	m_cluster_maxiter_text->SetValue(wxString::Format("%d", m_cluster_maxiter));
	m_cluster_tol_text->SetValue(wxString::Format("%.2f", m_cluster_tol));
	m_cluster_size_text->SetValue(wxString::Format("%d", m_cluster_size));
	m_cluster_eps_text->SetValue(wxString::Format("%.1f", m_cluster_eps));
	UpdateClusterMethod();

	//selection
	if (m_use_min)
	{
		m_analysis_min_check->SetValue(true);
		m_analysis_min_spin->Enable();
	}
	else
	{
		m_analysis_min_check->SetValue(false);
		m_analysis_min_spin->Disable();
	}
	m_analysis_min_spin->SetValue(m_min_num);
	if (m_use_max)
	{
		m_analysis_max_check->SetValue(true);
		m_analysis_max_spin->Enable();
	}
	else
	{
		m_analysis_max_check->SetValue(false);
		m_analysis_max_spin->Disable();
	}
	m_analysis_max_spin->SetValue(m_max_num);

	//options
	m_consistent_check->SetValue(m_consistent);
	m_colocal_check->SetValue(m_colocal);

	//output type
	m_output_multi_rb->SetValue(false);
	m_output_rgb_rb->SetValue(false);
	if (m_output_type == 1)
		m_output_multi_rb->SetValue(true);
	else if (m_output_type == 2)
		m_output_rgb_rb->SetValue(true);

	//generate
	EnableGenerate();
}

void ComponentDlg::GetSettings()
{
	//defaults
	//comp generate page
	m_basic_iter = 50;
	m_basic_thresh = 0.5;
	m_use_dist_field = false;
	m_basic_dist_strength = 0.5;
	m_basic_max_dist = 30;
	m_basic_dist_thresh = 0.25;
	m_basic_diff = false;
	m_basic_falloff = 0.01;
	m_basic_size = false;
	m_basic_size_lm = 100;
	m_basic_density = false;
	m_basic_density_thresh = 1.0;
	m_basic_density_window_size = 5;
	m_basic_density_stats_size = 15;
	m_basic_clean = false;
	m_basic_clean_iter = 5;
	m_basic_clean_size_vl = 5;

	//cluster
	m_cluster_method_kmeans = true;
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = false;
	m_cluster_clnum = 2;
	m_cluster_maxiter = 200;
	m_cluster_tol = 0.9f;
	m_cluster_size = 60;
	m_cluster_eps = 2.5;

	//selection
	m_use_min = false;
	m_min_num = 0;
	m_use_max = false;
	m_max_num = 0;
	//colocalization
	m_colocal = false;
	m_consistent = false;

	//distance
	m_dist_neighbor = 1;

	m_auto_update = false;

	//output
	m_output_type = 1;

	//read values
	LoadSettings("");
	Update();
}

void ComponentDlg::LoadSettings(wxString filename)
{
	bool get_basic = false;
	if (!wxFileExists(filename))
	{
		wxString expath = wxStandardPaths::Get().GetExecutablePath();
		expath = wxPathOnly(expath);
		filename = expath + "/default_component_settings.dft";
		get_basic = true;
	}
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//basic settings
	fconfig.Read("basic_iter", &m_basic_iter);
	fconfig.Read("basic_thresh", &m_basic_thresh);
	fconfig.Read("use_dist_field", &m_use_dist_field);
	fconfig.Read("basic_dist_strength", &m_basic_dist_strength);
	fconfig.Read("basic_max_dist", &m_basic_max_dist);
	fconfig.Read("basic_dist_thresh", &m_basic_dist_thresh);
	fconfig.Read("basic_diff", &m_basic_diff);
	fconfig.Read("basic_falloff", &m_basic_falloff);
	fconfig.Read("basic_size", &m_basic_size);
	fconfig.Read("basic_size_lm", &m_basic_size_lm);
	fconfig.Read("basic_density", &m_basic_density);
	fconfig.Read("basic_density_thresh", &m_basic_density_thresh);
	fconfig.Read("basic_density_window_size", &m_basic_density_window_size);
	fconfig.Read("basic_density_stats_size", &m_basic_density_stats_size);
	fconfig.Read("basic_clean", &m_basic_clean);
	fconfig.Read("basic_clean_iter", &m_basic_clean_iter);
	fconfig.Read("basic_clean_size_vl", &m_basic_clean_size_vl);

	//cluster
	fconfig.Read("cluster_method_kmeans", &m_cluster_method_kmeans);
	fconfig.Read("cluster_method_exmax", &m_cluster_method_exmax);
	fconfig.Read("cluster_method_dbscan", &m_cluster_method_dbscan);
	//parameters
	fconfig.Read("cluster_clnum", &m_cluster_clnum);
	fconfig.Read("cluster_maxiter", &m_cluster_maxiter);
	fconfig.Read("cluster_tol", &m_cluster_tol);
	fconfig.Read("cluster_size", &m_cluster_size);
	fconfig.Read("cluster_eps", &m_cluster_eps);

	//selection
	fconfig.Read("use_min", &m_use_min);
	fconfig.Read("min_num", &m_min_num);
	fconfig.Read("use_max", &m_use_max);
	fconfig.Read("max_num", &m_max_num);
	//colocalization
	fconfig.Read("colocal", &m_colocal);
	//output
	fconfig.Read("output_type", &m_output_type);

	//m_load_settings_text->SetValue(filename);
}

void ComponentDlg::SaveSettings(wxString filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	//comp generate settings
	fconfig.Write("basic_iter", m_basic_iter);
	fconfig.Write("basic_thresh", m_basic_thresh);
	fconfig.Write("use_dist_field", m_use_dist_field);
	fconfig.Write("basic_dist_strength", m_basic_dist_strength);
	fconfig.Write("basic_max_dist", m_basic_max_dist);
	fconfig.Write("basic_dist_thresh", m_basic_dist_thresh);
	fconfig.Write("basic_diff", m_basic_diff);
	fconfig.Write("basic_falloff", m_basic_falloff);
	fconfig.Write("basic_size", m_basic_size);
	fconfig.Write("basic_size_lm", m_basic_size_lm);
	fconfig.Write("basic_density", m_basic_density);
	fconfig.Write("basic_density_thresh", m_basic_density_thresh);
	fconfig.Write("basic_density_window_size", m_basic_density_window_size);
	fconfig.Write("basic_density_stats_size", m_basic_density_stats_size);
	fconfig.Write("basic_clean", m_basic_clean);
	fconfig.Write("basic_clean_iter", m_basic_clean_iter);
	fconfig.Write("basic_clean_size_vl", m_basic_clean_size_vl);

	//cluster
	fconfig.Write("cluster_method_kmeans", m_cluster_method_kmeans);
	fconfig.Write("cluster_method_exmax", m_cluster_method_exmax);
	fconfig.Write("cluster_method_dbscan", m_cluster_method_dbscan);
	//parameters
	fconfig.Write("cluster_clnum", m_cluster_clnum);
	fconfig.Write("cluster_maxiter", m_cluster_maxiter);
	fconfig.Write("cluster_tol", m_cluster_tol);
	fconfig.Write("cluster_size", m_cluster_size);
	fconfig.Write("cluster_eps", m_cluster_eps);

	//selection
	fconfig.Write("use_min", m_use_min);
	fconfig.Write("min_num", m_min_num);
	fconfig.Write("use_max", m_use_max);
	fconfig.Write("max_num", m_max_num);
	//colocalization
	fconfig.Write("colocal", m_colocal);
	//output
	fconfig.Write("output_type", m_output_type);

	if (filename == "")
	{
		wxString expath = wxStandardPaths::Get().GetExecutablePath();
		expath = wxPathOnly(expath);
		filename = expath + "/default_component_settings.dft";
	}

	wxFileOutputStream os(filename);
	fconfig.Save(os);
}

void ComponentDlg::OnLoadSettings(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a FluoRender component generator setting file",
		"", "", "*.txt;*.dft", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		LoadSettings(filename);
		Update();
	}

	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::OnSaveSettings(wxCommandEvent& event)
{
	wxString filename = m_load_settings_text->GetValue();
	if (wxFileExists(filename))
		SaveSettings(filename);
	else
	{
		wxCommandEvent e;
		OnSaveasSettings(e);
	}
}

void ComponentDlg::OnSaveasSettings(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save a FluoRender component generator setting file",
		"", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveSettings(filename);
		m_load_settings_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

//comp generate page
void ComponentDlg::OnBasicIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_iter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicIterText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_iter_text->GetValue().ToLong(&val);
	m_basic_iter = val;
	m_basic_iter_sldr->SetValue(m_basic_iter);

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_thresh_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_thresh_text->GetValue().ToDouble(&val);
	m_basic_thresh = val;
	m_basic_thresh_sldr->SetValue(int(m_basic_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::EnableUseDistField(bool value)
{
	m_use_dist_field = value;
	if (m_use_dist_field)
	{
		m_basic_dist_strength_sldr->Enable();
		m_basic_dist_strength_text->Enable();
		m_basic_max_dist_sldr->Enable();
		m_basic_max_dist_text->Enable();
		m_basic_dist_thresh_sldr->Enable();
		m_basic_dist_thresh_text->Enable();
	}
	else
	{
		m_basic_dist_strength_sldr->Disable();
		m_basic_dist_strength_text->Disable();
		m_basic_max_dist_sldr->Disable();
		m_basic_max_dist_text->Disable();
		m_basic_dist_thresh_sldr->Disable();
		m_basic_dist_thresh_text->Disable();
	}
}

void ComponentDlg::EnableBasicDiff(bool value)
{
	m_basic_diff = value;
	if (m_basic_diff)
	{
		m_basic_falloff_sldr->Enable();
		m_basic_falloff_text->Enable();
	}
	else
	{
		m_basic_falloff_sldr->Disable();
		m_basic_falloff_text->Disable();
	}
}

void ComponentDlg::OnBasicDistStrengthSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_dist_strength_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicDistStrengthText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_dist_strength_text->GetValue().ToDouble(&val);
	m_basic_dist_strength = val;
	m_basic_dist_strength_sldr->SetValue(int(m_basic_dist_strength * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicUseDistFieldCheck(wxCommandEvent &event)
{
	EnableUseDistField(m_use_dist_field_check->GetValue());

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicMaxDistSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_max_dist_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicMaxDistText(wxCommandEvent &event)
{
	long val = 1;
	m_basic_max_dist_text->GetValue().ToLong(&val);
	if (val > 255)
		val = 255;
	m_basic_max_dist = val;
	m_basic_max_dist_sldr->SetValue(m_basic_max_dist);

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicDistThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_dist_thresh_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicDistThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_dist_thresh_text->GetValue().ToDouble(&val);
	m_basic_dist_thresh = val;
	m_basic_dist_thresh_sldr->SetValue(int(m_basic_dist_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicDiffCheck(wxCommandEvent &event)
{
	EnableBasicDiff(m_basic_diff_check->GetValue());

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_falloff_text->GetValue().ToDouble(&val);
	m_basic_falloff = val;
	m_basic_falloff_sldr->SetValue(int(m_basic_falloff * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::EnableBasicSize(bool value)
{
	m_basic_size = value;
	if (m_basic_size)
	{
		//m_basic_size_sldr->Enable();
		//m_basic_size_text->Enable();
	}
	else
	{
		//m_basic_size_sldr->Disable();
		//m_basic_size_text->Disable();
	}
}

void ComponentDlg::OnBasicSizeCheck(wxCommandEvent &event)
{
	//EnableBasicSize(m_basic_size_check->GetValue());
}

void ComponentDlg::OnBasicSizeSldr(wxScrollEvent &event)
{
	//int val = event.GetPosition();
	//m_basic_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicSizeText(wxCommandEvent &event)
{
	//long val = 0;
	//m_basic_size_text->GetValue().ToLong(&val);
	//m_basic_size_lm = (int)val;
	//m_basic_size_sldr->SetValue(m_basic_size_lm);
}

void ComponentDlg::EnableBasicDensity(bool value)
{
	m_basic_density = value;
	if (m_basic_density)
	{
		m_basic_density_sldr->Enable();
		m_basic_density_text->Enable();
		m_basic_density_window_size_sldr->Enable();
		m_basic_density_window_size_text->Enable();
		m_basic_density_stats_size_sldr->Enable();
		m_basic_density_stats_size_text->Enable();
	}
	else
	{
		m_basic_density_sldr->Disable();
		m_basic_density_text->Disable();
		m_basic_density_window_size_sldr->Disable();
		m_basic_density_window_size_text->Disable();
		m_basic_density_stats_size_sldr->Disable();
		m_basic_density_stats_size_text->Disable();
	}
}

void ComponentDlg::OnBasicDensityCheck(wxCommandEvent &event)
{
	EnableBasicDensity(m_basic_density_check->GetValue());

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicDensitySldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_density_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicDensityText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_density_text->GetValue().ToDouble(&val);
	m_basic_density_thresh = val;
	m_basic_density_sldr->SetValue(int(m_basic_density_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicDensityWindowSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_density_window_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicDensityWindowSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_density_window_size_text->GetValue().ToLong(&val);
	m_basic_density_window_size = val;
	m_basic_density_window_size_sldr->SetValue(m_basic_density_window_size);

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicDensityStatsSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_density_stats_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicDensityStatsSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_density_stats_size_text->GetValue().ToLong(&val);
	m_basic_density_stats_size = val;
	m_basic_density_stats_size_sldr->SetValue(m_basic_density_stats_size);

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::EnableBasicClean(bool value)
{
	m_basic_clean = value;
	if (m_basic_clean)
	{
		m_basic_clean_btn->Enable();
		m_basic_clean_iter_sldr->Enable();
		m_basic_clean_iter_text->Enable();
		m_basic_clean_limit_sldr->Enable();
		m_basic_clean_limit_text->Enable();
	}
	else
	{
		m_basic_clean_btn->Disable();
		m_basic_clean_iter_sldr->Disable();
		m_basic_clean_iter_text->Disable();
		m_basic_clean_limit_sldr->Disable();
		m_basic_clean_limit_text->Disable();
	}
}

void ComponentDlg::OnBasicCleanCheck(wxCommandEvent &event)
{
	EnableBasicClean(m_basic_clean_check->GetValue());

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicCleanIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_clean_iter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicCleanIterText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_clean_iter_text->GetValue().ToLong(&val);
	m_basic_clean_iter = (int)val;
	m_basic_clean_iter_sldr->SetValue(m_basic_clean_iter);

	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnBasicCleanLimitSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_clean_limit_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicCleanLimitText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_clean_limit_text->GetValue().ToLong(&val);
	m_basic_clean_size_vl = (int)val;
	m_basic_clean_limit_sldr->SetValue(m_basic_clean_size_vl);

	if (m_auto_update)
		GenerateComp();
}

//clustering page
void ComponentDlg::UpdateClusterMethod()
{
	if (m_cluster_method_exmax ||
		m_cluster_method_kmeans)
	{
		m_cluster_clnum_sldr->Enable();
		m_cluster_clnum_text->Enable();
		m_cluster_size_sldr->Disable();
		m_cluster_size_text->Disable();
		m_cluster_eps_sldr->Disable();
		m_cluster_eps_text->Disable();
	}
	if (m_cluster_method_dbscan)
	{
		m_cluster_clnum_sldr->Disable();
		m_cluster_clnum_text->Disable();
		m_cluster_size_sldr->Enable();
		m_cluster_size_text->Enable();
		m_cluster_eps_sldr->Enable();
		m_cluster_eps_text->Enable();
	}
}

void ComponentDlg::OnClusterMethodExmaxCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = true;
	m_cluster_method_dbscan = false;
	m_cluster_method_kmeans = false;
	UpdateClusterMethod();
}

void ComponentDlg::OnClusterMethodDbscanCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = true;
	m_cluster_method_kmeans = false;
	UpdateClusterMethod();
}

void ComponentDlg::OnClusterMethodKmeansCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = false;
	m_cluster_method_kmeans = true;
	UpdateClusterMethod();
}

//parameters
void ComponentDlg::OnClusterClnumSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cluster_clnum_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnClusterClnumText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_clnum_text->GetValue().ToLong(&val);
	m_cluster_clnum = (int)val;
	m_cluster_clnum_sldr->SetValue(m_cluster_clnum);
}

void ComponentDlg::OnClusterMaxiterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cluster_maxiter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnClusterMaxiterText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_maxiter_text->GetValue().ToLong(&val);
	m_cluster_maxiter = (int)val;
	m_cluster_maxiter_sldr->SetValue(m_cluster_maxiter);
}

void ComponentDlg::OnClusterTolSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cluster_tol_text->SetValue(wxString::Format("%.2f", double(val) / 100.0));
}

void ComponentDlg::OnClusterTolText(wxCommandEvent &event)
{
	double val = 0.9;
	m_cluster_tol_text->GetValue().ToDouble(&val);
	m_cluster_tol = (float)val;
	m_cluster_tol_sldr->SetValue(int(val * 100));
}

void ComponentDlg::OnClusterSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cluster_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnClusterSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_size_text->GetValue().ToLong(&val);
	m_cluster_size = (int)val;
	m_cluster_size_sldr->SetValue(m_cluster_size);
}

void ComponentDlg::OnClusterEpsSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 10.0;
	m_cluster_eps_text->SetValue(wxString::Format("%.1f", val));
}

void ComponentDlg::OnClusterepsText(wxCommandEvent &event)
{
	double val = 0.0;
	m_cluster_eps_text->GetValue().ToDouble(&val);
	m_cluster_eps = val;
	m_cluster_eps_sldr->SetValue(int(m_cluster_eps * 10.0 + 0.5));
}

//analysis page
void ComponentDlg::OnCompIdText(wxCommandEvent &event)
{
	wxString str = m_comp_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			Color c = HSVColor(id % 360, 1.0, 1.0);
			color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
		}
		m_comp_id_text->SetBackgroundColour(color);
	}
	else
		m_comp_id_text->SetBackgroundColour(color);
	m_comp_id_text->Refresh();
}

void ComponentDlg::OnCompIdXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
}

void ComponentDlg::OnAnalysisMinCheck(wxCommandEvent &event)
{
	if (m_analysis_min_check->GetValue())
	{
		m_analysis_min_spin->Enable();
		m_use_min = true;
	}
	else
	{
		m_analysis_min_spin->Disable();
		m_use_min = false;
	}
}

void ComponentDlg::OnAnalysisMinSpin(wxSpinEvent &event)
{
	m_min_num = m_analysis_min_spin->GetValue();
}

void ComponentDlg::OnAnalysisMaxCheck(wxCommandEvent &event)
{
	if (m_analysis_max_check->GetValue())
	{
		m_analysis_max_spin->Enable();
		m_use_max = true;
	}
	else
	{
		m_analysis_max_spin->Disable();
		m_use_max = false;
	}
}

void ComponentDlg::OnAnalysisMaxSpin(wxSpinEvent &event)
{
	m_max_num = m_analysis_max_spin->GetValue();
}

void ComponentDlg::OnCompFull(wxCommandEvent &event)
{
	SelectFullComp();
}

void ComponentDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	std::string sstr;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();
	sstr = str.ToStdString();

	if (GetIds(sstr, id, brick_id))
	{
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		FL::ComponentSelector comp_selector(vd);
		comp_selector.SetId(id);
		comp_selector.SetBrickId(brick_id);

		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		comp_selector.SetMaxNum(use, num);
		comp_selector.SetAnalyzer(&m_comp_analyzer);
		comp_selector.Exclusive();

		m_view->RefreshGL();

		//frame
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetBrushToolDlg())
			vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
	}
}

void ComponentDlg::OnCompAppend(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	std::string sstr;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();
	sstr = str.ToStdString();
	bool get_all = GetIds(sstr, id, brick_id);
	get_all = !get_all;

	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.SetId(id);
	comp_selector.SetBrickId(brick_id);

	//cell size filter
	bool use = m_analysis_min_check->GetValue();
	unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
	comp_selector.SetMinNum(use, num);
	use = m_analysis_max_check->GetValue();
	num = (unsigned int)(m_analysis_max_spin->GetValue());
	comp_selector.SetMaxNum(use, num);
	comp_selector.SetAnalyzer(&m_comp_analyzer);
	comp_selector.Select(get_all);

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnCompAll(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.All();

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnCompClear(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnConsistentCheck(wxCommandEvent &event)
{
	m_consistent = m_consistent_check->GetValue();
}

void ComponentDlg::OnColocalCheck(wxCommandEvent &event)
{
	m_colocal = m_colocal_check->GetValue();
}

void ComponentDlg::EnableGenerate()
{
	int page = m_notebook->GetSelection();
	switch (page)
	{
	case 0:
	default:
		m_use_sel_chk->Show();
		m_generate_btn->Show();
		m_auto_update_btn->Show();
		m_cluster_btn->Hide();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 1:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_auto_update_btn->Hide();
		m_cluster_btn->Show();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 2:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_auto_update_btn->Hide();
		m_cluster_btn->Hide();
		m_analyze_btn->Show();
		m_analyze_sel_btn->Show();
		break;
	}
	Layout();
}

//output
void ComponentDlg::OnOutputTypeRadio(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
	case ID_OutputMultiRb:
		m_output_type = 1;
		break;
	case ID_OutputRgbRb:
		m_output_type = 2;
		break;
	}
}

void ComponentDlg::OutputMulti(int color_type)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	list<VolumeData*> channs;
	if (m_comp_analyzer.GenMultiChannels(channs, color_type, m_consistent))
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			wxString group_name = "";
			DataGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				VolumeData* vd = *i;
				if (vd)
				{
					vr_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group_name = m_view->AddGroup("");
						group = m_view->GetGroup(group_name);
					}
					m_view->AddVolumeData(vd, group_name);
				}
			}
			if (group)
			{
				//group->SetSyncRAll(true);
				//group->SetSyncGAll(true);
				//group->SetSyncBAll(true);
				FLIVR::Color col = vd->GetGamma();
				group->SetGammaAll(col);
				col = vd->GetBrightness();
				group->SetBrightnessAll(col);
				col = vd->GetHdr();
				group->SetHdrAll(col);
			}
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
			m_view->RefreshGL();
		}
	}
}

void ComponentDlg::OutputRgb(int color_type)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	list<VolumeData*> channs;
	if (m_comp_analyzer.GenRgbChannels(channs, color_type, m_consistent))
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			wxString group_name = "";
			DataGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				VolumeData* vd = *i;
				if (vd)
				{
					vr_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group_name = m_view->AddGroup("");
						group = m_view->GetGroup(group_name);
					}
					m_view->AddVolumeData(vd, group_name);
				}
			}
			if (group)
			{
				//group->SetSyncRAll(true);
				//group->SetSyncGAll(true);
				//group->SetSyncBAll(true);
				FLIVR::Color col = vd->GetGamma();
				group->SetGammaAll(col);
				col = vd->GetBrightness();
				group->SetBrightnessAll(col);
				col = vd->GetHdr();
				group->SetHdrAll(col);
			}
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
			m_view->RefreshGL();
		}
	}
}

void ComponentDlg::OnOutputChannels(wxCommandEvent &event)
{
	int id = event.GetId();
	int color_type;
	if (id == ID_OutputRandomBtn)
		color_type = 1;
	else if (id == ID_OutputSizeBtn)
		color_type = 2;

	if (m_output_type == 1)
		OutputMulti(color_type);
	else if (m_output_type == 2)
		OutputRgb(color_type);
}

void ComponentDlg::OnOutputAnn(wxCommandEvent &event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	Annotations* ann = new Annotations();
	if (m_comp_analyzer.GenAnnotations(*ann, m_consistent))
	{
		ann->SetVolume(vd);
		ann->SetTransform(vd->GetTexture()->transform());
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			DataManager* mgr = vr_frame->GetDataManager();
			if (mgr)
				mgr->AddAnnotations(ann);
			m_view->AddAnnotations(ann);
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
		}
		m_view->RefreshGL();
	}
}

//distance
void ComponentDlg::OnDistNeighborSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_dist_neighbor_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnDistNeighborText(wxCommandEvent &event)
{
	long val = 0;
	m_dist_neighbor_text->GetValue().ToLong(&val);
	m_dist_neighbor = (int)val;
	m_dist_neighbor_sldr->SetValue(m_dist_neighbor);
}

void ComponentDlg::OnDistOutput(wxCommandEvent &event)
{
	FL::CompList* list = m_comp_analyzer.GetCompList();
	if (!list || list->empty())
		return;

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;

	int num = list->size();
	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}
	//compute
	size_t x = 0, y = 0;
	double dist = 0;
	for (auto it1 = list->begin();
		it1 != list->end(); ++it1)
	{
		y = x;
		for (auto it2 = it1;
			it2 != list->end(); ++it2)
		{
			dist = (it1->second->GetPos(sx, sy, sz) -
				it2->second->GetPos(sx, sy, sz)).length();
			rm[x][y] = dist;
			rm[y][x] = dist;
			y++;
		}
		x++;
	}

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		string str = filename.ToStdString();
		std::ofstream outfile;
		outfile.open(str, std::ofstream::out);
		for (size_t i = 0; i < num; ++i)
		{
			for (size_t j = 0; j < num; ++j)
			{
				outfile << rm[i][j];
				if (j < num - 1)
					outfile << "\t";
			}
			outfile << "\n";
		}
		outfile.close();
	}
	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::OnNotebook(wxBookCtrlEvent &event)
{
	EnableGenerate();
}

void ComponentDlg::OnGenerate(wxCommandEvent &event)
{
	GenerateComp();
}

void ComponentDlg::OnAutoUpdate(wxCommandEvent &event)
{
	m_auto_update = m_auto_update_btn->GetValue();
	if (m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnCluster(wxCommandEvent &event)
{
	Cluster();
}

void ComponentDlg::OnBasicCleanBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	vd->AddEmptyMask(1);

	int clean_iter = m_basic_clean_iter;
	int clean_size = m_basic_clean_size_vl;
	if (!m_basic_clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();

	m_generate_prg->SetValue(0);

	FL::ComponentGenerator cg(vd);
	boost::signals2::connection connection =
		cg.m_sig_progress.connect(boost::bind(
			&ComponentDlg::UpdateProgress, this));

	cg.SetUseMask(m_use_sel_chk->GetValue());

	if (bn > 1)
		cg.ClearBorders3D();

	if (clean_iter > 0)
		cg.Cleanup3D(clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorder3D(0.1);

	vd->GetVR()->clear_tex_current();
	m_view->RefreshGL();

	m_generate_prg->SetValue(100);
	connection.disconnect();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->GetSettingDlg()->SetRunScript(true);
		vr_frame->GetMovieView()->GetScriptSettings();
	}
}

void ComponentDlg::Cluster()
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	int bits = vd->GetBits();
	void* data_data = nrrd_data->data;
	if (!data_data)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel();
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	double scale = vd->GetScalarScale();
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);

	FL::ClusterMethod* method = 0;
	//switch method
	if (m_cluster_method_exmax)
	{
		FL::ClusterExmax* method_exmax = new FL::ClusterExmax();
		method_exmax->SetClnum(m_cluster_clnum);
		method_exmax->SetMaxiter(m_cluster_maxiter);
		method_exmax->SetProbTol(m_cluster_tol);
		method = method_exmax;
	}
	else if (m_cluster_method_dbscan)
	{
		FL::ClusterDbscan* method_dbscan = new FL::ClusterDbscan();
		method_dbscan->SetSize(m_cluster_size);
		method_dbscan->SetEps(m_cluster_eps);
		method = method_dbscan;
	}
	else if (m_cluster_method_kmeans)
	{
		FL::ClusterKmeans* method_kmeans = new FL::ClusterKmeans();
		method_kmeans->SetClnum(m_cluster_clnum);
		method_kmeans->SetMaxiter(m_cluster_maxiter);
		method = method_kmeans;
	}

	if (!method)
		return;

	method->SetSpacings(spcx, spcy, spcz);

	//add cluster points
	size_t i, j, k;
	size_t index;
	size_t nxyz = nx * ny * nz;
	unsigned char mask_value;
	float data_value;
	unsigned int label_value;
	bool use_init_cluster = false;
	struct CmpCnt
	{
		unsigned int id;
		unsigned int size;
		bool operator<(const CmpCnt &cc) const
		{
			return size > cc.size;
		}
	};
	std::unordered_map<unsigned int, CmpCnt> init_clusters;
	std::set<CmpCnt> ordered_clusters;
	if (m_cluster_method_exmax)
	{
		for (index = 0; index < nxyz; ++index)
		{
			mask_value = data_mask[index];
			if (!mask_value)
				continue;
			label_value = data_label[index];
			if (!label_value)
				continue;
			auto it = init_clusters.find(label_value);
			if (it == init_clusters.end())
			{
				CmpCnt cc = { label_value, 1 };
				init_clusters.insert(std::pair<unsigned int, CmpCnt>(
					label_value, cc));
			}
			else
			{
				it->second.size++;
			}
		}
		if (init_clusters.size() >= m_cluster_clnum)
		{
			for (auto it = init_clusters.begin();
				it != init_clusters.end(); ++it)
				ordered_clusters.insert(it->second);
			use_init_cluster = true;
		}
	}

	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx * ny*k + nx * j + i;
		mask_value = data_mask[index];
		if (mask_value)
		{
			if (bits == 8)
				data_value = ((unsigned char*)data_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)data_data)[index] * scale / 65535.0f;
			FL::EmVec pnt = { i, j, k };
			int cid = -1;
			if (use_init_cluster)
			{
				label_value = data_label[index];
				cid = 0;
				bool found = false;
				for (auto it = ordered_clusters.begin();
					it != ordered_clusters.end(); ++it)
				{
					if (label_value == it->id)
					{
						found = true;
						break;
					}
					cid++;
				}
				if (!found)
					cid = -1;
			}
			method->AddClusterPoint(
				pnt, data_value, cid);
		}
	}

	if (method->Execute())
	{
		method->GenerateNewIDs(0, (void*)data_label, nx, ny, nz, 60);
		vd->GetVR()->clear_tex_pool();
		m_view->RefreshGL();
	}

	delete method;
}

bool ComponentDlg::GetIds(std::string &str, unsigned int &id, int &brick_id)
{
	std::string::size_type sz;
	try
	{
		id = std::stoul(str, &sz);
	}
	catch (...)
	{
		return false;
	}
	std::string str2;
	if (sz < str.size())
	{
		brick_id = id;
		for (size_t i = sz; i< str.size() - 1; ++i)
		{
			if (std::isdigit(static_cast<unsigned char>(str[i])))
			{
				str2 = str.substr(i);
				try
				{
					id = std::stoul(str2);
				}
				catch(...)
				{
					return false;
				}
				return true;
			}
		}
	}
	brick_id = -1;
	return true;
}

void ComponentDlg::GenerateComp()
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	vd->AddEmptyMask(1);

	int dsize = m_basic_density_window_size;
	int stats_size = m_basic_density_stats_size;
	int clean_iter = m_basic_clean_iter;
	int clean_size = m_basic_clean_size_vl;
	if (!m_basic_clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();
	double scale = vd->GetScalarScale();
	double scale2 = scale * scale;

	//m_prog_bit = 97.0f / float(bn * 3);
	//m_prog = 0.0f;
	m_generate_prg->SetValue(0);

	FL::ComponentGenerator cg(vd);
	//boost::signals2::connection connection =
	//	cg.m_sig_progress.connect(boost::bind(
	//		&ComponentDlg::UpdateProgress, this));

	cg.SetUseMask(m_use_sel_chk->GetValue());

	vd->AddEmptyLabel();
	cg.ShuffleID_3D();

	if (m_use_dist_field)
	{
		if (m_basic_density)
		{
			cg.DistDensityField3D(
				m_basic_diff, m_basic_iter,
				float(m_basic_thresh / scale),
				float(m_basic_falloff / scale2),
				m_basic_max_dist,
				float(m_basic_dist_thresh / scale),
				m_basic_dist_strength,
				dsize, stats_size,
				float(m_basic_density_thresh / scale),
				scale);
		}
		else
		{
			cg.DistGrow3D(m_basic_diff, m_basic_iter,
				float(m_basic_thresh / scale),
				float(m_basic_falloff / scale2),
				m_basic_max_dist,
				float(m_basic_dist_thresh / scale),
				scale, m_basic_dist_strength);
		}
	}
	else
	{
		if (m_basic_density)
		{
			cg.DensityField3D(dsize, stats_size,
				m_basic_diff, m_basic_iter,
				float(m_basic_thresh / scale),
				float(m_basic_falloff / scale2),
				float(m_basic_density_thresh / scale),
				scale);
		}
		else
		{
			cg.Grow3D(m_basic_diff, m_basic_iter,
				float(m_basic_thresh / scale),
				float(m_basic_falloff / scale2));
		}
	}

	if (clean_iter > 0)
		cg.Cleanup3D(clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorder3D(0.1);

	vd->GetVR()->clear_tex_current();
	m_view->RefreshGL();

	m_generate_prg->SetValue(100);
	//connection.disconnect();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->GetSettingDlg()->SetRunScript(true);
		vr_frame->GetMovieView()->GetScriptSettings();
	}
}

void ComponentDlg::SelectFullComp()
{
	//get id
	wxString str = m_comp_id_text->GetValue();
	if (str.empty())
	{
		if (!m_view)
			return;
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		FL::ComponentSelector comp_selector(vd);
		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		comp_selector.SetMaxNum(use, num);
		comp_selector.SetAnalyzer(&m_comp_analyzer);
		comp_selector.CompFull();
	}
	else
	{
		wxCommandEvent e;
		OnCompAppend(e);
	}

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnAnalyze(wxCommandEvent &event)
{
	Analyze(false);
}

void ComponentDlg::OnAnalyzeSel(wxCommandEvent &event)
{
	Analyze(true);
}

void ComponentDlg::Analyze(bool sel)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 2 + (m_consistent?1:0));
	m_prog = 0.0f;
	m_generate_prg->SetValue(0);

	boost::signals2::connection connection =
		m_comp_analyzer.m_sig_progress.connect(boost::bind(
		&ComponentDlg::UpdateProgress, this));

	m_comp_analyzer.SetVolume(vd);
	if (m_colocal)
	{
		m_comp_analyzer.ClearCoVolumes();
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
		{
			VolumeData* vdi = m_view->GetDispVolumeData(i);
			if (vdi != vd)
				m_comp_analyzer.AddCoVolume(vdi);
		}
	}
	m_comp_analyzer.Analyze(sel, m_consistent, m_colocal);

	if (m_consistent)
	{
		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_pool();
		m_view->RefreshGL();
	}

	if (m_comp_analyzer.GetListSize() > 10000)
	{
		wxFileDialog *fopendlg = new wxFileDialog(
			this, "Save Analysis Data", "", "",
			"Text file (*.txt)|*.txt",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int rval = fopendlg->ShowModal();
		if (rval == wxID_OK)
		{
			wxString filename = fopendlg->GetPath();
			string str = filename.ToStdString();
			m_comp_analyzer.OutputCompListFile(str, 1);
		}
		if (fopendlg)
			delete fopendlg;
	}
	else
	{
		string str;
		m_comp_analyzer.OutputCompListStr(str, 1);
		m_stat_text->SetValue(str);
	}

	m_generate_prg->SetValue(100);
	connection.disconnect();
}

void ComponentDlg::OnKey(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	case wxKeyCode('a'):
	case wxKeyCode('A'):
		if (event.ControlDown())
		{
			m_stat_text->SetSelection(-1, -1);
		}
		else
			event.Skip();
		break;

	default:
		event.Skip();
		break;
	}
}