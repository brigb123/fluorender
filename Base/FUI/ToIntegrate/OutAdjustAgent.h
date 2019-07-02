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
#ifndef _OUTADJUSTAGENT_H_
#define _OUTADJUSTAGENT_H_

#include <Fui/InterfaceAgent.h>
#include <Scenegraph/Node.h>

namespace FUI
{
	class OutAdjustPanel;
	class AgentFactory;
	class OutAdjustAgent : public InterfaceAgent
	{
	public:
		OutAdjustAgent(OutAdjustPanel &panel);

		virtual bool isSameKindAs(const FL::Object* obj) const
		{
			return dynamic_cast<const OutAdjustAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "OutAdjustAgent"; }

		virtual void setObject(FL::Node* vd);
		virtual FL::Node* getObject();

		virtual void UpdateAllSettings();

		friend class AgentFactory;

	protected:
		OutAdjustPanel &panel_;

		//update functions
		void OnGammaRChanged(FL::Event& event);
		void OnGammaGChanged(FL::Event& event);
		void OnGammaBChanged(FL::Event& event);
		void OnBrightnessRChanged(FL::Event& event);
		void OnBrightnessGChanged(FL::Event& event);
		void OnBrightnessBChanged(FL::Event& event);
		void OnEqualizeRChanged(FL::Event& event);
		void OnEqualizeGChanged(FL::Event& event);
		void OnEqualizeBChanged(FL::Event& event);
	};
}

#endif//_OUTADJUSTAGENT_H_