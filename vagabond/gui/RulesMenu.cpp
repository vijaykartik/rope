// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "AddRule.h"
#include "RulesMenu.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/gui/elements/TextButton.h>

RulesMenu::RulesMenu(Scene *prev) : ListView(prev)
{
	_md = Environment::metadata();
	_md->ruler().setResponder(this);
}

RulesMenu::~RulesMenu()
{
	_md->ruler().setResponder(nullptr);
}

void RulesMenu::setup()
{
	addTitle("Rule list");

	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}

	ListView::setup();
}

size_t RulesMenu::lineCount() 
{
	return _md->ruler().ruleCount() + 1;
}

Renderable *RulesMenu::getLine(int i)
{
	if (i == 0)
	{
		TextButton *add = new TextButton("(+) new rule", this);
		add->setReturnTag("add");
		return add;
	}

	i--;

	Rule &r = _md->ruler().rule(i);
	TextButton *option = new TextButton(r.desc(), this);
	option->setReturnTag("rule_" + i_to_str(i));
	return option;
}

void RulesMenu::presentAddRule(AddRule *view)
{
	view->setCaller(this);
	view->setEntityId(_entity_id);
	view->setData(_group);
	view->show();
}

void RulesMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "add")
	{
		AddRule *view = new AddRule(this);
		presentAddRule(view);
	}

	std::string rule_prefix = "rule_";

	if (tag.rfind(rule_prefix, 0) != std::string::npos)
	{
		std::string index = tag.substr(rule_prefix.length(), 
		                                  std::string::npos);
		
		int idx = atoi(index.c_str());
		Rule &r = _md->ruler().rule(idx);
		_lastRule = &r;
		
		AddRule *view = new AddRule(this, &r);
		presentAddRule(view);
	}

	ListView::buttonPressed(tag, button);
}

void RulesMenu::replaceRule(Rule &r)
{
	if (_lastRule != nullptr)
	{
		*_lastRule = r;
		_lastRule = nullptr;
	}
}

void RulesMenu::objectsChanged()
{
	refresh();
}
