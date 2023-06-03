#pragma once

#include "d3d11_include.h"
#include "rasterizer.h"
#include "shader_program.h"
#include "depth_buffer.h"
#include "buffer.h"
#include "window.h"

namespace sf11
{
	
class SfContext
{
	friend class SfInstance;
protected:

	struct ContextData
	{
		class SfInstance* Instance = nullptr;
		ComPtr<ID3D11DeviceContext> Context;

		// allocate arrays for multi bind here so we dont have to do it every time a bind call is made
		ID3D11ShaderResourceView* SRVsToBind[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT];
		ID3D11SamplerState* SamplersToBind[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		ID3D11RenderTargetView* RTsToBind[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ID3D11Buffer* CBsToBind[D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT];
		ID3D11UnorderedAccessView* ComputeUAVsToBind[D3D11_PS_CS_UAV_REGISTER_COUNT];
		ID3D11UnorderedAccessView* PipelineUAVsToBind[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	};

	std::shared_ptr<ContextData> Data;

public:

	SF_DEF_OPERATORS_AND_DEFAULT(SfContext)
	class SfInstance* GetInstance() const { return Data->Instance; }

	// resets all state associated with this context
	void ClearState();

	// takes a deferred context and executes its command list
	// option to keep the leftover state from the command list in the immediate context state
	void ExecuteDeferredCommands(class SfContext_Deferred* context, bool clearState = true);

	// change the cull mode (clockwise = front) and fill mode
	void SetCullAndFillMode(ECullMode cull, EFillMode fill);

	// draw raw vertices
	void Draw(UINT vertexCount, UINT vertexStart);

	// draw indices when an index buffer is bound
	void DrawIndexed(UINT indexCount, UINT indexStart, int baseVertexLocation);

	// draw indices when an index buffer and instance buffer are bound
	void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, int baseVertex, UINT startInstance);

	// draw vertices when an instance buffer is bound
	void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance);

	// dispatch threads for the currently bound compute shader
	void Dispatch(UINT countX, UINT countY, UINT countZ);

	// bind a texture to be written by a compute shader
	// texture must have had AllowUnorderedAccess flagged true in its params when creating it
	void SetUAVForCS(const class SfResource* view, UINT slot);
	void SetUAVsForCS(const class SfResource** views, UINT count, UINT startSlot);

	// bind shaders
	void BindVertexShader(const struct SfShader_Vertex& shader);
	void BindHullShader(const struct SfShader_Hull& shader);
	void BindDomainShader(const struct SfShader_Domain& shader);
	void BindGeometryShader(const struct SfShader_Geometry& shader);
	void BindPixelShader(const struct SfShader_Pixel& shader);
	void BindComputeShader(const struct SfShader_Compute& shader);

	// binds a full shader program consisting of at least a vertex and pixel shader
	void BindShaderProgram(const class SfShaderProgram& program);
	
	// binds a sampler to a given slot for the given shader stages
	void BindSampler(const class SfSamplerState& sampler, UINT slot, EShaderStage shaderStages);

	// bind multiple samplers for the given shader stages to multiple slots starting with the given start slot
	void BindSamplers(const class SfSamplerState* samplers, UINT startSlot, EShaderStage shaderStages, UINT count = 1);
	void BindSamplers(std::vector<class SfSamplerState>& samplers, UINT startSlot, EShaderStage shaderStages);
	
	// binds the render target associated with the swap chain of the specified window
	// if no window is specified, the first window created by the instance is used
	void BindBackBuffer(const SfDepthBuffer& depthBuffer = SF_NULL, const SfWindow& window = SF_NULL);

	// bind textures
	void BindTexture1D(const class SfTexture1D& tex, UINT slot, EShaderStage stage = EShaderStage::Pixel);
	void BindTexture2D(const class SfTexture2D& tex, UINT slot, EShaderStage stage = EShaderStage::Pixel);
	void BindTexture3D(const class SfTexture3D& tex, UINT slot, EShaderStage stage = EShaderStage::Pixel);
	void BindTexture(const class SfTexture* tex, UINT slot, EShaderStage stage = EShaderStage::Pixel);
	void BindTextures(const class SfTexture** tex, UINT count, UINT slot, EShaderStage stage = EShaderStage::Pixel);
	
	// bind a render target to the first slot for pixel shader output as well as an optional depth buffer
	// use SF_NULL to unbind the render target from the first slot
	void BindRenderTarget(const class SfRenderTarget& target, const class SfDepthBuffer& depth = SF_NULL);

	// bind multiple render targets which can be written from the pixel shader using SV_TARGET0 SV_TARGET1 ...
	void BindRenderTargets(const class SfRenderTarget* targets, UINT count, const class SfDepthBuffer& depth = SF_NULL);

	// unbinds all render targets from the pipeline
	void UnbindAllRenderTargets();
	
	// binds render targets an unordered access views to the pipeline
	// use SetUAVForCS and SetUAVsForCS for compute shader UAVs
	void BindRenderTargetsAndUnorderedAccessViews(
		const class SfRenderTarget* targets, 
		UINT rtCount, 
		const class SfTexture* UAVs, 
		UINT uavCount, 
		UINT uavStartSlot, 
		const class SfDepthBuffer& depth);

	// clears the specified render target with the given color value
	void ClearRenderTarget(const SfRenderTarget& target, float r, float g, float b, float a);

	// change the depth buffer state to ReadOnly, WriteOnly, ReadWrite, or Disabled
	// disables stencil usage
	void SetDepthBufferState(EDepthState state);

	// allows a more specific depth buffer state that allows stencil properies
	void SetDepthStencilState(const class SfDepthStencilState& state, UINT stencilRef = 0);

	// clears a depth/stencil buffer to the specified value
	void ClearDepthBuffer(const class SfDepthBuffer& buffer, float depthClear = 1, UINT8 stencilClear = 0);
	
	// change the way vertices form primitives
	// triangle list, line list, triangle strip, etc
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	// binds a blend state
	// factors modulate the values output by the pixel shader
	// sample mask: the color range to be affected by this blend state
	void BindBlendState(const class SfBlendState& state, float factorR = 1, float factorG = 1, float factorB = 1, float factorA = 1, UINT sampleMask = 0xFFFFFFFF);

	// clears the current blend state
	void ClearBlendState();

	// sets the viewport to the specified dimensions
	void SetViewport(float width, float height, float topLeftX = 0, float topLeftY = 0, float minDepth = 0, float maxDepth = 1);

	// binds a vertex buffer and optional instance buffer
	// also binds an index buffer if one is associated with the vertex buffer
	void BindVertexBuffer(const class SfBuffer_Vertex& buffer, const SfBuffer_Instance& instanceBuffer = SF_NULL);

	// binds an index buffer
	void BindIndexBuffer(const class SfBuffer_Index& buffer);
	
	// binds one or more structured buffers to the specified texture slot for the specified shader stages
	void BindStructuredBuffer(const class SfBuffer_Structured& buffer, UINT slot = -1, EShaderStage stage = EShaderStage::None);
	void BindStructuredBuffers(const class SfBuffer_Structured* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage);

	// binds one or move constant buffers to the specified slot for the specified shader stages
	void BindConstantBuffer(const class SfBuffer_Constant& buffer, UINT slot = -1, EShaderStage stage = EShaderStage::None);
	void BindConstantBuffers(const class SfBuffer_Constant* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage);

	void BindRawBuffer(const class SfBuffer_Raw& buffer, UINT slot = -1, EShaderStage stage = EShaderStage::None);
	void BindRawBuffers(const class SfBuffer_Raw* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage);

	// raw and structured buffers are resource buffers, as well as all textures
	void BindShaderResource(const class SfResource* resource, UINT slot = -1, EShaderStage stage = EShaderStage::None);
	void BindShaderResources(const class SfResource** resources, UINT numBuffers, UINT startSlot, EShaderStage stage);

	//void UpdateTexture(const class SfTexture* texture, void* data, UINT dataSize, UINT bufferOffset);

	// generic resource update function, use this if sub-element updates are needed
	void UpdateResource(const class SfResource* buffer, void* data, UINT dataSize, UINT bufferOffset);

	// updates the specified number of vertices with the given data
	void UpdateVertexBuffer(const class SfBuffer_Vertex& buffer, void* data, UINT numVertices = 0, UINT vertexOffset = 0);

	// updates the specified number of indices with the given data
	void UpdateIndexBuffer(const class SfBuffer_Index& buffer, void* data, UINT numIndices = 0, UINT indexOffset = 0);

	// updates a constant buffer with new data
	void UpdateConstantBuffer(const class SfBuffer_Constant& buffer, void* data, UINT bufferOffset = 0, UINT dataSize = 0);

	// updates the specified number of elements with the given data
	void UpdateStructuredBuffer(const class SfBuffer_Structured& buffer, void* data, UINT numElements = 0, UINT startIndexOffset = 0);

	// updates the specified number of instances with the given data
	void UpdateInstanceBuffer(const class SfBuffer_Instance& buffer, void* data, UINT numInstances = 0, UINT instanceOffset = 0);

	// updates the specified number of raw buffer elements with the given data
	// size of each element is according to the buffer format
	void UpdateRawBuffer(const class SfBuffer_Raw& buffer, void* data, UINT numElements = 0, UINT startIndexOffset = 0);

	void CopyResource(const SfResource& dst, const SfResource& src);
	D3D11_MAPPED_SUBRESOURCE MapResource(const SfResource& res);
	void UnmapResource(const SfResource& res);
};

class SfContext_Deferred : public SfContext
{
	friend class SfContext;

	ComPtr<ID3D11CommandList> CommandList;
public:

	SfContext_Deferred() = default;
	void FinishCommandList(bool clearState = true);
};

};