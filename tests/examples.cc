#include "json\value.hh"
#include "json\parser.hh"
#include "json\writer.hh"
#include <fstream>
#include <iostream>

using namespace std::literals;

void parse_file()
{
	unsigned short port;
	std::string ip;

	try
	{
		// Open the file to parse
		std::ifstream config_file("config.json");

		// Parse the file
		const json::value config_v = json::parser::parse(config_file);

		// Interpret the variant as an object
		const json::object & config = config_v.as_object();

		// Read the values of the json value
		port = static_cast<unsigned short>(config["Port"].as_uint());
		ip = config["IP"].as_string();
	}
	catch (const std::exception & ex)
	{
		std::cout << "Parsing the file \"config.json\" gave the following error: "s + ex.what() + "\nThe program will exit.\n";
		std::cin.get();
		exit(1);
	}
}

void write_json_file(unsigned short port, const std::string & ip)
{
	// Create a json::object
	json::object config;

	// Fill it with the values
	config["Port"] = port;
	config["IP"] = ip;

	// Open the file and write to it
	std::ofstream file{ "config.json" };
	file << config;
}

void using_static_string(unsigned short port, const std::string & ip)
{
	using namespace json::string_key_literals;

	// Create a json::object
	json::object config;

	// If the keys of the fields are string literals, it's better to pass them as static strings
	// because in that case the value will keep a reference to the original string instead of
	// making a local copy. This can save allocations and therefore improve performance.
	config["Port"_ss] = port;					// Using the static string literal operator
	config[json::static_string{"Port"}] = port; // Explicitly constructing a static_string object. Both are equivalent
	config["IP"_ss] = ip;

	// Open the file and write to it
	std::ofstream file{ "config.json" };
	file << config;
}
