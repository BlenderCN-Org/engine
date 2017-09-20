#pragma once

#include "core/Common.h"
#include "core/Array.h"
#include "persistence/Model.h"

static const char *FieldTypeNames[] = {
	CORE_STRINGIFY(STRING),
	CORE_STRINGIFY(LONG),
	CORE_STRINGIFY(INT),
	CORE_STRINGIFY(PASSWORD),
	CORE_STRINGIFY(TIMESTAMP)
};
static_assert(lengthof(FieldTypeNames) == persistence::Model::MAX_FIELDTYPES, "Invalid field type mapping");

static const char *ConstraintTypeNames[] = {
	CORE_STRINGIFY(UNIQUE),
	CORE_STRINGIFY(PRIMARYKEY),
	CORE_STRINGIFY(AUTOINCREMENT),
	CORE_STRINGIFY(NOTNULL)
};
static_assert(lengthof(ConstraintTypeNames) == persistence::Model::MAX_CONSTRAINTTYPES, "Invalid constraint type mapping");