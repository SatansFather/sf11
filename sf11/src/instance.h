#pragma once

#include "d3d11_include.h"
#include "window_state.h"
#include "rasterizer.h"
#include "window.h"
#include "shader_program.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include "sampler.h"
#include "blend_state.h"
#include "buffer.h"
#include "surface.h"
#include "depth_buffer.h"

namespace sf11
{

struct InstanceCreationParams
{
	InstanceCreationParams() = default;

	WindowCreationParams Window;

	// the graphics device to use, leave nullptr for system default
	class SfAdapter* Adapter = nullptr;

	std::function<void(int, int)> RawMouseCallback;
};

class SfInstance
{
	friend class SfContext;

	ComPtr<ID3D11Device> Device;
	std::unique_ptr<class SfContext> ImmediateContext;
	InstanceCreationParams CreationParams;
	SfWindow Window;
	
	SfRasterizer* CurrentRasterizer = nullptr;
	SfRasterizer RasterSolidCullNone;
	SfRasterizer RasterSolidCullFront;
	SfRasterizer RasterSolidCullBack;
	SfRasterizer RasterWireCullNone;
	SfRasterizer RasterWireCullFront;
	SfRasterizer RasterWireCullBack;

	ComPtr<ID3D11DepthStencilState> DepthReadWrite;
	ComPtr<ID3D11DepthStencilState> DepthReadOnly;
	ComPtr<ID3D11DepthStencilState> DepthWriteOnly;
	ComPtr<ID3D11DepthStencilState> DepthDisabled;

public:

	SfInstance(const InstanceCreationParams& params = InstanceCreationParams());
	~SfInstance();

	// returns the d3d11 device associated with this instance
	// this really isnt needed for api usage, only needed internally
	ID3D11Device* GetDevice() { return Device.Get(); }

	// returns the immediate context associated with this instance's device
	// render commands are issued from this object
	class SfContext GetImmediateContext() const;

	// creates a new window
	// multiple windows can be used with any instance
	SfWindow CreateNewWindow(const WindowCreationParams& params);

	// creates a render target that can be bound via SfContext::BindRenderTarget
	SfRenderTarget CreateRenderTarget(const TextureParams2D& params);

	// compile a shader from an file
	// returns a shader object that can be bound to the pipeline
	// template argument should match the desired return type
	template <typename T>
	T CompileShaderFromFile(std::string filePath, EShaderStage stage);

	// creates a full shader program from the given parameters
	// shader programs represent all shader stages for a pipeline
	// use SfContext::BindShaderProgram to bind all of the shaders in one call
	SfShaderProgram CreateShaderProgram(const ShaderProgramCreateParams& params);

	// deferred contexts can be used to generate command lists to be executed later on
	// usage looks identical the instance's immediate context, however there are no commands issued to the GPU
	// call SfContext_Deferred::FinishCommandList to finalize render commands
	// call ImmediateContext.ExecuteCommandList(deferredContext) to run the deferred commands
	// each context has separate state (bindings etc) so the entire pipeline needs to be setup with each context
	class SfContext_Deferred CreateDeferredContext();

	// create common sampler states
	SfSamplerState CreateSampler_PointWrap();
	SfSamplerState CreateSampler_PointClamp();
	SfSamplerState CreateSampler_PointMirror();
	SfSamplerState CreateSampler_BilinearWrap();
	SfSamplerState CreateSampler_BilinearClamp();
	SfSamplerState CreateSampler_BilinearMirror();

	// create a sampler with any parameters
	SfSamplerState CreateSampler(const D3D11_SAMPLER_DESC& desc);

	// creates a generic depth buffer with no stencil operations enabled
	SfDepthBuffer CreateDepthBuffer(const TextureParams2D& params, bool enableStencil = false);

	// creates a depth/stencil buffer with any desired parameters
	SfDepthStencilState CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc);

	// create a blend state from a single blend description
	SfBlendState CreateBlendState(const SfBlendData& data);

	// create a blend state from an array of blend descriptions
	// each index into the blend data array corresponds to the render target bound to that same slot
	SfBlendState CreateBlendState(SfBlendData* data, UINT count);
	SfBlendState CreateBlendState(std::vector<SfBlendData>& data)
	{
		CreateBlendState(data.data(), (UINT)data.size());
	}

	// creates a texture2d with data from the specified surface
	SfTexture2D CreateTexture2DFromSurface(std::unique_ptr<SfSurface2D> surface);
	// width, height, and format values of params will be replaced
	SfTexture2D CreateTexture2DFromSurface(std::unique_ptr<SfSurface2D> surface, TextureParams2D& params);

	// TODO allow other file types
	// loads a png file into a texture2d
	SfTexture2D LoadTexture2D(const std::string& path);
	// width, height, and format values of params will be replaced
	SfTexture2D LoadTexture2D(const std::string& path, TextureParams2D& params);

	// creates a texture filled with the passed data
	SfTexture1D CreateTexture1D(const TextureParams1D& params, void* data = nullptr);
	SfTexture2D CreateTexture2D(const TextureParams2D& params, void* data = nullptr);
	SfTexture3D CreateTexture3D(const TextureParams3D& params, void* data = nullptr);

	SfBuffer_Vertex CreateVertexBuffer( 
		UINT vertexSize, 
		UINT vertexCount,
		SfUsage usage = SfUsage::Static,
		void* vertices = nullptr);

	template <typename V>
	SfBuffer_Vertex CreateVertexBuffer(std::vector<V>& vertices, SfUsage usage = SfUsage::Static)
	{
		return CreateVertexBuffer(sizeof(V), (UINT)vertices.size(), usage, vertices.data());
	}

	SfBuffer_Index CreateIndexBuffer( 
		UINT indexSize, 
		UINT indexCount,
		SfUsage usage = SfUsage::Static,
		void* indices = nullptr);

	template <typename I>
	SfBuffer_Index CreateIndexBuffer(std::vector<I>& indices, SfUsage usage = SfUsage::Static)
	{
		return CreateIndexBuffer(sizeof(I), (UINT)indices.size(), usage, indices.data());
	}

	SfBuffer_Structured CreateStructuredBuffer(
		UINT typeSize,
		UINT numElements,
		SfUsage usage = SfUsage::Static,
		EShaderStage defaultShaderStage = EShaderStage::Pixel | EShaderStage::Vertex,
		UINT defaultSlot = 0,
		void* initialData = nullptr,
		bool unorderedAccess = false);

	SfBuffer_Constant CreateConstantBuffer(
		UINT typeSize,
		SfUsage usage = SfUsage::Static,
		EShaderStage defaultShaderStage = EShaderStage::Pixel | EShaderStage::Vertex,
		UINT defaultSlot = 0,
		void* initialData = nullptr);

	SfBuffer_Instance CreateInstanceBuffer(
		UINT typeSize,
		UINT numElements,
		SfUsage usage = SfUsage::Static,
		void* initialData = nullptr);

	SfBuffer_Raw CreateRawBuffer(
		SfFormat format, 
		UINT numElements,
		SfUsage usage = SfUsage::Static,
		EShaderStage defaultShaderStage = EShaderStage::Pixel | EShaderStage::Vertex,
		UINT defaultSlot = 0,
		void* initialData = nullptr, 
		bool unorderedAccess = false);

private:

	void CreateDevice();
	void CreateRasterizerStates();
	void InitDepthStates();

public:
	
	void PumpWindowEvents(const SfWindow& window = SF_NULL);
	void PumpWindowEvents_Blocking(const SfWindow& window = SF_NULL);

	SfWindow GetWindow() { return Window; }

};

}