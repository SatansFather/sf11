#include "instance.h"
#include "adapter.h"
#include "sfassert.h"
#include <d3dcompiler.h>
#include "comdef.h"
#include "shader_program.h"
#include "depth_buffer.h"
#include "context.h"
#include "surface.h"
#include "gdi.h"

namespace sf11
{

SfInstance::SfInstance(const InstanceCreationParams& params /*= InstanceCreationParams()*/)
	: CreationParams(params)
{
	static bool rawMouseRegistered = false;
	if (!rawMouseRegistered)
	{
		RAWINPUTDEVICE device[1];
		device[0].usUsagePage = 1; 
		device[0].usUsage = 2; 
		device[0].dwFlags = 0;
		device[0].hwndTarget = NULL; 
		if (RegisterRawInputDevices(device, 1, sizeof(RAWINPUTDEVICE)))
		{
			rawMouseRegistered = true;
		}
	}

	InitGDI();
	CreateDevice();
	Window = CreateNewWindow(params.Window);
	CreateRasterizerStates();
	
	InitDepthStates();
	ImmediateContext->SetDepthBufferState(EDepthState::ReadWrite);
	ImmediateContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ImmediateContext->SetViewport((float)params.Window.Width, (float)params.Window.Height);
	ImmediateContext->SetCullAndFillMode(ECullMode::CullBack, EFillMode::Solid);
}

SfInstance::~SfInstance()
{
	
}

SfRenderTarget SfInstance::CreateRenderTarget(const TextureParams2D& params)
{
	return SfRenderTarget(this, params);	
}

template <typename T>
T SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage)
{
	T shader;
	shader.Data = std::make_shared<SfShader::ShaderData>();
	shader.Data->Instance = this;
	shader.Data->Stage = stage;
	shader.CompileFromFile(stage, filePath);
	return shader;
}

SfShaderProgram SfInstance::CreateShaderProgram(const ShaderProgramCreateParams& params)
{
	SfShaderProgram program;
	program.Data = std::make_shared<SfShaderProgram::ShaderProgramData>();
	program.Data->Instance = this;
	program.InitFromParams(params);
	return program;
}

SfContext_Deferred SfInstance::CreateDeferredContext()
{
	SfContext_Deferred context;
	context.Data = std::make_shared<SfContext::ContextData>();
	context.Data->Instance = this;
	Device->CreateDeferredContext(0, &context.Data->Context);
	return context;
}

SfSamplerState SfInstance::CreateSampler_PointWrap()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler_PointClamp()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler_PointMirror()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler_BilinearWrap()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler_BilinearClamp()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler_BilinearMirror()
{
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(sd);
}

SfSamplerState SfInstance::CreateSampler(const D3D11_SAMPLER_DESC& desc)
{
	SfSamplerState sampler;
	sampler.Data = std::make_shared<SfSamplerState::SamplerStateData>();
	sampler.Data->Instance = this;
	sampler.Data->Desc = desc;
	Device->CreateSamplerState(&desc, sampler.Data->State.GetAddressOf());
	return sampler;
}

SfDepthStencilState SfInstance::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc)
{
	return SfDepthStencilState(this, desc);
}

SfDepthBuffer SfInstance::CreateDepthBuffer(const TextureParams2D& params, bool enableStencil /*= false*/)
{
	return SfDepthBuffer(this, params.Width, params.Height, enableStencil);
}

SfBlendState SfInstance::CreateBlendState(const SfBlendData& data)
{
	return SfBlendState(this, data);
}

SfBlendState SfInstance::CreateBlendState(SfBlendData* data, UINT count)
{
	return SfBlendState(this, data, count);
}

SfTexture2D SfInstance::CreateTexture2DFromSurface(std::unique_ptr<SfSurface2D> surface)
{
	TextureParams2D p;
	return CreateTexture2DFromSurface(std::move(surface), p);
}

SfTexture2D SfInstance::CreateTexture2DFromSurface(std::unique_ptr<SfSurface2D> surface, TextureParams2D& params)
{
	params.Width = surface->GetPaddedWidth();
	params.Height = surface->GetPaddedHeight();
	params.TextureFormat = { SfFormat::UNorm8BGRA, 4 };
	return SfTexture2D(this, params, false, std::move(surface));
}

SfTexture2D SfInstance::LoadTexture2D(const std::string& path)
{
	TextureParams2D p;
	return LoadTexture2D(path, p);
}

SfTexture2D SfInstance::LoadTexture2D(const std::string& path, TextureParams2D& params)
{
	auto surface = std::make_unique<SfSurface2D>();
	surface->LoadPNG(path);	
	return CreateTexture2DFromSurface(std::move(surface), params);
}

SfTexture1D SfInstance::CreateTexture1D(const TextureParams1D& params, void* data /*= nullptr*/)
{
	return SfTexture1D(this, params, data);
}

SfTexture2D SfInstance::CreateTexture2D(const TextureParams2D& params, void* data /*= nullptr*/)
{
	return SfTexture2D(this, params, data);
}

SfTexture3D SfInstance::CreateTexture3D(const TextureParams3D& params, void* data /*= nullptr*/)
{
	return SfTexture3D(this, params, data);
}

SfBuffer_Vertex SfInstance::CreateVertexBuffer(UINT vertexSize, UINT vertexCount, SfUsage usage/* = SfUsage::Static*/, void* vertices /*= nullptr*/)
{
	return SfBuffer_Vertex(this, vertexSize, vertexCount, usage, vertices);
}

SfBuffer_Index SfInstance::CreateIndexBuffer(UINT indexSize, UINT indexCount, SfUsage usage/* = SfUsage::Static*/, void* indices /*= nullptr*/)
{
	return SfBuffer_Index(this, indexSize, indexCount, usage, indices);
}

SfBuffer_Structured SfInstance::CreateStructuredBuffer(
	UINT typeSize, 
	UINT numElements,
	SfUsage usage/* = SfUsage::Static*/,
	EShaderStage defaultShaderStage /*= EShaderStage::Pixel | EShaderStage::Vertex*/, 
	UINT defaultSlot /*= 0*/,
	void* initialData /*= nullptr*/,
	bool unorderedAccess /*= false*/)
{
	return SfBuffer_Structured(
		this,
		typeSize, 
		numElements, 
		defaultShaderStage,
		defaultSlot,
		usage,
		initialData,
		unorderedAccess);	
}

SfBuffer_Constant SfInstance::CreateConstantBuffer(
	UINT typeSize, 
	SfUsage usage/* = SfUsage::Static*/,
	EShaderStage defaultShaderStage /*= EShaderStage::Pixel | EShaderStage::Vertex*/, 
	UINT defaultSlot /*= 0*/,
	void* initialData /*= nullptr*/)
{
	return SfBuffer_Constant(
		this,
		typeSize,
		1,
		defaultShaderStage,
		defaultSlot,
		usage,
		initialData);
}

SfBuffer_Instance SfInstance::CreateInstanceBuffer(UINT typeSize, UINT numElements, SfUsage usage/* = SfUsage::Static*/, void* initialData /*= nullptr*/)
{
	return SfBuffer_Instance(
		this,
		typeSize,
		numElements,
		false,
		initialData);
}

SfBuffer_Raw SfInstance::CreateRawBuffer(
	SfFormat format, 
	UINT numElements, 
	SfUsage usage/* = SfUsage::Static*/,
	EShaderStage defaultShaderStage /*= EShaderStage::Pixel | EShaderStage::Vertex*/, 
	UINT defaultSlot /*= 0*/,
	void* initialData /*= nullptr*/, 
	bool unorderedAccess /*= false*/)
{
	return SfBuffer_Raw(this, format, numElements, defaultShaderStage, defaultSlot, usage, unorderedAccess, initialData);
}

void SfInstance::CreateDevice()
{
	ImmediateContext = std::make_unique<SfContext>();
	ImmediateContext->Data = std::make_shared<SfContext::ContextData>();

	HRESULT hr = D3D11CreateDevice(
		CreationParams.Adapter ? CreationParams.Adapter->GetDXGI() : nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
#if !NDEBUG
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#else
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#endif
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&Device,
		NULL,
		&ImmediateContext->Data->Context);

	ImmediateContext->Data->Instance = this;
}

SfContext SfInstance::GetImmediateContext() const
{
	return *ImmediateContext.get();
}

SfWindow SfInstance::CreateNewWindow(const WindowCreationParams& params)
{
	return SfWindow(params, this);
}

void SfInstance::CreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rast;

	ZeroMemory(&rast, sizeof(D3D11_RASTERIZER_DESC));
	//rast.DepthClipEnable = TRUE;
	//rast.DepthBias = 100000;
	rast.DepthBiasClamp = 0;
	rast.SlopeScaledDepthBias = 1;

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_NONE;
	RasterSolidCullNone.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterSolidCullNone.GetPtrAddress());

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_BACK;
	RasterSolidCullBack.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterSolidCullBack.GetPtrAddress());

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_FRONT;
	RasterSolidCullFront.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterSolidCullFront.GetPtrAddress());

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_NONE;
	RasterWireCullNone.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterWireCullNone.GetPtrAddress());

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_BACK;
	RasterWireCullBack.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterWireCullBack.GetPtrAddress());

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_FRONT;
	RasterWireCullFront.Data = std::shared_ptr<SfRasterizer::RasterizerData>(new SfRasterizer::RasterizerData());
	GetDevice()->CreateRasterizerState(&rast, RasterWireCullFront.GetPtrAddress());
}

void SfInstance::InitDepthStates()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};

	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_NEVER;
	Device->CreateDepthStencilState(&desc, DepthDisabled.GetAddressOf());

	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	Device->CreateDepthStencilState(&desc, DepthReadOnly.GetAddressOf());

	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	Device->CreateDepthStencilState(&desc, DepthReadWrite.GetAddressOf());

	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	Device->CreateDepthStencilState(&desc, DepthWriteOnly.GetAddressOf());
}

void SfInstance::PumpWindowEvents(const SfWindow& window /*= SF_NULL*/)
{
	const SfWindow& win = window ? window : Window;
	sfAssert(win.Data->Instance == this, "cannot pump events on window that does not belong to this instance");

	MSG msg;
	while (PeekMessage(&msg, win.GetHandle(), 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void SfInstance::PumpWindowEvents_Blocking(const SfWindow& window /*= SF_NULL*/)
{
	const SfWindow& win = window ? window : Window;
	sfAssert(win.Data->Instance == this, "cannot pump events on window that does not belong to this instance");

	MSG msg;
	// use GetMessage if an input thread is used
	while (GetMessage(&msg, win.GetHandle(), 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// definitions for shader compilation functions
template SfShader_Vertex SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);
template SfShader_Pixel SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);
template SfShader_Hull SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);
template SfShader_Domain SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);
template SfShader_Geometry SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);
template SfShader_Compute SfInstance::CompileShaderFromFile(std::string filePath, EShaderStage stage);

}