/**
 * @file
 */

#pragma once

#include "Task.h"
#include "cooldown/CooldownType.h"

namespace backend {

/**
 * @ingroup AI
 */
class TriggerCooldown: public Task {
private:
	cooldown::Type _cooldownId;
public:
	TriggerCooldown(const std::string& name, const std::string& parameters, const ai::ConditionPtr& condition);
	NODE_FACTORY(TriggerCooldown)

	ai::TreeNodeStatus doAction(backend::AICharacter& chr, int64_t deltaMillis) override;
};

}

