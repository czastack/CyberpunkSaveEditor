#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

#if __has_include(<span>) && (!defined(_HAS_CXX20) or _HAS_CXX20)
#include <span>
#else
#include "span.hpp"
namespace std { using tcb::span; }
#endif

constexpr uint16_t byteswap(uint16_t value) noexcept
{
	return (value << 8) | (value >> 8);
}

constexpr uint32_t byteswap(uint32_t value) noexcept
{
	uint32_t tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
	return (tmp << 16) | (tmp >> 16);
}

class CRC32
{
	uint32_t value = 0xFFFFFFFF;

public:
	CRC32() = default;

	void feed(const void* data, size_t len)
	{
		constexpr std::array<uint32_t, 16> lut = {
			0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
			0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
		};

		auto pc = reinterpret_cast<const uint8_t*>(data);
		while (len--)
		{
			value = lut[(value ^ *pc) & 0x0F] ^ (value >> 4);
			value = lut[(value ^ (*pc >> 4)) & 0x0F] ^ (value >> 4);
			pc++;
		};
	}

	template <typename T, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>>>
	void feed_swaporder(T value)
	{
		value = byteswap(value);
		feed(&value, sizeof(T));
	}

	uint32_t get()
	{
		return ~value;
	}

	void reset(uint32_t crc = 0xFFFFFFFF)
	{
		value = crc;
	}
};

inline uint32_t crc32(std::string_view str)
{
	CRC32 crc;
	crc.feed(str.data(), str.size());
	return crc.get();
}


constexpr uint64_t FNV1a(std::string_view str)
{
	constexpr uint64_t basis = 0xCBF29CE484222325;
	constexpr uint64_t prime = 0x00000100000001B3;

	uint64_t hash = basis;
	for (auto c : str)
	{
		hash ^= c;
		hash *= prime;
	}

	return hash;
}


template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vector_streambuf : public std::basic_streambuf<CharT, TraitsT> {
public:
	vector_streambuf(std::vector<CharT> &vec) {
		this->setg(vec.data(), vec.data(), vec.data() + vec.size());
	}
};


void replace_all_in_str(std::string& s, const std::string& from, const std::string& to);

std::string u64_to_cpp(uint64_t val);

std::string bytes_to_hex(const void* buf, size_t len);

std::vector<uintptr_t> sse2_strstr_masked(const unsigned char* s, size_t m, const unsigned char* needle, size_t n, const char* mask, size_t maxcnt = 0);
std::vector<uintptr_t> sse2_strstr(const unsigned char* s, size_t m, const unsigned char* needle, size_t n, size_t maxcnt = 0);

