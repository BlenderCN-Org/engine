/**
 * @file
 */

#include "AI.h"

namespace ai {

TreeNodePtr AI::setBehaviour(const TreeNodePtr& newBehaviour) {
	TreeNodePtr current = _behaviour;
	_behaviour = newBehaviour;
	_reset = true;
	return current;
}

void AI::update(int64_t dt, bool debuggingActive) {
	if (isPause()) {
		return;
	}

	if (_character) {
		_character->update(dt, debuggingActive);
	}

	if (_reset) {
		// safe to do it like this, because update is not called from multiple threads
		_reset = false;
		_lastStatus.clear();
		_lastExecMillis.clear();
		_filteredEntities.clear();
		_selectorStates.clear();
	}

	_debuggingActive = debuggingActive;
	_time += dt;
	_aggroMgr.update(dt);
}

}
