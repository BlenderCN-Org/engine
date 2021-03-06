/**
 * @file
 */

#include "TriggerCooldown.h"
#include "core/Common.h"
#include "backend/entity/Npc.h"

namespace backend {

TriggerCooldown::TriggerCooldown(const std::string& name, const std::string& parameters, const ai::ConditionPtr& condition) :
		Task(name, parameters, condition) {
	_cooldownId = cooldown::getType(parameters);
	core_assert_always(_cooldownId != cooldown::Type::NONE);
}

ai::TreeNodeStatus TriggerCooldown::doAction(backend::AICharacter& chr, int64_t deltaMillis) {
	backend::Npc& npc = chr.getNpc();
	const cooldown::CooldownTriggerState s = npc.cooldownMgr().triggerCooldown(_cooldownId);
	if (s == cooldown::CooldownTriggerState::SUCCESS) {
		return ai::TreeNodeStatus::FINISHED;
	}
	return ai::TreeNodeStatus::FAILED;
}

}
