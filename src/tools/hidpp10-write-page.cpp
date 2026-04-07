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

#include <cstdio>
#include <memory>
#include <print>

#include <hidpp/SimpleDispatcher.h>
#include <hidpp10/Device.h>
#include <hidpp10/IMemory.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path page";
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

	if (argc-first_arg != 2) {
		std::print (stderr, "{}", getUsage (argv[0], args, &options));
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	unsigned int page = atoi (argv[first_arg+1]);

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		std::println (stderr, "Failed to open device: {}.", e.what ());
		return EXIT_FAILURE;
	}
	HIDPP10::Device dev (dispatcher.get (), device_index);
	static constexpr std::size_t PageSize = 512;
	std::vector<uint8_t> data (PageSize);
	std::size_t bytes_read = fread (data.data (), sizeof(uint8_t), PageSize, stdin);
	if (ferror (stdin)) {
		std::println (stderr, "Failed to read data.");
		return EXIT_FAILURE;
	}
	if (bytes_read != PageSize) {
		std::println (stderr, "Short read: got {} bytes, expected {}.", bytes_read, PageSize);
		return EXIT_FAILURE;
	}

	HIDPP10::IMemory (&dev).writePage (page, data);

	return EXIT_SUCCESS;
}
