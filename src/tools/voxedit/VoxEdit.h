/**
 * @file
 */

#pragma once

#include "ui/turbobadger/UIApp.h"
#include "video/MeshPool.h"
#include "ui/VoxEditWindow.h"
#include "core/Array.h"
#include "SceneManager.h"

/**
 * @brief This is a voxel editor that can import and export multiple mesh/voxel formats.
 *
 * @ingroup Tools
 */
class VoxEdit: public ui::turbobadger::UIApp {
private:
	using Super = ui::turbobadger::UIApp;
	voxedit::VoxEditWindow* _mainWindow;
	video::MeshPoolPtr _meshPool;
	SceneManager _sceneMgr;

public:
	VoxEdit(const metric::MetricPtr& metric, const io::FilesystemPtr& filesystem, const core::EventBusPtr& eventBus, const core::TimeProviderPtr& timeProvider, const video::MeshPoolPtr& meshPool);

	bool importheightmapFile(const std::string& file);
	bool saveFile(const std::string& file);
	bool loadFile(const std::string& file);
	bool screenshotFile(const std::string& file);
	bool prefabFile(const std::string& file);
	bool importFile(const std::string& file);
	bool exportFile(const std::string& file);
	bool newFile(bool force = false);

	SceneManager& sceneMgr();
	const SceneManager& sceneMgr() const;

	video::MeshPoolPtr meshPool() const;

	core::AppState onConstruct() override;
	core::AppState onInit() override;
	core::AppState onCleanup() override;
	core::AppState onRunning() override;
};

inline SceneManager& VoxEdit::sceneMgr() {
	return _sceneMgr;
}

inline const SceneManager& VoxEdit::sceneMgr() const {
	return _sceneMgr;
}

inline video::MeshPoolPtr VoxEdit::meshPool() const {
	return _meshPool;
}
