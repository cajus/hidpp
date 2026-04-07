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

#include <memory>
#include <print>

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IOnboardProfiles.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path";
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

	if (argc-first_arg < 1) {
		std::println (stderr, "Too few arguments.");
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

	HIDPP20::Device dev (dispatcher.get (), device_index);
	try {
		HIDPP20::IOnboardProfiles iop (&dev);
		auto desc = iop.getDescription ();
		std::println ("Memory model:\t{}", static_cast<uint8_t> (desc.memory_model));
		std::println ("Profile format:\t{}", static_cast<uint8_t> (desc.profile_format));
		std::println ("Macro format:\t{}", static_cast<uint8_t> (desc.macro_format));
		std::println ("Profile count:\t{}", desc.profile_count);
		std::println ("Profile count OOB:\t{}", desc.profile_count_oob);
		std::println ("Button count:\t{}", desc.button_count);
		std::println ("Sector count:\t{}", desc.sector_count);
		std::println ("Sector size:\t{}", desc.sector_size);
		std::print ("Mechanical layout:\t0x{:x} (", desc.mechanical_layout);
		bool first = true;
		if ((desc.mechanical_layout & 0x03) == 2) {
			if (first)
				first = false;
			else
				std::print (", ");
			std::print ("G-shift");
		}
		if ((desc.mechanical_layout & 0x0c) >> 2 == 2) {
			if (first)
				first = false;
			else
				std::print (", ");
			std::print ("DPI shift");
		}
		std::println (")");
		std::print ("Various info:\t0x{:x} (", desc.various_info);
		switch (desc.various_info & 0x07) {
		case 1:
			std::print ("Corded");
			break;
		case 2:
			std::print ("Wireless");
			break;
		case 4:
			std::print ("Corded + Wireless");
			break;
		}
		std::println (")");
	}
	catch (HIDPP20::Error &e) {
		std::println (stderr, "Error code {}: {}", e.errorCode (), e.what ());
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}

