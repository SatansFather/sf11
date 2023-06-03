#include "usage.h"
#include "sfassert.h"

namespace sf11
{

D3D11_USAGE SfUsage::GetUsage() const
{
	sfAssert(Value <= Immutable, "cannot get usage from invalid value");
	
	switch (Value)
	{
		case Static:    return D3D11_USAGE_DEFAULT;
		case Dynamic:   return D3D11_USAGE_DYNAMIC;
		case Staging:   return D3D11_USAGE_STAGING;
		case Immutable: return D3D11_USAGE_IMMUTABLE;
	}

	return D3D11_USAGE_DEFAULT;
}

}
