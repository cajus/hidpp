/*
 * Copyright 2015-2017 Clément Vuchener
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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp/Device.h>
#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>
#include <hidpp10/Sensor.h>
#include <hidpp10/IResolution.h>
#include <hidpp20/Device.h>
#include <hidpp20/IAdjustableDPI.h>
#include <print>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

template<typename InputIt>
void printDPIList (InputIt begin, InputIt end)
{
	bool first = true;
	for (auto it = begin; it != end; ++it) {
		if (first)
			first = false;
		else
			std::print (", ");
		std::print ("{}", *it);
	}
	std::println ("");
}

void printDPIRange (unsigned int min, unsigned int max, unsigned int step)
{
	std::println ("min: {}, max: {}, step: {}", min, max, step);
}

class Operations
{
public:
	const bool separated_axes;

	Operations (bool separated_axes):
		separated_axes (separated_axes)
	{
	}

	virtual ~Operations () = default;
	virtual void set (unsigned int dpi_x, unsigned int dpi_y) = 0;
	virtual void get (unsigned int &dpi_x, unsigned int &dpi_y) = 0;
	virtual void info () = 0;
};

class Operations10: public Operations
{
protected:
	HIDPP10::Device _dev;
	const HIDPP10::Sensor *_sensor;

	Operations10 (bool separated_axes, HIDPP10::Device &&dev, const HIDPP10::Sensor *sensor):
		Operations (separated_axes),
		_dev (std::move (dev)),
		_sensor (sensor)
	{
	}

public:
	void info ()
	{
		const HIDPP10::ListSensor *list;
		const HIDPP10::RangeSensor *range;
		if ((list = dynamic_cast<const HIDPP10::ListSensor *> (_sensor)) != nullptr)
			printDPIList (list->begin (), list->end ());
		else if ((range = dynamic_cast<const HIDPP10::RangeSensor *> (_sensor)) != nullptr)
			printDPIRange (range->minimumResolution (),
				       range->maximumResolution (),
				       range->resolutionStepHint ());
		else
			std::println ("unsupported sensor");
	}

};

class Operations10Type0: public Operations10
{
	HIDPP10::IResolution0 _iresolution;

public:
	Operations10Type0 (HIDPP10::Device &&dev, const HIDPP10::Sensor *sensor):
		Operations10 (false, std::move (dev), sensor),
		_iresolution (&_dev, sensor)
	{
	}

	void set (unsigned int dpi_x, unsigned int dpi_y)
	{
		_iresolution.setCurrentResolution (dpi_x);
	}

	void get (unsigned int &dpi_x, unsigned int &dpi_y)
	{
		dpi_x = _iresolution.getCurrentResolution ();
	}

};

class Operations10Type3: public Operations10
{
	HIDPP10::IResolution3 _iresolution;

public:
	Operations10Type3 (HIDPP10::Device &&dev, const HIDPP10::Sensor *sensor):
		Operations10 (true, std::move (dev), sensor),
		_iresolution (&_dev, sensor)
	{
	}

	void set (unsigned int dpi_x, unsigned int dpi_y)
	{
		_iresolution.setCurrentResolution (dpi_x, dpi_y);
	}

	void get (unsigned int &dpi_x, unsigned int &dpi_y)
	{
		_iresolution.getCurrentResolution (dpi_x, dpi_y);
	}

};

class Operations20: public Operations
{
	HIDPP20::Device _dev;
	HIDPP20::IAdjustableDPI _iadjustabledpi;
	unsigned int _sensor_index;

public:
	Operations20 (HIDPP20::Device &&dev, unsigned int sensor_index):
		Operations (false),
		_dev (std::move (dev)),
		_iadjustabledpi (&_dev),
		_sensor_index (sensor_index)
	{
	}

	void set (unsigned int dpi_x, unsigned int dpi_y)
	{
		_iadjustabledpi.setSensorDPI (_sensor_index, dpi_x);
	}

	void get (unsigned int &dpi_x, unsigned int &dpi_y)
	{
		std::tie (dpi_x, std::ignore) = _iadjustabledpi.getSensorDPI (_sensor_index);
	}

	void info ()
	{
		unsigned int sensor_count = _iadjustabledpi.getSensorCount ();
		std::println ("Sensor count: {}", sensor_count);
		for (unsigned int i = 0; i < sensor_count; ++i) {
			std::print ("{}: ", i);
			std::vector<unsigned int> list;
			unsigned int step;
			if (_iadjustabledpi.getSensorDPIList (i, list, step))
				printDPIRange (list[0], list[1], step);
			else
				printDPIList (list.begin (), list.end ());
		}
	}
};

int main (int argc, char *argv[])
{
	static const char *args = "device_path get|set dpi [y_dpi]";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;
	int sensor = 0;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
		Option ('s', "sensor",
			Option::RequiredArgument, "sensor_index",
			"use the sensor sensor_index",
			[&sensor] (const char *optarg) -> bool {
				char *endptr;
				sensor = strtol (optarg, &endptr, 10);
				if (*endptr != '\0') {
					std::println (stderr, "Invalid sensor index: {}", optarg);
					return false;
				}
				return true;
			}),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg < 2) {
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

	HIDPP::Device dev (dispatcher.get (), device_index);
	unsigned int major, minor;
	std::tie (major, minor) = dev.protocolVersion ();

	std::unique_ptr<Operations> ops;
	if (major == 1) {
		const HIDPP10::MouseInfo *info = HIDPP10::getMouseInfo (dev.productID ());
		if (!info) {
			std::println (stderr, "Unsupported mice.");
			return EXIT_FAILURE;
		}

		switch (info->iresolution_type) {
		case HIDPP10::IResolutionType0:
			ops.reset (new Operations10Type0 (std::move (dev), info->sensor));
			break;
		case HIDPP10::IResolutionType3:
			ops.reset (new Operations10Type3 (std::move (dev), info->sensor));
			break;
		default:
			std::println (stderr, "Unsupported resolution type.");
			return EXIT_FAILURE;
		}
	}
	else {
		ops.reset (new Operations20 (HIDPP20::Device (std::move (dev)), sensor));
	}

	std::string op = argv[first_arg+1];
	first_arg += 2;
	if (op == "get") {
		unsigned int dpi_x, dpi_y;
		ops->get (dpi_x, dpi_y);
		if (ops->separated_axes)
			std::println ("X: {} dpi, Y: {} dpi", dpi_x, dpi_y);
		else
			std::println ("{} dpi", dpi_x);
	}
	else if (op == "set") {
		char *endptr;
		unsigned int dpi_x, dpi_y;
		if (argc - first_arg < 1) {
			std::println (stderr, "missing dpi_x");
			return EXIT_FAILURE;
		}
		dpi_x = strtol (argv[first_arg], &endptr, 10);
		if (*endptr != '\0') {
			std::println (stderr, "Invalid dpi_x value.");
			return EXIT_FAILURE;
		}
		if (ops->separated_axes) {
			if (argc - first_arg < 2) {
				dpi_y = dpi_x;
			}
			else {
				dpi_y = strtol (argv[first_arg+1], &endptr, 10);
				if (*endptr != '\0') {
					std::println (stderr, "Invalid dpi_y value.");
					return EXIT_FAILURE;
				}
			}
		}
		ops->set (dpi_x, dpi_y);
	}
	else if (op == "info") {
		ops->info ();
	}
	else {
		std::println (stderr, "Invalid operation: {}", op);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

