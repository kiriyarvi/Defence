#include "params_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ParamsManager::ParamsManager() {
	std::ifstream params_file("params.json");

	std::stringstream input_stream;
	input_stream.set_rdbuf(params_file.rdbuf());
	std::stringstream output;

	std::string line;
	while (std::getline(input_stream, line)) {
		size_t comment_pos = line.find("//");
		if (comment_pos != std::string::npos) {
			// Игнорируем комментарий, сохраняем только часть до //
			line = line.substr(0, comment_pos);
		}
		output << line << '\n';
	}


	try {
		auto json = nlohmann::json::parse(output.str());
		from_json(json, params);
	}
	catch (const nlohmann::json::parse_error& e) {
		std::cerr << "Parse error:\n";
		std::cerr << e.what() << '\n';
	}
}