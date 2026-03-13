#include "cli/cli.hpp"

#include <chrono>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "argparse/argparse.hpp"
#include "core/device_engine/device_engine.hpp"
#include "core/event_engine/event.hpp"
#include "core/scheduler/event_scheduler.hpp"

using json = nlohmann::json;

namespace {

Event build_event(const std::string& type,
				  const std::string& device_id,
				  const std::string& payload,
				  int delay_ms) {
	Event event;
	event.type = type;
	event.device_id = device_id;
	event.payload = payload;
	event.scheduled_at = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
	return event;
}

void handle_show_devices() {
	const auto devices = DeviceEngine::instance().snapshot_devices();
	if (devices.empty()) {
		std::cout << "[cli] Aucun device charge.\n";
		return;
	}

	for (const auto& device : devices) {
		std::cout << "- id=" << device->id()
				  << " type=" << device->type()
				  << " label=" << std::quoted(device->label())
				  << " room=" << std::quoted(device->room())
				  << " model=" << device->modelId() << "\n";
	}
}

void handle_show_models() {
	const auto models = DeviceEngine::instance().snapshot_models();
	if (models.empty()) {
		std::cout << "[cli] Aucun model charge.\n";
		return;
	}

	for (const auto& model : models) {
		std::cout << "- id=" << model.id
				  << " type=" << model.type
				  << " protocol=" << model.protocol
				  << " label=" << std::quoted(model.label)
				  << " caps=";

		for (std::size_t i = 0; i < model.capabilities.size(); ++i) {
			if (i != 0) {
				std::cout << ',';
			}
			std::cout << model.capabilities[i];
		}

		std::cout << "\n";
	}
}

void handle_show_states(const std::string& device_id) {
	std::shared_ptr<VirtualDevice> device = DeviceEngine::instance().get_device_shared(device_id);
	if (!device) {
		std::cerr << "[cli] Device introuvable: " << device_id << "\n";
		return;
	}

	json body = json::object();
	for (const auto& [key, value] : device->states()) {
		body[key] = value;
	}

	if (body.empty()) {
		std::cout << "[cli] Aucun state expose pour " << device_id << ".\n";
		return;
	}

	std::cout << body.dump(2) << "\n";
}

} // namespace

void CLI::print_help() {
	std::cout
		<< "Commandes disponibles:\n"
		<< "  help\n"
		<< "  show-devices\n"
		<< "  show-models\n"
		<< "  show-states --id <device_id>\n"
		<< "  add-device --id <id> --label <label> --room <room> --model <model_id>\n"
		<< "  delete-device --id <device_id>\n"
		<< "  state --id <device_id> --payload <json> [--delay-ms <ms>]\n"
		<< "  event --type <event_type> [--id <device_id>] [--payload <json>] [--delay-ms <ms>]\n"
		<< "  trigger --payload <json> [--delay-ms <ms>]\n"
		<< "  exit\n";
}

bool CLI::tokenize_command_line(const std::string& line, std::vector<std::string>& tokens) {
	tokens.clear();
	std::string current;
	bool in_quotes = false;
	char quote_char = '\0';

	for (std::size_t index = 0; index < line.size(); ++index) {
		const char ch = line[index];

		if (in_quotes) {
			if (ch == '\\' && index + 1 < line.size() && line[index + 1] == quote_char) {
				current.push_back(line[index + 1]);
				++index;
				continue;
			}

			if (ch == quote_char) {
				in_quotes = false;
				quote_char = '\0';
				continue;
			}

			current.push_back(ch);
			continue;
		}

		if (ch == '"' || ch == '\'') {
			in_quotes = true;
			quote_char = ch;
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(ch))) {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}

		current.push_back(ch);
	}

	if (in_quotes) {
		std::cerr << "[cli] unterminated quoted string\n";
		return false;
	}

	if (!current.empty()) {
		tokens.push_back(current);
	}

	return true;
}

bool CLI::dispatch_cli_command(const std::vector<std::string>& raw_tokens) {
	std::vector<std::string> tokens;
	tokens.reserve(raw_tokens.size() + 1);
	tokens.push_back("smart-home-cli");
	tokens.insert(tokens.end(), raw_tokens.begin(), raw_tokens.end());

	argparse::ArgumentParser program("smart-home-cli");
	program.set_prefix_chars("-+/");
	program.set_assign_chars("=:");

	program.add_argument("command");
	program.add_argument("--id").default_value(std::string());
	program.add_argument("--label").default_value(std::string());
	program.add_argument("--room").default_value(std::string());
	program.add_argument("--model").default_value(std::string());
	program.add_argument("--type").default_value(std::string());
	program.add_argument("--payload").default_value(std::string());
	program.add_argument("--delay-ms").default_value(0).scan<'i', int>();

	try {
		program.parse_args(tokens);
	} catch (const std::exception& err) {
		std::cerr << "[cli] " << err.what() << "\n";
		std::cerr << program;
		return true;
	}

	const std::string command = program.get<std::string>("command");
	const std::string id = program.get<std::string>("--id");
	const std::string label = program.get<std::string>("--label");
	const std::string room = program.get<std::string>("--room");
	const std::string model = program.get<std::string>("--model");
	const std::string type = program.get<std::string>("--type");
	const std::string payload = program.get<std::string>("--payload");
	const int delay_ms = program.get<int>("--delay-ms");

	if (command == "help") {
		print_help();
		return true;
	}

	if (command == "show-devices") {
		handle_show_devices();
		return true;
	}

	if (command == "show-models") {
		handle_show_models();
		return true;
	}

	if (command == "show-states") {
		if (id.empty()) {
			std::cerr << "[cli] show-states requiert --id.\n";
			return true;
		}
		handle_show_states(id);
		return true;
	}

	if (command == "add-device") {
		if (id.empty() || label.empty() || room.empty() || model.empty()) {
			std::cerr << "[cli] add-device requiert --id, --label, --room et --model.\n";
			return true;
		}

		if (DeviceEngine::instance().add_device(id, label, room, model)) {
			std::cout << "[cli] Device ajoute: " << id << "\n";
		}
		return true;
	}

	if (command == "delete-device") {
		if (id.empty()) {
			std::cerr << "[cli] delete-device requiert --id.\n";
			return true;
		}

		if (DeviceEngine::instance().remove_device(id)) {
			std::cout << "[cli] Device supprime: " << id << "\n";
		} else {
			std::cerr << "[cli] Device introuvable: " << id << "\n";
		}
		return true;
	}

	if (command == "state") {
		if (id.empty() || payload.empty()) {
			std::cerr << "[cli] state requiert --id et --payload.\n";
			return true;
		}

		EventScheduler::instance().schedule_event(build_event("state_change", id, payload, delay_ms));
		std::cout << "[cli] State change planifie pour " << id << ".\n";
		return true;
	}

	if (command == "event") {
		if (type.empty()) {
			std::cerr << "[cli] event requiert --type.\n";
			return true;
		}

		EventScheduler::instance().schedule_event(build_event(type, id, payload, delay_ms));
		std::cout << "[cli] Event planifie: " << type << "\n";
		return true;
	}

	if (command == "trigger") {
		if (payload.empty()) {
			std::cerr << "[cli] trigger requiert --payload.\n";
			return true;
		}

		EventScheduler::instance().schedule_event(build_event("trigger", id, payload, delay_ms));
		std::cout << "[cli] Trigger planifie.\n";
		return true;
	}

	if (command == "exit" || command == "quit") {
		return false;
	}

	std::cerr << "[cli] Commande inconnue: " << command << "\n";
	print_help();
	return true;
}

void CLI::run_interactive_loop() {
	std::cout << "[main] CLI interactive prete. Tapez 'help' pour les commandes.\n";

	std::string line;
	while (true) {
		std::cout << "smart-home> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}

		if (line.empty()) {
			continue;
		}

		std::vector<std::string> tokens;
		if (!tokenize_command_line(line, tokens)) {
			continue;
		}

		if (tokens.empty()) {
			continue;
		}

		if (!dispatch_cli_command(tokens)) {
			break;
		}
	}
}
