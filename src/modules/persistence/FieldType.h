/**
 * @file
 */

#pragma once

#include "core/Common.h"

namespace persistence {

// don't change the order without changing the string mapping
enum class FieldType {
	STRING,
	TEXT,
	LONG,
	INT,
	PASSWORD,
	TIMESTAMP,
	BOOLEAN,
	SHORT,
	BYTE,
	DOUBLE,
	MAX
};
static constexpr int MAX_FIELDTYPES = std::enum_value(FieldType::MAX);

extern FieldType toFieldType(const std::string& type);
extern const char* toFieldType(const FieldType type);

}
