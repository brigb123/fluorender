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
#include <Fui/ListModel.h>
#include <Global/Global.h>

using namespace FUI;

ListModel::ListModel()
{

}

void ListModel::nodeAdded(FL::Event& event)
{
	if (event.child)
	{
		wxDataViewItem parent_item = wxDataViewItem(0);
		wxDataViewItem child_item = wxDataViewItem(event.child);
		ItemAdded(parent_item, child_item);
	}
}

void ListModel::nodeRemoved(FL::Event& event)
{
	if (event.child)
	{
		wxDataViewItem parent_item = wxDataViewItem(0);
		wxDataViewItem child_item = wxDataViewItem(event.child);
		ItemDeleted(parent_item, child_item);
	}
}

int ListModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	//return wxDataViewModel::Compare(item1, item2, column, ascending);
	wxVariant var1, var2;
	GetValue(var1, item1, column);
	GetValue(var2, item2, column);
	wxString str1, str2;
	switch (column)
	{
	case 0:
	{
		wxDataViewIconText it1, it2;
		it1 << var1;
		it2 << var2;
		str1 = it1.GetText();
		str2 = it2.GetText();
	}
		break;
	default:
		str1 = var1.GetString();
		str2 = var2.GetString();
	}
	return ascending ? str1.CmpNoCase(str2) : str2.CmpNoCase(str1);
}

//model definition
unsigned int ListModel::GetColumnCount() const
{
	return 3;
}

wxString ListModel::GetColumnType(unsigned int col) const
{
	switch (col)
	{
	case 0:
		return wxDataViewIconTextRenderer::GetDefaultType();
	case 1:
		return "string";
	case 2:
		return "string";
	}
	return "string";
}

void ListModel::GetValue(wxVariant &variant,
	const wxDataViewItem &item, unsigned int col) const
{
	//if (!item.IsOk())
	//	return;
	FL::Node* node = (FL::Node*)item.GetID();
	if (!node)
		return;
	switch (col)
	{
	case 0:
		variant << wxDataViewIconText(
			wxString(node->getName()),
			FL::Global::instance().getIconList().
			get(node->className()));
		break;
	case 1:
		variant = wxString(node->className());
		break;
	case 2:
	{
		std::wstring path;
		node->getValue("data path", path);
		variant = wxString(path);
		break;

	}
	}
}

bool ListModel::SetValue(const wxVariant &variant,
	const wxDataViewItem &item, unsigned int col)
{
	if (!item.IsOk())
		return false;
	FL::Node* node = (FL::Node*)item.GetID();
	if (!node)
		return false;
	switch (col)
	{
	case 0:
		{
			wxDataViewIconText it;
			it << variant;
			node->setName(it.GetText().ToStdString());
		}
		return true;
	case 1:
		return false;
	case 2:
		return false;
	}
	return false;
}


bool ListModel::IsEnabled(const wxDataViewItem &item,
	unsigned int col) const
{
	return true;
}

bool ListModel::IsContainer(const wxDataViewItem &item) const
{
	return false;
}

bool ListModel::HasContainerColumns(const wxDataViewItem & item) const
{
	return true;
}

wxDataViewItem ListModel::GetParent(const wxDataViewItem &item) const
{
	return wxDataViewItem(0);
}

unsigned int ListModel::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
	FL::Node *node = (FL::Node*)parent.GetID();
	if (!node)
	{
		size_t size = FL::Global::instance().getNum();
		for (size_t i = 0; i < size; ++i)
		{
			FL::Object* obj = FL::Global::instance().get(i);
			array.Add(wxDataViewItem((void*)obj));
		}
		return size;
	}
	else
		return 0;
}

void ListModel::setObject(FL::Node* root)
{
	FL::Global::instance().addListObserver(this);
}

