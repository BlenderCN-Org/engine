/**
 * @file
 */

#include "core/JSON.h"
#include "core/Log.h"
#include "core/Color.h"
#include "core/UUID.h"
#include "core/Var.h"
#include "core/App.h"
#include "TrazeTypes.h"
#include "TrazeEvents.h"
#include "TrazeProtocol.h"
#include "voxel/polyvox/Region.h"
#include "voxel/MaterialColor.h"
#include "voxel/polyvox/RawVolume.h"
#include "voxel/polyvox/Voxel.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace traze {

Protocol::Protocol(const core::EventBusPtr& eventBus) :
		_eventBus(eventBus) {
}

bool Protocol::init() {
	if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
		return false;
	}

	_clientToken = core::generateUUID(); // + "_" + core::App::getInstance()->appname();
	Log::debug("Client token: %s", _clientToken.c_str());

	_mosquitto = mosquitto_new(_clientToken.c_str(), true, this);
	if (_mosquitto == nullptr) {
		Log::error("Failed to create mosquitto instance");
		return false;
	}

	mosquitto_message_callback_set(_mosquitto, [] (struct mosquitto *, void *userdata, const struct mosquitto_message *msg) { ((Protocol*)userdata)->onMessage(msg); });
	mosquitto_connect_callback_set(_mosquitto, [] (struct mosquitto *, void *userdata, int rc) {
		uint8_t s = (uint8_t)rc;
		ConnectState state = (ConnectState)s;
		if (s > ConnectState::MaxKnown) {
			state = ConnectState::Unknown;
		}
		((Protocol*)userdata)->onConnect(state);
	});

	return true;
}

bool Protocol::connect() {
	if (_connected) {
		return true;
	}
	const char *host = core::Var::getSafe("mosquitto_host")->strVal().c_str();
	const int port = core::Var::getSafe("mosquitto_port")->intVal();
	Log::info("Trying to connect to %s at port %i...", host, port);
	const int rc = mosquitto_connect_async(_mosquitto, host, port, 0);
	if (rc != MOSQ_ERR_SUCCESS) {
		Log::error("Failed to connect to the mqtt broker %s", mosquitto_strerror(rc));
		return false;
	}

	mosquitto_loop_start(_mosquitto);
	return true;
}

void Protocol::shutdown() {
	if (_mosquitto) {
		mosquitto_disconnect(_mosquitto);
		mosquitto_loop_stop(_mosquitto, false);
		mosquitto_destroy(_mosquitto);
		_mosquitto = nullptr;
	}
	mosquitto_lib_cleanup();
}

bool Protocol::unsubscribe() {
	bool unsubscribed = true;
	for (const char* topic : {"traze/+/grid", "traze/+/players", "traze/+/ticker"}) {
		const int rc = mosquitto_unsubscribe(_mosquitto, nullptr, topic);
		if (rc != MOSQ_ERR_SUCCESS) {
			Log::warn("Failed to unsubscribe from topic %s with error %s", topic, mosquitto_strerror(rc));
			unsubscribed = false;
		} else {
			Log::debug("Unsubscribed from topic %s", topic);
		}
	}
	_subscribed = false;
	_instanceName = "";
	return unsubscribed;
}

bool Protocol::subscribe(const GameInfo& game) {
	if (_subscribed) {
		return true;
	}
	bool subscribed = true;
	const std::string& privateChannel = core::string::format("player/%s", _clientToken.c_str());
	for (const char* topic : {"grid", "players", "ticker", privateChannel.c_str()}) {
		char topicBuf[128];
		SDL_snprintf(topicBuf, sizeof(topicBuf), "traze/%s/%s", game.name.c_str(), topic);
		const int rc = mosquitto_subscribe(_mosquitto, nullptr, topicBuf, 0);
		if (rc != MOSQ_ERR_SUCCESS) {
			subscribed = false;
			Log::warn("Failed to subscribe to topic '%s' with error %s", topicBuf, mosquitto_strerror(rc));
		} else {
			Log::info("Subscribed to topic '%s'", topicBuf);
		}
	}
	_subscribed = subscribed;
	if (!_subscribed) {
		unsubscribe();
	} else {
		_instanceName = game.name;
	}

	return subscribed;
}

bool Protocol::send(const std::string& topic, const std::string& json) const {
	const int rc = mosquitto_publish(_mosquitto, nullptr, topic.c_str(), json.size(), json.c_str(), 0, true);
	if (rc != MOSQ_ERR_SUCCESS) {
		Log::warn("Failed to send to topic '%s' with error %s", topic.c_str(), mosquitto_strerror(rc));
	} else {
		Log::debug("Sent to topic '%s' with payload '%s'", topic.c_str(), json.c_str());
	}
	return rc == MOSQ_ERR_SUCCESS;
}

bool Protocol::join(const std::string& name) {
	if (core::string::contains(name, "#")
	 || core::string::contains(name, "/")
	 || core::string::contains(name, "+")) {
		Log::warn("Illegal client name");
		return false;
	}
	core::json j;
	j["name"] = name;
	j["mqttClientName"] = _clientToken;
	Log::info("Trying to join the game %s with client token %s and name %s",
			_instanceName.c_str(), _clientToken.c_str(), name.c_str());
	return send(core::string::format("traze/%s/join", _instanceName.c_str()), j.dump());
}

bool Protocol::steer(BikeDirection direction) const {
	if (_playerId == 0) {
		Log::info("Not joined");
		return false;
	}
	const char *course = "N";
	if (direction == BikeDirection::E) {
		course = "E";
	} else if (direction == BikeDirection::S) {
		course = "S";
	} else if (direction == BikeDirection::W) {
		course = "W";
	}
	core::json j;
	j["course"] = course;
	j["playerToken"] = _playerToken;
	return send(core::string::format("traze/%s/%i/steer", _instanceName.c_str(), _playerId), j.dump());
}

bool Protocol::bail() {
	if (_playerId == 0) {
		Log::info("Not joined");
		return false;
	}
	core::json j;
	j["playerToken"] = _playerToken;
	if (send(core::string::format("traze/%s/%i/bail", _instanceName.c_str(), _playerId), j.dump())) {
		_playerId = 0;
		_playerToken = "";
		return true;
	}
	return false;
}

void Protocol::parseOwnPlayer(const std::string& json) {
	const core::json j = core::json::parse(json);
	_playerToken = j["secretUserToken"];
	_playerId = j["id"].get<int>();
	const glm::ivec2& position = j["position"];
	Log::info("Player token %s with id %u at pos %i:%i", _playerToken.c_str(), _playerId, position.x, position.y);
	_eventBus->enqueue(std::make_shared<SpawnEvent>(Spawn{position, true}));
}

void Protocol::parsePlayers(const std::string& json) {
	const core::json j = core::json::parse(json);
	std::vector<Player> players;
	players.reserve(j.size());
	const voxel::MaterialColorArray& materialColors = voxel::getMaterialColors();
	for (const auto& player : j) {
		Player p;
		p.name = player["name"];
		const std::string& hex = player["color"];
		const glm::vec4& color = core::Color::fromHex(hex.c_str());
		const uint8_t index = core::Color::getClosestMatch(color, materialColors);
		p.colorIndex = index;
		p.color = materialColors[index];
		p.id = player["id"].get<int>();
		p.frags = player["frags"].get<int>();
		players.push_back(p);
		Log::debug("Player %s with id %i", p.name.c_str(), p.id);
	}
	_eventBus->enqueue(std::make_shared<PlayerListEvent>(players));
	std::unordered_map<uint32_t, Player> playerMap;
	for (const auto& p : players) {
		playerMap[p.id] = p;
	}
	_players = playerMap;
}

void Protocol::parseTicker(const std::string& json) const {
	const core::json j = core::json::parse(json);
	Ticker ticker;
	const std::string& type = j["type"];
	if (type == "suicide") {
		ticker.type = TickerType::Suicide;
	} else if (type == "frag") {
		ticker.type = TickerType::Frag;
	} else if (type == "collision") {
		ticker.type = TickerType::Collision;
	} else {
		ticker.type = TickerType::Unknown;
	}
	ticker.casualty = j["casualty"].get<int>();
	ticker.fragger = j["fragger"].get<int>();
	_eventBus->enqueue(std::make_shared<TickerEvent>(ticker));
}

void Protocol::parseGames(const std::string& json) const {
	const core::json j = core::json::parse(json);
	const size_t size = j.size();
	if (size == 0) {
		Log::debug("No active game found");
		return;
	}
	Log::debug("%i active games found", (int)size);
	std::vector<GameInfo> games;
	games.reserve(size);
	for (auto& it : j) {
		GameInfo g;
		g.activePlayers = it["activePlayers"];
		g.name = it["name"];
		games.push_back(g);
		Log::debug("%s with %i players", g.name.c_str(), g.activePlayers);
	}
	_eventBus->enqueue(std::make_shared<NewGamesEvent>(games));
}

void Protocol::fillGroundAndWall(voxel::RawVolume* v) {
	const voxel::Region& region = v->region();
	const voxel::Voxel voxel = voxel::createColorVoxel(voxel::VoxelType::Dirt, 0);
	// ground
	for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); ++z) {
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); ++x) {
			v->setVoxel(x, region.getLowerY(), z, voxel);
		}
	}
	// walls
	for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); ++z) {
		v->setVoxel(region.getLowerX(), region.getLowerY() + 1, z, voxel);
		v->setVoxel(region.getUpperX(), region.getLowerY() + 1, z, voxel);
		v->setVoxel(region.getLowerX(), region.getLowerY() + 2, z, voxel);
		v->setVoxel(region.getUpperX(), region.getLowerY() + 2, z, voxel);
	}
	for (int32_t x = region.getLowerX(); x <= region.getUpperX(); ++x) {
		v->setVoxel(x, region.getLowerY() + 1, region.getLowerZ(), voxel);
		v->setVoxel(x, region.getLowerY() + 1, region.getUpperZ(), voxel);
		v->setVoxel(x, region.getLowerY() + 2, region.getLowerZ(), voxel);
		v->setVoxel(x, region.getLowerY() + 2, region.getUpperZ(), voxel);
	}
}

void Protocol::parseGridAndUpdateVolume(const std::string& json) {
	const core::json j = core::json::parse(json);
	const int height = j["height"].get<int>();
	const int width = j["width"].get<int>();
	// x and z and swapped here
	const voxel::Region region(glm::ivec3(-1), glm::ivec3(height + 1, 1, width + 1));
	voxel::RawVolume* v = new voxel::RawVolume(region);
	fillGroundAndWall(v);
	const auto& grid = j["tiles"];
	int x = 0;
	for (const auto& line : grid) {
		if (x >= width) {
			Log::warn("Width overflow detected");
			break;
		}
		int z = 0;
		for (const auto& voxel : line) {
			if (z >= height) {
				Log::warn("Height overflow detected");
				break;
			}
			const int data = voxel.get<int>();
			if (data != 0) {
				const auto& iter = _players.find(data);
				if (iter == _players.end()) {
					Log::error("Can't find grid player id %i in player list", data);
					continue;
				}
				v->setVoxel(glm::ivec3(z, 1, x), voxel::createColorVoxel(voxel::VoxelType::Generic, iter->second.colorIndex));
			}
			++z;
		}
		++x;
	}
	if (j.find("bikes") != j.end()) {
		const auto& bikes = j["bikes"];
		for (const auto& bike : bikes) {
			Bike b;
			b.playerId = bike["playerId"].get<int>();
			int locX = bike["currentLocation"][0].get<int>();
			int locY = bike["currentLocation"][1].get<int>();
			b.currentLocation = glm::ivec2(locX, locY);
			const std::string direction = bike["direction"];
			if (direction == "W") {
				b.direction = BikeDirection::W;
			} else if (direction == "E") {
				b.direction = BikeDirection::E;
			} else if (direction == "N") {
				b.direction = BikeDirection::N;
			} else {
				b.direction = BikeDirection::S;
			}
			// TODO: "trail":[[2,0],[2,1]]
			_eventBus->enqueue(std::make_shared<BikeEvent>(b));
		}
	}
	if (j.find("spawns") != j.end()) {
		const auto& spawns = j["spawns"];
		for (const auto& spawn : spawns) {
			const glm::ivec2 spawnPos(spawn[0].get<int>(), spawn[1].get<int>());
			_eventBus->enqueue(std::make_shared<SpawnEvent>(Spawn{spawnPos, false}));
		}
	}
	_eventBus->enqueue(std::make_shared<NewGridEvent>(v));
}

void Protocol::onMessage(const struct mosquitto_message *msg) {
	Log::debug("MQTT: received message with topic: '%s'", msg->topic);
	if (!msg->payloadlen) {
		Log::debug("MQTT: empty message - no payload");
		return;
	}
	const char *payload = (const char*)msg->payload;
	const std::string p(payload, msg->payloadlen);
	Log::debug("MQTT: received message with payload: '%s'", p.c_str());
	const char *subTopic = core::string::after(msg->topic, '/');
	if (!strcmp(subTopic, "games")) {
		parseGames(p);
	} else if (!strcmp(subTopic, "grid")) {
		parseGridAndUpdateVolume(p);
	} else if (!strcmp(subTopic, "players")) {
		parsePlayers(p);
	} else if (!strcmp(subTopic, "ticker")) {
		parseTicker(p);
	} else if (!strcmp(subTopic, _clientToken.c_str())) {
		parseOwnPlayer(p);
	} else {
		Log::error("Unknown message for topic %s", msg->topic);
	}
}

void Protocol::onConnect(ConnectState status) {
	if (status != ConnectState::Success) {
		_connected = false;
		Log::error("Failed to connect to mqtt broker: %i", (int)status);
		return;
	}
	Log::info("Connected - subscribing now...");
	_connected = true;
	_subscribed = false;
	for (const char* topic : {"traze/games"}) {
		const int rc = mosquitto_subscribe(_mosquitto, nullptr, topic, 0);
		if (rc != MOSQ_ERR_SUCCESS) {
			Log::warn("Failed to subscribe to topic %s with error %s", topic, mosquitto_strerror(rc));
		} else {
			Log::debug("Subscribed to topic %s", topic);
		}
	}
}

}
