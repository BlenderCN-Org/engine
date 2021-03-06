/**
 * @file
 */

#include "AbstractBenchmark.h"
#include "core/Log.h"
#include "core/Var.h"
#include "core/command/Command.h"

namespace core {

void AbstractBenchmark::SetUp(benchmark::State& st) {
	const core::EventBusPtr eventBus = std::make_shared<core::EventBus>();
	const io::FilesystemPtr filesystem = std::make_shared<io::Filesystem>();
	const core::TimeProviderPtr timeProvider = std::make_shared<core::TimeProvider>();
	const metric::MetricPtr& metric = std::make_shared<metric::Metric>();
	_benchmarkApp = new BenchmarkApp(metric, filesystem, eventBus, timeProvider, this);
}

void AbstractBenchmark::TearDown(benchmark::State& st) {
	// prevent cvars from begin saved and reloaded for the next fiture in the test
	core::Var::shutdown();
	delete _benchmarkApp;
	_benchmarkApp = nullptr;
}

AbstractBenchmark::BenchmarkApp::BenchmarkApp(const metric::MetricPtr& metric, const io::FilesystemPtr& filesystem, const core::EventBusPtr& eventBus, const core::TimeProviderPtr& timeProvider, AbstractBenchmark* benchmark) :
		Super(metric, filesystem, eventBus, timeProvider), _benchmark(benchmark) {
	init(ORGANISATION, "benchmark");
	while (_curState < AppState::Running) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
}

AppState AbstractBenchmark::BenchmarkApp::onCleanup() {
	AppState state = Super::onCleanup();
	_benchmark->onCleanupApp();
	return state;
}

AppState AbstractBenchmark::BenchmarkApp::onInit() {
	AppState state = Super::onInit();
	if (state != core::AppState::Running) {
		return state;
	}

	if (!_benchmark->onInitApp()) {
		return AppState::InitFailure;
	}

	return state;
}

AbstractBenchmark::BenchmarkApp::~BenchmarkApp() {
	while (AppState::InvalidAppState != _curState) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
}

}
