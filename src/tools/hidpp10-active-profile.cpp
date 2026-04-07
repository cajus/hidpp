/*
 * Copyright 2015 Clément Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <print>
#include <memory>

#include <hidpp/SimpleDispatcher.h>
#include <hidpp10/Device.h>
#include <hidpp10/IProfile.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path current|load|load-default|load-address|reload";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	char *endptr;
	if (argc-first_arg < 2) {
		std::print (stderr, "{}", getUsage (argv[0], args, &options));
		return EXIT_FAILURE;
	}

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (argv[first_arg]);
	}
	catch (std::exception &e) {
		std::println (stderr, "Failed to open device: {}.", e.what ());
		return EXIT_FAILURE;
	}

	HIDPP10::Device dev (dispatcher.get (), device_index);
	HIDPP10::IProfile iprofile (&dev);

	std::string command = argv[first_arg+1];
	if (command == "current") {
		int current = iprofile.activeProfile ();
		if (current == -1)
			std::println ("default");
		else
			std::println ("{}", current);
	}
	else if (command == "load") {
		if (argc-first_arg != 3) {
			std::print (stderr, "{}", getUsage (argv[0], args, &options));
			return EXIT_FAILURE;
		}
		int index = strtol (argv[first_arg+2], &endptr, 10);
		if (*endptr != '\0') {
			std::println (stderr, "Invalid profile index \"{}\"", argv[first_arg+2]);
			return EXIT_FAILURE;
		}
		iprofile.loadProfileFromIndex (index);
	}
	else if (command == "load-default") {
		iprofile.loadFactoryDefault ();
	}
	else if (command == "load-address") {
		if (argc-first_arg < 3 || argc-first_arg > 4) {
			std::print (stderr, "{}", getUsage (argv[0], args, &options));
			return EXIT_FAILURE;
		}
		int page = strtol (argv[first_arg+2], &endptr, 0);
		if (*endptr != '\0' || page < 0 || page > 255) {
			std::println (stderr, "Invalid page number \"{}\"", argv[first_arg+2]);
			return EXIT_FAILURE;
		}
		int offset;
		if (argc-first_arg == 4) {
			offset = strtol (argv[first_arg+3], &endptr, 0);
			if (*endptr != '\0' || page < 0 || page > 255) {
				std::println (stderr, "Invalid offset \"{}\"", argv[first_arg+2]);
				return EXIT_FAILURE;
			}
		}
		else
			offset = 0;
		iprofile.loadProfileFromAddress ({
			0,
			static_cast<uint8_t> (page),
			static_cast<uint8_t> (offset)
		});
	}
	else if (command == "reload") {
		iprofile.reloadActiveProfile ();
	}
	else {
		std::println (stderr, "Invalid command \"{}\".", argv[first_arg+1]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
