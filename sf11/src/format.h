#pragma once

#include "d3d11_include.h"

namespace sf11
{

// Int_Norm values are mapped to floats from 0 - 1
// Int values are treated as they are
struct SfFormat
{
	enum
	{
		Float,
		HalfFloat,
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		SNorm8,
		SNorm16,
		UNorm8,
		UNorm8BGRA,
		UNorm16,
		Float11, // returns the (11, 11, 10) format, ignores channels
		Typeless8,
		Typeless16,
		Typeless32,
		Mat4x4,
		NullFormat
	};

	int Type = UNorm8;
	int Channels = 4;

	SfFormat() = default;
	SfFormat(int type) : Type(type), Channels(0) {}
	SfFormat(int type, int channels) : Type(type), Channels(channels) {}

	DXGI_FORMAT GetFormat() const;
	UINT GetTypeSize() const;
};

}