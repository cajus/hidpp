/*
 * Copyright 2016 Clément Vuchener
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

#ifndef LIBHIDPP_ENDIAN_H
#define LIBHIDPP_ENDIAN_H

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>

// ── little-endian write ───────────────────────────────────────────────────────

template<std::integral T, typename OutputIt>
OutputIt writeLE (OutputIt it, T value)
{
	if constexpr (std::endian::native != std::endian::little)
		value = std::byteswap (value);
	auto bytes = std::bit_cast<std::array<uint8_t, sizeof (T)>> (value);
	for (auto b : bytes)
		*(it++) = b;
	return it;
}

template<std::integral T, typename Container>
void writeLE (Container &cont, unsigned int index, T value)
{
	writeLE<T> (cont.begin () + index, value);
}

// ── little-endian read ────────────────────────────────────────────────────────

template<std::integral T, typename InputIt>
T readLE (InputIt it)
{
	std::array<uint8_t, sizeof (T)> bytes;
	for (auto &b : bytes)
		b = static_cast<uint8_t> (*(it++));
	auto value = std::bit_cast<T> (bytes);
	if constexpr (std::endian::native != std::endian::little)
		value = std::byteswap (value);
	return value;
}

template<std::integral T, typename Container>
T readLE (const Container &cont, unsigned int index)
{
	return readLE<T> (cont.begin () + index);
}

// ── little-endian push ────────────────────────────────────────────────────────

template<std::integral T, typename Container>
void pushLE (Container &cont, T value)
{
	if constexpr (std::endian::native != std::endian::little)
		value = std::byteswap (value);
	auto bytes = std::bit_cast<std::array<uint8_t, sizeof (T)>> (value);
	for (auto b : bytes)
		cont.push_back (b);
}

// ── big-endian write ──────────────────────────────────────────────────────────

template<std::integral T, typename OutputIt>
OutputIt writeBE (OutputIt it, T value)
{
	if constexpr (std::endian::native != std::endian::big)
		value = std::byteswap (value);
	auto bytes = std::bit_cast<std::array<uint8_t, sizeof (T)>> (value);
	for (auto b : bytes)
		*(it++) = b;
	return it;
}

template<std::integral T, typename Container>
void writeBE (Container &cont, unsigned int index, T value)
{
	writeBE<T> (cont.begin () + index, value);
}

// ── big-endian read ───────────────────────────────────────────────────────────

template<std::integral T, typename InputIt>
T readBE (InputIt it)
{
	std::array<uint8_t, sizeof (T)> bytes;
	for (auto &b : bytes)
		b = static_cast<uint8_t> (*(it++));
	auto value = std::bit_cast<T> (bytes);
	if constexpr (std::endian::native != std::endian::big)
		value = std::byteswap (value);
	return value;
}

template<std::integral T, typename Container>
T readBE (const Container &cont, unsigned int index)
{
	return readBE<T> (cont.begin () + index);
}

// ── big-endian push ───────────────────────────────────────────────────────────

template<std::integral T, typename Container>
void pushBE (Container &cont, T value)
{
	if constexpr (std::endian::native != std::endian::big)
		value = std::byteswap (value);
	auto bytes = std::bit_cast<std::array<uint8_t, sizeof (T)>> (value);
	for (auto b : bytes)
		cont.push_back (b);
}

#endif
