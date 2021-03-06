/**
 * @file
 */

#pragma once

#include <gmock/gmock.h>
#include "persistence/PersistenceMgr.h"

namespace persistence {
class DBHandlerMock : public DBHandler {
public:
	MOCK_METHOD0(init, bool());
	MOCK_METHOD0(shutdown, void());
};

class PersistenceMgrMock : public PersistenceMgr {
public:
	PersistenceMgrMock() :
			PersistenceMgr(std::make_shared<DBHandlerMock>()) {
	}
	MOCK_METHOD0(init, bool());
	MOCK_METHOD0(shutdown, void());

	MOCK_METHOD2(registerSavable, bool(uint32_t, ISavable *));
	MOCK_METHOD2(unregisterSavable, bool(uint32_t, ISavable *));
};
}
