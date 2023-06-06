#include "texture.h"
#include "instance.h"
#include "sfassert.h"
#include "context.h"

namespace sf11
{

template <int I, typename PARAMS, typename TEXTURE>
void CreateViews(
	SfInstance* instance, 
	const PARAMS& params, 
	ComPtr<ID3D11ShaderResourceView>& srv, 
	ComPtr<ID3D11UnorderedAccessView>& uav,
	TEXTURE& texture)
{
	if (params.AllowShaderResource && params.Usage.Value != SfUsage::Staging)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
		ZeroMemory(&shaderResourceDesc, sizeof(shaderResourceDesc));
		shaderResourceDesc.Format = params.TextureFormat.GetFormat();
		if constexpr (I == 1)
		{
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			shaderResourceDesc.Texture1D.MostDetailedMip = 0;
			shaderResourceDesc.Texture1D.MipLevels = 1;
		}
		else if constexpr (I == 2)
		{
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceDesc.Texture2D.MipLevels = 1;
		}
		else if constexpr (I == 3)
		{
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			shaderResourceDesc.Texture3D.MostDetailedMip = 0;
			shaderResourceDesc.Texture3D.MipLevels = 1;
		}
		sfAssertHR(instance->GetDevice()->CreateShaderResourceView(texture.Get(), &shaderResourceDesc, &srv),
			"could not create shader resource view");
	}

	if (params.AllowUnorderedAccess)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = params.TextureFormat.GetFormat();
		if constexpr (I == 1)
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture1D.MipSlice = 0;
		}
		if constexpr (I == 2)
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
		}
		if constexpr (I == 3)
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.MipSlice = 0;
		}
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = 0;
		sfAssertHR(instance->GetDevice()->CreateUnorderedAccessView(texture.Get(), &uavDesc, &uav),
			"could not create unordered access view for texture");
	}
}

template <int I, typename TEXDESC, typename PARAMS>
void CreateTexDesc(TEXDESC& desc, const PARAMS& params, bool renderTarget = false)
{
	bool staging = params.Usage.Value == SfUsage::Staging;
	ZeroMemory(&desc, sizeof(desc));

	desc.MipLevels = params.MipLevels;
	desc.Format = params.TextureFormat.GetFormat();
	desc.Usage = params.Usage.GetUsage();
	desc.CPUAccessFlags = 
		desc.Usage == D3D11_USAGE_DYNAMIC ? 
			D3D11_CPU_ACCESS_WRITE : 
		desc.Usage == D3D11_USAGE_STAGING ? 
			D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE : 
		0;

	desc.BindFlags = staging ? 0 
		: ((renderTarget ? D3D11_BIND_RENDER_TARGET : 0)
			| (params.AllowShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0) 
			| (params.AllowUnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0));

	if constexpr (I < 3)
		desc.ArraySize = 1;

	if constexpr (I == 2)
	{
		desc.SampleDesc.Count = params.SampleCount;
		desc.SampleDesc.Quality = params.SampleQuality;
	}

	desc.Width = params.Width;
	if constexpr (I > 1)
		desc.Height = params.Height;
	if constexpr (I > 2)
		desc.Depth = params.Depth;
}

SfTexture::SfTexture(SfInstance* instance)
	: SfResource(instance)
{

}

SfTexture1D::SfTexture1D(SfInstance* instance, const TextureParams1D& params, void* data)
	: SfTexture(instance)
{
	Data->Texture.Width = params.Width;
	Data->Texture.Dimensions = 1;
	CreateTexDesc<1>(Data->Texture.TextureDesc1D, params);

	Data->Texture.Params1D = params;
	Data->Usage = params.Usage;

	UINT size = params.TextureFormat.GetTypeSize();
	D3D11_SUBRESOURCE_DATA d = {};
	if (data) d.pSysMem = data;

	sfAssertHR(Data->Instance->GetDevice()->CreateTexture1D(&Data->Texture.TextureDesc1D, data ? &d : NULL, &Data->Texture.Texture1D),
		"could not create texture1D");
	Data->Resource = Data->Texture.Texture1D.Get();

	CreateViews<1>(Data->Instance, params, Data->Texture.ShaderResource, Data->Texture.UnorderedAccessView, Data->Texture.Texture1D);
	Data->ShaderResource = Data->Texture.ShaderResource.Get();
	Data->UnorderedAccess = Data->Texture.UnorderedAccessView.Get();
}

SfTexture2D::SfTexture2D(SfInstance* instance, const TextureParams2D& params, bool renderTarget, std::unique_ptr<SfSurface2D> surface)
	: SfTexture(instance)
{
	Data->Texture.Width = params.Width;
	Data->Texture.Height = params.Height;
	Data->Texture.Dimensions = 2;
	CreateTexDesc<2>(Data->Texture.TextureDesc2D, params, renderTarget);

	Data->Texture.Params2D = params;
	Data->Usage = params.Usage;

	UINT size = params.TextureFormat.GetTypeSize();
	D3D11_SUBRESOURCE_DATA d = {};
	if (surface.get())
	{
		d.pSysMem = surface->GetData();
		d.SysMemPitch = surface->GetPaddedWidth() * 4;
	}

	sfAssertHR(Data->Instance->GetDevice()->CreateTexture2D(&Data->Texture.TextureDesc2D, d.pSysMem ? &d : NULL, &Data->Texture.Texture2D),
		"could not create texture2D");
	Data->Resource = Data->Texture.Texture2D.Get();

	CreateViews<2>(Data->Instance, params, Data->Texture.ShaderResource, Data->Texture.UnorderedAccessView, Data->Texture.Texture2D);
	Data->ShaderResource = Data->Texture.ShaderResource.Get();
	Data->UnorderedAccess = Data->Texture.UnorderedAccessView.Get();
}

SfTexture2D::SfTexture2D(SfInstance* instance, const TextureParams2D& params, void* data /*= nullptr*/)
	: SfTexture(instance)
{
	Data->Texture.Width = params.Width;
	Data->Texture.Height = params.Height;
	Data->Texture.Dimensions = 2;
	CreateTexDesc<2>(Data->Texture.TextureDesc2D, params);

	Data->Texture.Params2D = params;
	Data->Usage = params.Usage;

	D3D11_SUBRESOURCE_DATA d = {};
	if (data)
	{
		d.pSysMem = data;
		d.SysMemPitch = params.Width * params.TextureFormat.GetTypeSize() * params.TextureFormat.Channels;
	}

	sfAssertHR(Data->Instance->GetDevice()->CreateTexture2D(&Data->Texture.TextureDesc2D, d.pSysMem ? &d : NULL, &Data->Texture.Texture2D),
		"could not create texture2D");
	Data->Resource = Data->Texture.Texture2D.Get();

	CreateViews<2>(Data->Instance, params, Data->Texture.ShaderResource, Data->Texture.UnorderedAccessView, Data->Texture.Texture2D);
	Data->ShaderResource = Data->Texture.ShaderResource.Get();
	Data->UnorderedAccess = Data->Texture.UnorderedAccessView.Get();
}

SfTexture3D::SfTexture3D(SfInstance* instance, const TextureParams3D& params, void* data)
	: SfTexture(instance)
{
	Data->Texture.Width = params.Width;
	Data->Texture.Height = params.Height;
	Data->Texture.Depth = params.Depth;
	Data->Texture.Dimensions = 3;
	CreateTexDesc<3>(Data->Texture.TextureDesc3D, params);

	Data->Texture.Params3D = params;
	Data->Usage = params.Usage;

	UINT size = params.TextureFormat.GetTypeSize();
	D3D11_SUBRESOURCE_DATA d = {};
	if (data)
	{
		d.pSysMem = data;
		d.SysMemPitch = params.Width * size;
		d.SysMemSlicePitch = params.Height * params.Width * size;
	}

	sfAssertHR(Data->Instance->GetDevice()->CreateTexture3D(&Data->Texture.TextureDesc3D, data ? &d : NULL, &Data->Texture.Texture3D),
		"could not create texture3D");
	Data->Resource = Data->Texture.Texture3D.Get();

	CreateViews<3>(Data->Instance, params, Data->Texture.ShaderResource, Data->Texture.UnorderedAccessView, Data->Texture.Texture3D);
	Data->ShaderResource = Data->Texture.ShaderResource.Get();
	Data->UnorderedAccess = Data->Texture.UnorderedAccessView.Get();
}

}
