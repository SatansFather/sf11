#include "render_target.h"
#include "instance.h"
#include "sfassert.h"

namespace sf11
{

SfRenderTarget::SfRenderTarget(SfInstance* instance, const TextureParams2D& params)
	: SfTexture2D(instance, params, true, nullptr)
{
	bool staging = params.Usage.Value == SfUsage::Staging;

	// staging targets will not be bound to the gpu
	// instead they can be used to feed data from gpu resources to the cpu
	if (!staging)
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
		ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
		renderTargetDesc.Format = Data->Texture.TextureDesc2D.Format;
		renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetDesc.Texture2D.MipSlice = 0;
		sfAssertHR(Data->Instance->GetDevice()->CreateRenderTargetView(Data->Texture.Texture2D.Get(), &renderTargetDesc, &Data->Texture.RenderTargetView),
			"could not create render target view");
	}

	if (params.AllowUnorderedAccess)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = Data->Texture.TextureDesc2D.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = 0;
		sfAssertHR(Data->Instance->GetDevice()->CreateUnorderedAccessView(Data->Texture.Texture2D.Get(), &uavDesc, &Data->Texture.UnorderedAccessView),
			"could not create unordered access view for render target");
		Data->UnorderedAccess = Data->Texture.UnorderedAccessView.Get();
	}
}

}
