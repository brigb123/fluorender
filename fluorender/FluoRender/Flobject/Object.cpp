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
#include <Flobject/Object.h>

using namespace flrd;

Object::Object():
	Referenced(),
	_id(0)
{
	ValueSet* value_set = new ValueSet();
	_vs_stack.push(value_set);
}

Object::Object(const Object& obj, const CopyOp& copyop):
	Referenced(),
	_id(0),
	m_name(obj.m_name)
{
	if (copyop.getCopyFlags() & CopyOp::SHALLOW_COPY)
		_vs_stack.push(obj._vs_stack.top());
	else
	{
		_vs_stack.push(obj._vs_stack.top()->clone(copyop));
		//also observe the values
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			it->second->addObserver(this);
		}
	}
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(void* ptr)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	if (refd)
		_vs_stack.top()->resetRefPtr(refd);

	//remove observee
	removeObservee(refd);
}

void Object::objectChanged(void* ptr, const std::string &exp)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	if (refd->className() == std::string("Value"))
	{
		_vs_stack.top()->syncValue(dynamic_cast<Value*>(refd));
	}
	else if (refd->className() == std::string("Object"))
	{
		//do something in response
		
	}
}

//add functions
bool Object::addValue(ValueTuple &vt)
{
	if (_vs_stack.top())
	{
		bool result = _vs_stack.top()->addValue(vt);
		if (result)
		{
			std::string name = std::get<0>(vt);
			Value* vs_value = _vs_stack.top()->findValue(name);
			if (vs_value)
				vs_value->addObserver(this);
		}
		return result;
	}
	return false;
}

//define function bodies first
#define OBJECT_ADD_VALUE_BODY \
	if (_vs_stack.top()) \
	{ \
		bool result = _vs_stack.top()->addValue(name, value); \
		if (result) \
		{ \
			Value* vs_value = _vs_stack.top()->findValue(name); \
			if (vs_value) \
				vs_value->addObserver(this); \
		} \
		return result; \
	} \
	else \
		return false

//actual add functions
bool Object::addValue(const std::string &name, Referenced* value)
{
	if (value) value->addObserver(this);
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, bool value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, char value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned char value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, short value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned short value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, long long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned long long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, float value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, double value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const std::string &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const std::wstring &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Point &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Vector &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::BBox &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::HSVColor &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Color &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Plane &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::PlaneSet &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Quaternion &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Ray &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Transform &value)
{
	OBJECT_ADD_VALUE_BODY;
}

//define function bodies first
bool Object::setValue(ValueTuple &vt, bool notify)
{
	ValueTuple old_vt;
	std::string name = std::get<0>(vt);
	std::get<0>(old_vt) = name;
	if (getValue(old_vt) && vt != old_vt)
	{
		bool result = false;
		if (_vs_stack.top())
		{
			result = _vs_stack.top()->setValue(vt, notify);
			if (result)
				notifyObserversOfChange(name);
		}
		return result;
	}
	return false;
}

//set functions
#define OBJECT_SET_VALUE_BODY \
	if (getValue(name, old_value) && value != old_value) \
	{ \
		bool result = false; \
		if (_vs_stack.top()) \
		{ \
			result = _vs_stack.top()->setValue(name, value, notify); \
			if (result) \
				notifyObserversOfChange(name); \
		} \
		return result; \
	} \
	return false

//set functions
bool Object::setValue(const std::string &name, Referenced* value, bool notify)
{
	Referenced* old_value;
	if (getValue(name, &old_value) && value != old_value)
	{
		if (old_value)
			old_value->removeObserver(this);
		if (value)
			value->addObserver(this);
		bool result = false;
		if (_vs_stack.top())
		{
			result = _vs_stack.top()->setValue(name, value, notify);
			if (result)
				notifyObserversOfChange();
		}
		return result;
	}
	return false;
}

bool Object::setValue(const std::string &name, bool value, bool notify)
{
	bool old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, char value, bool notify)
{
	char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned char value, bool notify)
{
	unsigned char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, short value, bool notify)
{
	short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned short value, bool notify)
{
	unsigned short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long value, bool notify)
{
	long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long value, bool notify)
{
	unsigned long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long long value, bool notify)
{
	long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long long value, bool notify)
{
	unsigned long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, float value, bool notify)
{
	float old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, double value, bool notify)
{
	double old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::string &value, bool notify)
{
	std::string old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::wstring &value, bool notify)
{
	std::wstring old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Point &value, bool notify)
{
	fluo::Point old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Vector &value, bool notify)
{
	fluo::Vector old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::BBox &value, bool notify)
{
	fluo::BBox old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::HSVColor &value, bool notify)
{
	fluo::HSVColor old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Color &value, bool notify)
{
	fluo::Color old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Plane &value, bool notify)
{
	fluo::Plane old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::PlaneSet &value, bool notify)
{
	fluo::PlaneSet old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Quaternion &value, bool notify)
{
	fluo::Quaternion old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Ray &value, bool notify)
{
	fluo::Ray old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Transform &value, bool notify)
{
	fluo::Transform old_value;
	OBJECT_SET_VALUE_BODY;
}

//toggle value for bool
bool Object::toggleValue(const std::string &name, bool &value, bool notify)
{
	bool result = false;
	if (_vs_stack.top())
		result = _vs_stack.top()->toggleValue(name, value, notify);
	if (result)
		notifyObserversOfChange(name);
	return result;
}

//get functions
bool Object::getValue(ValueTuple &vt)
{
	if (_vs_stack.top())
		return _vs_stack.top()->getValue(vt);
	else
		return false;
}

//define function bodies first
#define OBJECT_GET_VALUE_BODY \
	if (_vs_stack.top()) \
		return _vs_stack.top()->getValue(name, value); \
	else \
		return false

//get functions
bool Object::getValue(const std::string &name, Referenced** value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, bool &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, char &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned char &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, short &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned short &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, long long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned long long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, float &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, double &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, std::string &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, std::wstring &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Point &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Vector &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::BBox &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::HSVColor &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Color &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Plane &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::PlaneSet &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Quaternion &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Ray &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Transform &value)
{
	OBJECT_GET_VALUE_BODY;
}

//sync a value
//observer's value updates when this updates
bool Object::syncValue(const std::string &name, Observer* obsrvr)
{
	Value* value = getValue(name);
	if (value)
	{
		value->addObserver(obsrvr);
		return true;
	}
	return false;
}

//unsync a value
bool Object::unsyncValue(const std::string &name, Observer* obsrvr)
{
	Value* value = getValue(name);
	if (value)
	{
		value->removeObserver(obsrvr);
		return true;
	}
	return false;
}

//sync a list of values
bool Object::syncValues(const std::vector<std::string> &names, Observer* obsrvr)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= syncValue(*it, obsrvr);
	}
	return result;
}

//unsync a list of values
bool Object::unsyncValues(const std::vector<std::string> &names, Observer* obsrvr)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= unsyncValue(*it, obsrvr);
	}
	return result;
}

//sync all values
bool Object::syncAllValues(Observer* obsrvr)
{
	bool result = false;
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			if (it->second)
			{
				it->second->addObserver(obsrvr);
				result = true;
			}
		}
	}
	return result;
}

//unsync all values
bool Object::unsyncAllValues(Observer* obsrvr)
{
	bool result = false;
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			if (it->second)
			{
				it->second->removeObserver(obsrvr);
				result = true;
			}
		}
	}
	return result;
}

//propagate a value
bool Object::propValue(const std::string &name, Object* obj)
{
	Value* value = getValue(name);
	if (value)
	{
		obj->objectChanged(value, "");
		return true;
	}
	return false;
}

//propagate a list of values
bool Object::propValues(const std::vector<std::string> &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= propValue(*it, obj);
	}
	return result;
}

//propagate all values
bool Object::propAllValues(Object* obj)
{
	bool result = false;
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			if (it->second)
			{
				propValue(it->second->getName(), obj);
				result = true;
			}
		}
	}
	return result;
}

std::vector<std::string> Object::getValueNames()
{
	std::vector<std::string> result;
	if (_vs_stack.top())
	{
		ValueSet::Values values = _vs_stack.top()->getValues();
		for (auto it = values.begin();
			it != values.end(); ++it)
		{
			result.push_back((*it).first);
		}
	}
	return result;
}