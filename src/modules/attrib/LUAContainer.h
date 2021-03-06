/**
 * @file
 */

#pragma once

#include <memory>
#include <string>

namespace attrib {

class ContainerProvider;

class LUAContainer {
private:
	std::string _name;
	ContainerProvider* _ctx;
	Values _percentage;
	Values _absolute;
public:
	LUAContainer(const std::string& name, ContainerProvider* ctx) :
			_name(name), _ctx(ctx) {
	}

	void addPercentage(Type type, double value) {
		auto i = _percentage.insert(std::make_pair(type, value));
		if (!i.second) {
			_percentage.erase(type);
			_percentage.insert(std::make_pair(type, value));
		}
	}

	void addAbsolute(Type type, double value) {
		auto i = _absolute.insert(std::make_pair(type, value));
		if (!i.second) {
			_absolute.erase(type);
			_absolute.insert(std::make_pair(type, value));
		}
	}

	inline bool registered() const {
		return _name.empty();
	}

	void createContainer() {
		_ctx->addContainer(std::make_shared<Container>(_name, _percentage, _absolute));
		_percentage.clear();
		_absolute.clear();
		_name.clear();
	}

	inline const std::string& name() const {
		return _name;
	}
};

}
