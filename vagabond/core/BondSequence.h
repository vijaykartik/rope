// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__BondSequence__
#define __vagabond__BondSequence__

#include <map>
#include <mutex>
#include <vector>
#include <climits>
#include "../utils/glm_import.h"
#include <vagabond/utils/version.h>
#include "AtomPosMap.h"
#include "programs/RingProgram.h"
#include "HasBondSequenceCustomisation.h"
#include "BondSequenceHandler.h"
#include <vagabond/utils/Vec3s.h>
#include "BondTorsion.h"
#include "AtomBlock.h"
#include "Grapher.h"
#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include "Sampler.h"

class Atom;
class Sampler;
class BondSequenceHandler;

class MiniJobSeq;


class BondSequence: public HasBondSequenceCustomisation
{
public:
	BondSequence(BondSequenceHandler *handler = nullptr);
	~BondSequence();
	
	BondCalculator *calculator();
	
	const size_t addedAtomsCount() const
	{
		return _grapher.atoms().size();
	}
	
	const std::vector<Atom *> &addedAtoms() const
	{
		return _grapher.atoms();
	}
	
	std::map<std::string, int> elementList() const;
	
	Atom *atomForBlock(int i) const
	{
		return _blocks[i].atom;
	}
	
	int numBondsForBlock(int i)
	{
		return _blocks[i].nBonds;
	}
	
	const size_t blockCount() const
	{
		return _blocks.size();
	}
	
	void wipe();
	
	glm::vec3 positionForPreviousBlock(int i)
	{
		return _blocks[i].parent_position();
	}
	
	glm::vec3 positionForAtomBlock(int i)
	{
		return _blocks[i].my_position();
	}
	
	glm::vec3 positionForNextBlock(int i, int j)
	{
		return _blocks[i].child_position(j);
	}
	
	void setSampler(Sampler *sampler)
	{
		_sampler = sampler;
	}
	
	const Grapher &grapher() const
	{
		return _grapher;
	}
	
	void cleanUpToIdle();
	void beginJob(Job *job);
	void removeTorsionBasis();
	
	const size_t maxDepth() const
	{
		return _maxDepth;
	}
	
	void setTorsionBasisType(TorsionBasis::Type type)
	{
		_basisType = type;
	}
	
	TorsionBasis *torsionBasis() const
	{
		return _torsionBasis;
	}
	
	Job *job()
	{
		return _job;
	}

	/* extend the atom graph for bond sequence generation
	 * 	@param atom beginning anchor atom
	 * 	@param count maximum bond depth to consider */
	void addToGraph(AnchorExtension &atom);
	
	void multiplyUpBySampleCount();
	void reflagDepth(int min, int max, int sidemax);
	
	const size_t flagged() const;

	std::vector<bool> activeParameterMask(size_t *programs);
	
	void prepareForIdle();

	void reset()
	{
		_state = SequenceInPreparation;
	}

	void calculate();
	void superpose();
	const AtomPosMap &extractPositions();
	const AtomPosList &extractVector();
	
	struct ElePos
	{
		glm::vec3 pos;
		char element[3];
	};

	std::vector<ElePos> extractForMap();
	double calculateDeviations();

	void setSampleCount(int count)
	{
		_sampleCount = count;
	}
	
	const size_t sampleCount() const
	{
		return _sampleCount;
	}
	
	SequenceState state();
	void signal(SequenceState newState);
	
	std::vector<AtomBlock> &blocks()
	{
		return _blocks;
	}
private:

	struct AtomBlockTodo
	{
		AtomBlock *block;
		int idx;
		int num;
	};

	
	bool atomGraphChildrenOnlyHydrogens(AtomGraph &g);
	void markHydrogenGraphs();
	
	std::vector<AtomBlock> _blocks;

	void generateBlocks();
	void acquireCustomVector(int sampleNum);
	void makeTorsionBasis();
	void fastCalculate();
	void prewarnPositionSampler();
	void prewarnTorsions();

	int calculateBlock(int idx);
	float fetchTorsion(int idx);
	void fetchAtomTarget(int idx);
	Coord::Interpolate<float> getTorsionFunction(int idx);

	Grapher _grapher;
	std::map<Atom *, AtomGraph *> _atom2Graph;

	std::map<Atom *, int> _atom2Block;

	size_t _sampleCount = 1;
	size_t _addedAtomsCount = 0;
	SequenceState _state = SequenceInPreparation;
	
	void setJob(Job *job);
	void printBlock(int idx);
	
	bool _fullRecalc = true;
	int _sampleNum = 0;
	int _startCalc = 0;
	int _endCalc = INT_MAX;
	int _maxDepth = 0;
	int _customIdx = 0;
	int _blocksDone = 0;
	int _singleSequence = 0;
	CustomVector *_custom = nullptr;
	void checkCustomVectorSizeFits();
	
	Coord::Get _acquireCoord;
	int _nCoord = 0;

	Job *_job = nullptr;
	BondSequenceHandler *_handler = nullptr;
	TorsionBasis *_torsionBasis = nullptr;
	TorsionBasis::Type _basisType;
	Sampler *_sampler = nullptr;
	std::vector<RingProgram> _programs;

#ifdef VERSION_PROLINE
	bool _usingPrograms = true;
#else
	bool _usingPrograms = false;
#endif

	bool _programsInitialised = false;
	
	bool _acceptablePositions = false;
	Vec3s _atomPositions;

	AtomPosMap _posAtoms;
	AtomPosList _posList;
	
	struct Torsioner
	{
		Coord::Interpolate<float> get_torsion;
		Coord::NeedsUpdate needs_update;
	};
	std::map<int, std::map<int, Torsioner>> _saveIdxFunc;
};

#endif
