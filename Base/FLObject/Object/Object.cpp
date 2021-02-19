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
#include <Object.hpp>

using namespace fluo;

Object::Object():
	EventHandler(),
	_id(0)
{
	_value_set = new ValueSet();
	setDefaultValueChangingFunction(
		std::bind(&Object::handleValueChanging,
			this, std::placeholders::_1));
	setDefaultValueChangedFunction(
		std::bind(&Object::handleValueChanged,
			this, std::placeholders::_1));
}

Object::Object(const Object& obj, const CopyOp& copyop, bool copy_values):
	EventHandler(),
	_id(0),
	m_name(obj.m_name)
{
	_value_set = new ValueSet();
	if (copy_values)
		copyValues(obj, copyop);
	setDefaultValueChangingFunction(
		std::bind(&Object::handleValueChanging,
			this, std::placeholders::_1));
	setDefaultValueChangedFunction(
		std::bind(&Object::handleValueChanged,
			this, std::placeholders::_1));
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(Event& event)
{
	Referenced* refd = event.sender;
	//remove observee
	removeObservee(refd);
}

void Object::handleEvent(Event& event)
{
	Referenced* refd = event.sender;
	if (!refd)
		return;

	Value* value = dynamic_cast<Value*>(refd);
	std::string value_name;
	if (value)
		value_name = value->getName();

	switch (event.type)
	{
	case Event::EVENT_VALUE_CHANGING:
		onValueChanging(value_name, event);
		break;
	case Event::EVENT_VALUE_CHANGED:
		onValueChanged(value_name, event);
		break;
	case Event::EVENT_NODE_ADDED:
		onNodeAdded(event);
		break;
	case Event::EVENT_NODE_REMOVED:
		onNodeRemoved(event);
		break;
	}
}

void Object::processNotification(Event& event)
{
	//handle event
	handleEvent(event);
	//notify observers
	if (event.type == Event::EVENT_VALUE_CHANGING)
		return;
	notifyObservers(event);
}
/*
//add functions
bool Object::addValue(ValueTuple &vt)
{
	if (_value_set)
	{
		bool result = _value_set->addValue(vt);
		if (result)
		{
			std::string name = std::get<0>(vt);
			Value* vs_value = _value_set->findValue(name);
			if (vs_value)
			{
				vs_value->addObserver(this);
				Event event;
				event.init(Event::EVENT_VALUE_ADDED, this, vs_value, true);
				vs_value->notify(event);
			}
		}
		return result;
	}
	return false;
}

//define function bodies first
#define OBJECT_ADD_VALUE_BODY \
	if (_value_set) \
	{ \
		bool result = _value_set->addValue(name, value); \
		if (result) \
		{ \
			Value* vs_value = _value_set->findValue(name); \
			if (vs_value) \
			{ \
				vs_value->addObserver(this); \
				Event event; \
				event.init(Event::EVENT_VALUE_ADDED, \
					this, vs_value, true); \
				vs_value->notify(event); \
			} \
		} \
		return result; \
	} \
	else \
		return false

//actual add functions
bool Object::addValue(const std::string &name, Referenced* value)
{
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

bool Object::addValue(const std::string &name, const fluo::Vector4f &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const fluo::Vector4i &value)
{
	OBJECT_ADD_VALUE_BODY;
}

//set functions
bool Object::setValue(ValueTuple &vt, const Event& event)
{
	ValueTuple old_vt;
	std::string name = std::get<0>(vt);
	std::get<0>(old_vt) = name;
	Value* value = getValue(name);
	if (getValue(old_vt) && vt != old_vt)
	{
		bool result = false;
		if (_value_set)
		{
			if (!event.sender)
				event.init(Event::EVENT_VALUE_CHANGING,
					this, value, true);
			result = _value_set->setValue(vt, event);
		}
		return result;
	}
	return false;
}

//define function bodies first
#define OBJECT_SET_VALUE_BODY \
	if (getValue(name, old_value) && value != old_value) \
	{ \
		bool result = false; \
		if (_value_set) \
		{ \
			if (!event.sender) \
				event.init(Event::EVENT_VALUE_CHANGING, \
					this, getValue(name), true); \
			else \
				event.push(this); \
			result = _value_set->setValue(name, value, event); \
			event.pop(); \
		} \
		return result; \
	} \
	return false

//set functions
bool Object::setValue(const std::string &name, Referenced* value, Event& event)
{
	Referenced* old_value;
	if (getValue(name, &old_value) && value != old_value)
	{
		bool result = false;
		if (_value_set)
		{
			if (!event.sender)
				event.init(Event::EVENT_VALUE_CHANGING,
					this, getValue(name), true);
			else
				event.push(this);
			result = _value_set->setValue(name, value, event);
			event.pop();
		}
		return result;
	}
	return false;
}

bool Object::setValue(const std::string &name, bool value, Event& event)
{
	bool old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, char value, Event& event)
{
	char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned char value, Event& event)
{
	unsigned char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, short value, Event& event)
{
	short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned short value, Event& event)
{
	unsigned short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long value, Event& event)
{
	long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long value, Event& event)
{
	unsigned long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long long value, Event& event)
{
	long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long long value, Event& event)
{
	unsigned long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, float value, Event& event)
{
	float old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, double value, Event& event)
{
	double old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::string &value, Event& event)
{
	std::string old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::wstring &value, Event& event)
{
	std::wstring old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Point &value, Event& event)
{
	fluo::Point old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Vector &value, Event& event)
{
	fluo::Vector old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::BBox &value, Event& event)
{
	fluo::BBox old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::HSVColor &value, Event& event)
{
	fluo::HSVColor old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Color &value, Event& event)
{
	fluo::Color old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Plane &value, Event& event)
{
	fluo::Plane old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::PlaneSet &value, Event& event)
{
	fluo::PlaneSet old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Quaternion &value, Event& event)
{
	fluo::Quaternion old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Ray &value, Event& event)
{
	fluo::Ray old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Transform &value, Event& event)
{
	fluo::Transform old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Vector4f &value, Event& event)
{
	fluo::Vector4f old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const fluo::Vector4i &value, Event& event)
{
	fluo::Vector4i old_value;
	OBJECT_SET_VALUE_BODY;
}
*/
//toggle value for bool
bool Object::toggleValue(const std::string &name, bool &value, Event& event)
{
	bool result = false;
	if (_value_set)
	{
		if (!event.sender)
			event.init(Event::EVENT_VALUE_CHANGING,
                this, getValuePointer(name), true);
		else
			event.push(this);
		result = _value_set->toggleValue(name, value, event);
		event.pop();
	}
	return result;
}

/*
//get functions
bool Object::getValue(ValueTuple &vt)
{
	if (_value_set)
		return _value_set->getValue(vt);
	else
		return false;
}

//define function bodies first
#define OBJECT_GET_VALUE_BODY \
	if (_value_set) \
		return _value_set->getValue(name, value); \
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

bool Object::getValue(const std::string &name, fluo::Vector4f &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, fluo::Vector4i &value)
{
	OBJECT_GET_VALUE_BODY;
}
*/
//sync a value
//observer's value updates when this updates
bool Object::syncValue(const std::string &name, Object* obj)
{
	if (obj)
	{
        Value* value = getValuePointer(name);
        Value* value2 = obj->getValuePointer(name);
		if (value && value2)
		{
			value->addObserver(value2);
			return true;
		}
	}
	return false;
}

//unsync a value
bool Object::unsyncValue(const std::string &name, Object* obj)
{
	if (obj)
	{
        Value* value = getValuePointer(name);
        Value* value2 = obj->getValuePointer(name);
		if (value && value2)
		{
			value->removeObserver(value2);
			return true;
		}
	}
	return false;
}

//sync a list of values
bool Object::syncValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= syncValue(*it, obj);
	}
	return result;
}

//unsync a list of values
bool Object::unsyncValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= unsyncValue(*it, obj);
	}
	return result;
}

//sync all values
bool Object::syncAllValues(Object* obj)
{
	bool result = false;
	std::string name;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				name = it->second->getName();
				result |= syncValue(name, obj);
			}
		}
	}
	return result;
}

//unsync all values
bool Object::unsyncAllValues(Object* obj)
{
	bool result = false;
	std::string name;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				name = it->second->getName();
				result |= unsyncValue(name, obj);
			}
		}
	}
	return result;
}

//propagate a value
bool Object::propValue(const std::string &name, Object* obj)
{
    Value* value = getValuePointer(name);
	if (value)
	{
        Value* obj_value = obj->getValuePointer(name);
		if (obj_value)
		{
			Event event;
			event.init(Event::EVENT_SYNC_VALUE,
				value, value, true);
			obj_value->sync(event);
		}
		return true;
	}
	return false;
}

//propagate a list of values
bool Object::propValues(const ValueCollection &names, Object* obj)
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
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
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

//sync values belonging to the same object (mutual! hope this is not confusing)
bool Object::syncValues(const std::string &name1, const std::string &name2)
{
    Value* value1 = getValuePointer(name1);
    Value* value2 = getValuePointer(name2);
	if (value1 && value2 &&
		value1->getType() ==
		value2->getType())
	{
		value1->addObserver(value2);
		value2->addObserver(value1);
		return true;
	}
	return false;
}

bool Object::unsyncValues(const std::string &name1, const std::string &name2)
{
    Value* value1 = getValuePointer(name1);
    Value* value2 = getValuePointer(name2);
	if (value1 && value2)
	{
		value1->removeObserver(value2);
		value2->removeObserver(value1);
		return true;
	}
	return false;
}

bool Object::syncValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it1 = names.begin();
		it1 != names.end(); ++it1)
	{
		for (auto it2 = std::next(it1);
			it2 != names.end(); ++it2)
			result |= syncValues(*it1, *it2);
	}
	return result;
}

bool Object::unsyncValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it1 = names.begin();
		it1 != names.end(); ++it1)
	{
		for (auto it2 = std::next(it1);
			it2 != names.end(); ++it2)
			result |= unsyncValues(*it1, *it2);
	}
	return result;
}

//propagate values belonging to the same object (1 -> 2)
bool Object::propValues(const std::string &name1, const std::string &name2)
{
    Value* value1 = getValuePointer(name1);
    Value* value2 = getValuePointer(name2);
	if (value1 && value2 &&
		value1 != value2)
	{
		Event event;
		event.init(Event::EVENT_SYNC_VALUE,
			value1, value1, true);
		value2->sync(event);
		return true;
	}
	return false;
}

bool Object::propValues(const std::string &name1, const ValueCollection &names)
{
	bool result = false;
    Value* value1 = getValuePointer(name1);
	if (!value1)
		return result;
	Event event;
	event.init(Event::EVENT_SYNC_VALUE,
		value1, value1, true);
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
        Value* value2 = getValuePointer(*it);
		if (value2 && value2 != value1)
			result |= value2->sync(event);
	}
	return result;
}
