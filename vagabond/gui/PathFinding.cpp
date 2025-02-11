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

#include "PathFinding.h"
#include "PathFinderView.h"
#include <vagabond/core/Entity.h>
#include <vagabond/core/PathFinder.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include "SpecificNetwork.h"
#include "AddRule.h"
#include "MapView.h"

PathFinding::PathFinding(Scene *prev, Entity *ent) : Scene(prev),
_all(ent->makeTorsionDataGroup())
{
	_entity = ent;
}

PathFinding::~PathFinding()
{
	
}

void PathFinding::setup()
{
	addTitle("Pathfinding for " + _entity->name());

	float top = 0.2;

	{
		Text *t = new Text("Choose subset of molecules using rule");
		t->setLeft(0.2, top);
		t->addAltTag("Devise a rule to limit pathfinding to molecule subset");
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("make_rule");
		t->setCentre(0.8, top);
		addObject(t);
	}
	
	top += 0.2;

	{
		Text *t = new Text("or start from existing saved space:");
		t->setLeft(0.2, top);
		t->addAltTag("Find a previously saved json file");
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("find_json");
		t->setCentre(0.8, top);
		addObject(t);
	}
	
	{
		TextButton *start = new TextButton("Start", this);
		start->setReturnTag("start");
		start->setRight(0.8, 0.8);
		addObject(start);
	}
}

void PathFinding::findJson()
{
	AskForText *aft = new AskForText(this, "Enter json filename to open", 
	                                 "json_filename", this);
	setModal(aft);
}

void PathFinding::loadSpace(std::string filename)
{
	json data;
	std::ifstream f;
	f.open(filename);
	f >> data;
	f.close();

	SpecificNetwork *spec = new SpecificNetwork(data);
	_prior = spec;
}

void PathFinding::makeRule()
{
	if (_rule == nullptr)
	{
		_rule = new Rule();
	}

	AddRule *ar = new AddRule(this, _rule);
	ar->setData(&_all);
	ar->setEntityId(_entity->name());
	ar->show();
}

void PathFinding::deleteRule()
{
	delete _rule;
	_rule = nullptr;
	refresh();
}

void PathFinding::start()
{
	if (_prior)
	{
		MapView *mv = new MapView(this, _prior);
		mv->show();
		return;
	}

	std::vector<Instance *> instances;

	if (_rule)
	{
		std::vector<HasMetadata *> hms = _all.subsetFromRule(*_rule);

		for (HasMetadata *hm : hms)
		{
			instances.push_back(static_cast<Instance *>(hm));
		}
	}
	else
	{
		instances = _entity->instances();
	}

	MapView *mv = new MapView(this, _entity, instances);
	mv->show();

}

void PathFinding::buttonPressed(std::string tag, Button *button)
{
	if (tag == "make_rule")
	{
		makeRule();
	}
	else if (tag == "delete_rule")
	{
		deleteRule();
	}
	else if (tag == "start")
	{
		start();
	}
	else if (tag == "find_json")
	{
		findJson();
	}
	else if (tag == "json_filename")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string text = te->scratch();
		loadSpace(text);
	}

	Scene::buttonPressed(tag, button);
}


void PathFinding::refresh()
{
	deleteTemps();

	if (_rule != nullptr)
	{
		if (_rule->header().length() == 0)
		{
			deleteRule();
			return;
		}

		int n = _all.subsetFromRule(*_rule).size();
		std::string num = std::to_string(n);

		TextButton *t = new TextButton(_rule->desc(), this);
		t->setRight(0.75, 0.3);
		t->addAltTag("found " + num + " molecules");
		t->setReturnTag("make_rule");
		addTempObject(t);

		ImageButton *ib = new ImageButton("assets/images/cross.png", this);
		ib->resize(0.06);
		ib->setRight(0.8, 0.3);
		ib->setReturnTag("delete_rule");
		addTempObject(ib);
	}
}
