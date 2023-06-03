#include "blend_state.h"
#include "instance.h"

namespace sf11
{

SfBlendState::SfBlendState(SfInstance* instance, const SfBlendData& data)
	: Data(std::make_shared<BlendStateData>(instance))
{
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

	rtbd.BlendEnable = data.BlendEnable;
	rtbd.SrcBlend = (D3D11_BLEND)data.SrcBlend;
	rtbd.DestBlend = (D3D11_BLEND)data.DestBlend;
	rtbd.BlendOp = (D3D11_BLEND_OP)data.BlendOp;
	rtbd.SrcBlendAlpha = (D3D11_BLEND)data.SrcBlendAlpha;
	rtbd.DestBlendAlpha = (D3D11_BLEND)data.DestBlendAlpha;
	rtbd.BlendOpAlpha = (D3D11_BLEND_OP)data.BlendOpAlpha;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	bd.AlphaToCoverageEnable = false;

	for (int i = 0; i < 8; i++)
		bd.RenderTarget[i] = rtbd;

	Data->Instance->GetDevice()->CreateBlendState(&bd, &Data->State);
}

SfBlendState::SfBlendState(SfInstance* instance, SfBlendData* data, UINT count)
	: Data(std::make_shared<BlendStateData>(instance))
{
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

	for (UINT i = 0; i < count; i++)
	{
		rtbd.BlendEnable = data[i].BlendEnable;
		rtbd.SrcBlend = (D3D11_BLEND)data[i].SrcBlend;
		rtbd.DestBlend = (D3D11_BLEND)data[i].DestBlend;
		rtbd.BlendOp = (D3D11_BLEND_OP)data[i].BlendOp;
		rtbd.SrcBlendAlpha = (D3D11_BLEND)data[i].SrcBlendAlpha;
		rtbd.DestBlendAlpha = (D3D11_BLEND)data[i].DestBlendAlpha;
		rtbd.BlendOpAlpha = (D3D11_BLEND_OP)data[i].BlendOpAlpha;
		rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

		bd.AlphaToCoverageEnable = false;
		bd.RenderTarget[i] = rtbd;
	}

	Data->Instance->GetDevice()->CreateBlendState(&bd, &Data->State);
}

}