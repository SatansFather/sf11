#include "depth_buffer.h"
#include "instance.h"
#include "sfassert.h"

namespace sf11
{

SfDepthStencilState::SfDepthStencilState(class SfInstance* instance, const D3D11_DEPTH_STENCIL_DESC& desc):
	Data(std::make_shared<DepthStencilStateData>(instance, desc))
{
	Data->Instance->GetDevice()->CreateDepthStencilState(&Data->Desc, Data->State.GetAddressOf());
}

SfDepthBuffer::SfDepthBuffer(class SfInstance* instance, UINT width, UINT height, bool enableStencil /*= false*/)
{
	Data = std::make_shared<SfResource::ResourceData>(instance, false);
	Data->Texture.Width = width;
	Data->Texture.Height = height;

	Data->Texture.Params2D.Height = height;
	Data->Texture.Params2D.Width = width;

	D3D11_TEXTURE2D_DESC descdepth = {};
	descdepth.Width = width;
	descdepth.Height = height;
	descdepth.MipLevels = 1;
	descdepth.ArraySize = 1;
	descdepth.Format = enableStencil ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
	descdepth.SampleDesc.Count = 1;
	descdepth.SampleDesc.Quality = 0;
	descdepth.Usage = D3D11_USAGE_DEFAULT;
	descdepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	sfAssertHR(Data->Instance->GetDevice()->CreateTexture2D(&descdepth, nullptr, &Data->Texture.Texture2D),
		"could not create depth stencil texture");
	Data->Resource = Data->Texture.Texture2D.Get();

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
	dsvdesc.Flags = 0;
	dsvdesc.Format = enableStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
	dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvdesc.Texture2D.MipSlice = 0;
	sfAssertHR(Data->Instance->GetDevice()->CreateDepthStencilView(Data->Texture.Texture2D.Get(), &dsvdesc, &Data->Texture.DepthStencilView),
		"could not create depth stencil view");

	D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = enableStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MostDetailedMip = 0;
	srv.Texture2D.MipLevels = 1;
	sfAssertHR(Data->Instance->GetDevice()->CreateShaderResourceView(Data->Texture.Texture2D.Get(), &srv, &Data->Texture.ShaderResource),
		"could not create shader resource view from depth texture");
	Data->ShaderResource = Data->Texture.ShaderResource.Get();
}

}
