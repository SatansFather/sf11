#include "format.h"
#include <map>
#include <vector>
#include "sfassert.h"


namespace sf11
{

std::map<int, std::vector<DXGI_FORMAT>> Formats =
{
	{
		SfFormat::Float,
		{
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
		},
	},
	{
		SfFormat::HalfFloat,
		{
			DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_R16G16_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
		},
	},
	{
		SfFormat::Int8,
		{
			DXGI_FORMAT_R8_SINT,
			DXGI_FORMAT_R8G8_SINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_SINT,
		},
	},
	{
		SfFormat::UInt8,
		{
			DXGI_FORMAT_R8_UINT,
			DXGI_FORMAT_R8G8_UINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_UINT,
		},
	},
	{
		SfFormat::Int16,
		{
			DXGI_FORMAT_R16_SINT,
			DXGI_FORMAT_R16G16_SINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_SINT,
		},
	},
	{
		SfFormat::UInt16,
		{
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_R16G16_UINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_UINT,
		},
	},
	{
		SfFormat::Int32,
		{
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32G32_SINT,
			DXGI_FORMAT_R32G32B32_SINT,
			DXGI_FORMAT_R32G32B32A32_SINT,
		},
	},
	{
		SfFormat::UInt32,
		{
			DXGI_FORMAT_R32_UINT,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R32G32B32_UINT,
			DXGI_FORMAT_R32G32B32A32_UINT,
		},
	},
	{
		SfFormat::SNorm8,
		{
			DXGI_FORMAT_R8_SNORM,
			DXGI_FORMAT_R8G8_SNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_SNORM,
		},
	},
	{
		SfFormat::SNorm16,
		{
			DXGI_FORMAT_R16_SNORM,
			DXGI_FORMAT_R16G16_SNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_SNORM,
		},
	},
	{
		SfFormat::UNorm8,
		{
			DXGI_FORMAT_R8_UNORM,
			DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_UNORM,
		},
	},
	{
		SfFormat::UNorm8BGRA,
		{
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_B8G8R8A8_UNORM,
		},
	},
	{
		SfFormat::UNorm16,
		{
			DXGI_FORMAT_R16_UNORM,
			DXGI_FORMAT_R16G16_UNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_UNORM,
		},
	},
	{
		SfFormat::Typeless8,
		{
			DXGI_FORMAT_R8_TYPELESS,
			DXGI_FORMAT_R8G8_TYPELESS,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_TYPELESS,
		}
	},
	{
		SfFormat::Typeless16,
		{
			DXGI_FORMAT_R16_TYPELESS,
			DXGI_FORMAT_R16G16_TYPELESS,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_TYPELESS,
		},
	},
	{
		SfFormat::Typeless32,
		{
			DXGI_FORMAT_R32_TYPELESS,
			DXGI_FORMAT_R32G32_TYPELESS,
			DXGI_FORMAT_R32G32B32_TYPELESS,
			DXGI_FORMAT_R32G32B32A32_TYPELESS,
		},
	},
};

static std::map<int, UINT> Sizes = 
{
	{ SfFormat::Float,      4 },
	{ SfFormat::HalfFloat,  2 },
	{ SfFormat::Int8,       1 },
	{ SfFormat::UInt8,      1 },
	{ SfFormat::Int16,      2 },
	{ SfFormat::UInt16,     2 },
	{ SfFormat::Int32,      4 },
	{ SfFormat::UInt32,     4 },
	{ SfFormat::SNorm8,     1 },
	{ SfFormat::SNorm16,    2 },
	{ SfFormat::UNorm8,     1 },
	{ SfFormat::UNorm8BGRA, 4 },
	{ SfFormat::UNorm16,    2 },
	{ SfFormat::Float11,    4 },
	{ SfFormat::Typeless8,  1 },
	{ SfFormat::Typeless16, 2 },
	{ SfFormat::Typeless32, 4 },
	{ SfFormat::Mat4x4,    64 },
};

DXGI_FORMAT SfFormat::GetFormat() const
{
	if (Type == Float11) return DXGI_FORMAT_R11G11B10_FLOAT;

	sfAssert(Type < Mat4x4, "cannot directly get a format from the passed type");
	sfAssert(Formats.contains(Type), "cannot get a texture format of an invalid type");
	sfAssert(Channels <= 4, "texture formats must be between 1 and 4 channels");
	DXGI_FORMAT format = Formats[Type][Channels - 1];
	sfAssert(format != DXGI_FORMAT_UNKNOWN, "invalid texture format");

	return format;
}

UINT SfFormat::GetTypeSize() const
{
	sfAssert(Sizes.contains(Type), "cannot find size for type");
	return Sizes[Type];
}

}
