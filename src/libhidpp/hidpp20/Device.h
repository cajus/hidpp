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

#ifndef LIBHIDPP_HIDPP20_DEVICE_H
#define LIBHIDPP_HIDPP20_DEVICE_H

#include <hidpp/Device.h>

#include <cstdint>
#include <span>
#include <vector>

namespace HIDPP { class Dispatcher; }

namespace HIDPP20 {

class Device: public HIDPP::Device
{
public:
	static unsigned int softwareID;
	Device (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice);
	Device (HIDPP::Device &&other);

	/**
	 * Send a HID++ 2.0 feature call and return the response parameters.
	 *
	 * \param feature_index  Feature table index (from IRoot::getFeature).
	 * \param function       Function code within the feature.
	 * \param params         Parameter bytes (must fit in a single HID++ report).
	 * \returns Response parameter bytes.
	 */
	[[nodiscard]] std::vector<uint8_t> callFunction (uint8_t feature_index,
	                                                  unsigned int function,
	                                                  std::span<const uint8_t> params = {});
};

}

#endif
