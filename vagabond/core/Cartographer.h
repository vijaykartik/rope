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

#ifndef __vagabond__Cartographer__
#define __vagabond__Cartographer__

#include <vector>
#include <atomic>
#include <map>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Responder.h>
#include "ScoreMap.h"


class Atom;
class Entity;
class Instance;
class Network;
class Parameter;
class ProblemPrep;
class AltersNetwork;
class SpecificNetwork;
class MappingToMatrix;
template <typename Type> class Mapped;
template <typename Type> class Mappable;

class Cartographer : public Responder<SpecificNetwork>, 
public Responder<AltersNetwork>,
public HasResponder<Responder<Cartographer>>
{
public:
	typedef std::vector<std::vector<float>> Points;

	Cartographer(Entity *entity, std::vector<Instance *> instances);
	
	SpecificNetwork *specified()
	{
		return _specified;
	}
	
	PCA::Matrix &matrix();
	
	Mapped<float> *mapped()
	{
		return _mapped;
	}
	
	MappingToMatrix *map2Matrix()
	{
		return _mat2Map;
	}
	
	void stopASAP()
	{
		_stop = true;
	}
	
	void skipCurrentJob()
	{
		_skip = true;
	}

	void makeMapping();
	void setup();

	static void flip(Cartographer *cg);
	static void nudge(Cartographer *cg, const std::vector<float> &point);
	static void assess(Cartographer *cg);
	static void flipIdx(Cartographer *cg, int i, int j);

	void supplyExisting(SpecificNetwork *spec);
protected:
	void sendObject(std::string tag, void *object);
private:
	int bestStartingPoint(std::vector<int> &ruled_out);
	Mappable<float> *bootstrapFace(std::vector<int> &pidxs);
	void hookReferences();

	void preparePoints(int idx);
	float scoreForTriangle(int idx, ScoreMap::Mode mode = ScoreMap::Basic);
	void checkTriangles(ScoreMap::Mode mode = ScoreMap::Basic);
	
	float scoreForPoints(const Cartographer::Points &points, 
	                     ScoreMap::Mode options);
	ScoreMap basicScorer(ScoreMap::Mode options);
	float scoreWithScorer(const Points &points, ScoreMap scorer);
	std::function<float()> scorerForNudge(int tidx);

	void flipPoints();
	void flipPoint(int i, int j);
	void flipPointsFor(Mappable<float> *face, const std::vector<int> &points);
	void flipGroup(Mappable<float> *face, int g, int pidx);

	void permute(std::vector<Parameter *> &params, 
	             std::function<float()> score, int pidx);

	void refineFace(Parameter *param, const std::vector<float> &point,
	                const std::function<float()> &score);
	void nudgePoints(const std::vector<float> &point);

	bool nudgePoint(float begin, Parameter *param, const int &pidx,
	                const std::function<float()> &score, bool old);
	Mappable<float> *extendFace(std::vector<int> &pidxs, int &tidx);
	Points cartesiansForFace(Mappable<float> *face, int paramCount);
	Points cartesiansForPoint(int pidx, int paramCount);
	void cartesiansForTriangle(int tidx, int paramCount,
	                           Cartographer::Points &carts);
	int pointToWork(Parameter *param, const std::vector<float> &point,
	                int last_idx);

	SpecificNetwork *_specified = nullptr;
	Network *_network = nullptr;
	std::vector<Instance *> _instances;
	Entity *_entity = nullptr;
	Mapped<float> *_mapped = nullptr;
	MappingToMatrix *_mat2Map = nullptr;
	PCA::Matrix _distMat{};
	
	struct PointsInTriangle
	{
		Points points;
		float score;
	};
	
	int worstTriangleScore();
	std::map<int, PointsInTriangle> _pointsInTriangles;
	ProblemPrep *_prepwork = nullptr;
	
	std::vector<Atom *> _atoms;
	std::atomic<bool> _stop{false};
	std::atomic<bool> _skip{false};
};

#endif
