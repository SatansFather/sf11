#include "context.h"
#include "instance.h"
#include "sfassert.h"
#include "depth_buffer.h"
#include "render_target.h"
#include "buffer.h"

namespace sf11
{

void SfContext::ClearState()
{
	Data->Context->ClearState();
}

void SfContext::ExecuteDeferredCommands(class SfContext_Deferred* context, bool clearState /*= true*/)
{
	sfAssert(*this == Data->Instance->GetImmediateContext(), "deferred commands must be executed by the immediate context");
	Data->Context->ExecuteCommandList(context->CommandList.Get(), !clearState);
}

void SfContext::SetCullAndFillMode(ECullMode cull, EFillMode fill)
{
	switch (fill)
	{
		case EFillMode::Solid:
		{
			switch (cull)
			{
				case ECullMode::CullBack:
					Data->Context->RSSetState(Data->Instance->RasterSolidCullBack.Data->State.Get());
					return;
				case ECullMode::CullFront:
					Data->Context->RSSetState(Data->Instance->RasterSolidCullFront.Data->State.Get());
					return;
				case ECullMode::CullNone:
					Data->Context->RSSetState(Data->Instance->RasterSolidCullNone.Data->State.Get());
					return;
			}
		}
		case EFillMode::Wireframe:
		{
			switch (cull)
			{
				case ECullMode::CullBack:
					Data->Context->RSSetState(Data->Instance->RasterWireCullBack.Data->State.Get());
					return;
				case ECullMode::CullFront:
					Data->Context->RSSetState(Data->Instance->RasterWireCullFront.Data->State.Get());
					return;
				case ECullMode::CullNone:
					Data->Context->RSSetState(Data->Instance->RasterWireCullNone.Data->State.Get());
					return;
			}
		}
	}
}

void SfContext::Draw(UINT vertexCount, UINT vertexStart)
{
	Data->Context->Draw(vertexCount, vertexStart);
}

void SfContext::DrawIndexed(UINT indexCount, UINT indexStart, int baseVertexLocation)
{
	Data->Context->DrawIndexed(indexCount, indexStart, baseVertexLocation);
}

void SfContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, int baseVertex, UINT startInstance)
{
	Data->Context->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndex, baseVertex, startInstance);
}

void SfContext::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance)
{
	Data->Context->DrawInstanced(vertexCountPerInstance, instanceCount, startVertex, startInstance);
}

void SfContext::Dispatch(UINT countX, UINT countY, UINT countZ)
{
	Data->Context->Dispatch(countX, countY, countZ);
}

void SfContext::SetUAVForCS(const SfResource* view, UINT slot)
{
	SetUAVsForCS(view && view->Data ? &view : nullptr, 1, slot);
}

void SfContext::SetUAVsForCS(const SfResource** views, UINT count, UINT startSlot)
{
	for (UINT i = 0; i < count; i++)
		Data->ComputeUAVsToBind[i] = views[i] ? views[i]->Data->UnorderedAccess : nullptr;

	Data->Context->CSSetUnorderedAccessViews(startSlot, count, Data->ComputeUAVsToBind, nullptr);
}

void SfContext::BindVertexShader(const struct SfShader_Vertex& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->VSSetShader(shader.GetShader(), nullptr, 0);
	if (shader.GetInputLayout())
		Data->Context->IASetInputLayout(shader.GetInputLayout());
}

void SfContext::BindHullShader(const struct SfShader_Hull& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->HSSetShader(shader.GetShader(), nullptr, 0);
}

void SfContext::BindDomainShader(const struct SfShader_Domain& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->DSSetShader(shader.GetShader(), nullptr, 0);
}

void SfContext::BindGeometryShader(const struct SfShader_Geometry& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->GSSetShader(shader.GetShader(), nullptr, 0);
}

void SfContext::BindPixelShader(const struct SfShader_Pixel& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->PSSetShader(shader.GetShader(), nullptr, 0);
}

void SfContext::BindComputeShader(const SfShader_Compute& shader)
{
	sfAssert(shader.GetInstance() == Data->Instance, "cannot bind shader belonging to another instance");
	Data->Context->CSSetShader(shader.GetShader(), nullptr, 0);
}

void SfContext::BindShaderProgram(const class SfShaderProgram& program)
{
	// creation of shader program enforces vertex + pixel requirement
	// also assures that hull and domain always occur together
	// we do not need to check at this point

	EShaderStage shaders = program.GetActiveShaders();
	BindVertexShader(program.GetVertexShader());
	if (shaders & EShaderStage::Hull) BindHullShader(program.GetHullShader());
	if (shaders & EShaderStage::Domain) BindDomainShader(program.GetDomainShader());
	if (shaders & EShaderStage::Geometry) BindGeometryShader(program.GetGeometryShader());
	BindPixelShader(program.GetPixelShader());
}

void SfContext::BindSampler(const SfSamplerState& sampler, UINT slot, EShaderStage shaderStages)
{
	BindSamplers(&sampler, slot, shaderStages, 1);
}

void SfContext::BindSamplers(const SfSamplerState* samplers, UINT startSlot, EShaderStage shaderStages, UINT count /*= 1*/)
{
	sfAssert(startSlot + count <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT , "cannot bind samplers over slot 16");
	for (UINT i = 0; i < count; i++)
		Data->SamplersToBind[i] = samplers[i].Data->State.Get();

	if (shaderStages & EShaderStage::Vertex)
		Data->Context->VSSetSamplers(startSlot, count, Data->SamplersToBind);
	if (shaderStages & EShaderStage::Pixel)
		Data->Context->PSSetSamplers(startSlot, count, Data->SamplersToBind);
	if (shaderStages & EShaderStage::Hull)
		Data->Context->HSSetSamplers(startSlot, count, Data->SamplersToBind);
	if (shaderStages & EShaderStage::Domain)
		Data->Context->DSSetSamplers(startSlot, count, Data->SamplersToBind);
	if (shaderStages & EShaderStage::Geometry)
		Data->Context->GSSetSamplers(startSlot, count, Data->SamplersToBind);
	if (shaderStages & EShaderStage::Compute)
		Data->Context->CSSetSamplers(startSlot, count, Data->SamplersToBind);
}

void SfContext::BindSamplers(std::vector<class SfSamplerState>& samplers, UINT startSlot, EShaderStage shaderStages)
{
	BindSamplers(samplers.data(), startSlot, shaderStages, (UINT)samplers.size());
}

void SfContext::BindBackBuffer(const SfDepthBuffer& depthBuffer /*= SF_NULL*/, const SfWindow& window /*= SF_NULL*/)
{
	const SfWindow& win = window ? window : Data->Instance->Window;

	sfAssert(win.Data->Instance == Data->Instance, 
		"cannot draw to window that does not belong to this instance");

	Data->Context->OMSetRenderTargets(1, win.Data->BackBuffer.Data->Texture.RenderTargetView.GetAddressOf(), 
		depthBuffer ? depthBuffer.Data->Texture.DepthStencilView.Get() : nullptr);
}

void SfContext::BindTexture1D(const SfTexture1D& tex, UINT slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	BindTexture(tex.Data ? &tex : nullptr, slot, stage);
}

void SfContext::BindTexture2D(const SfTexture2D& tex, UINT slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	BindTexture(tex.Data ? &tex : nullptr, slot, stage);
}

void SfContext::BindTexture3D(const SfTexture3D& tex, UINT slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	BindTexture(tex.Data ? &tex : nullptr, slot, stage);
}

void SfContext::ClearRenderTarget(const SfRenderTarget& target, float r, float g, float b, float a)
{
	float c[] = { r, g, b, a };
	Data->Context->ClearRenderTargetView(target.Data->Texture.RenderTargetView.Get(), c);
}

void SfContext::SetDepthBufferState(EDepthState state)
{
	switch (state)
	{
		case EDepthState::ReadWrite:
			Data->Context->OMSetDepthStencilState(Data->Instance->DepthReadWrite.Get(), 0);
			break;
		case EDepthState::ReadOnly:
			Data->Context->OMSetDepthStencilState(Data->Instance->DepthReadOnly.Get(), 0);
			break;
		case EDepthState::WriteOnly:
			Data->Context->OMSetDepthStencilState(Data->Instance->DepthWriteOnly.Get(), 0);
			break;
		case EDepthState::Disabled:
			Data->Context->OMSetDepthStencilState(Data->Instance->DepthDisabled.Get(), 0);
			break;	
	}
}

void SfContext::SetDepthStencilState(const SfDepthStencilState& state, UINT stencilRef /*= 0*/)
{
	sfAssert(state.Data->Instance == Data->Instance, "cannot use depth stencil state from another instance");
	Data->Context->OMSetDepthStencilState(state.Data->State.Get(), stencilRef);
}

void SfContext::ClearDepthBuffer(const SfDepthBuffer& buffer, float depthClear, UINT8 stencilClear)
{
	Data->Context->ClearDepthStencilView(buffer.Data->Texture.DepthStencilView.Get(), 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
		depthClear, 
		stencilClear);
}

void SfContext::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	Data->Context->IASetPrimitiveTopology(topology);
}

void SfContext::BindBlendState(const SfBlendState& state, float factorR, float factorG, float factorB, float factorA, UINT sampleMask /*= 0xFFFFFFFF*/)
{
	float factor[] = { factorR, factorG, factorB, factorA };
	Data->Context->OMSetBlendState(state.Data->State.Get(), factor, sampleMask);
}

void SfContext::ClearBlendState()
{
	Data->Context->OMSetBlendState(0, 0, 0xFFFFFFFF);
}

void SfContext::SetViewport(float width, float height, float topLeftX /*= 0*/, float topLeftY /*= 0*/, float minDepth /*= 0*/, float maxDepth /*= 1*/)
{
	D3D11_VIEWPORT view 
	{
		.TopLeftX = topLeftX,
		.TopLeftY = topLeftY,
		.Width = width,
		.Height = height,
		.MinDepth = minDepth,
		.MaxDepth = maxDepth,
	};
	Data->Context->RSSetViewports(1, &view);
}

void SfContext::BindVertexBuffer(const SfBuffer_Vertex& buffer, const SfBuffer_Instance& instanceBuffer /*= SF_NULL*/)
{
	if (buffer)
	{
		if (buffer.GetNumElements() > 0)
		{
			if (instanceBuffer)
			{
				UINT offset[] = { 0, 0 };
				UINT stride[] = { buffer.GetTypeSize(), instanceBuffer.GetTypeSize() };
				ID3D11Buffer* buffs[] = 
				{ 
					buffer.Data->Buffer.Buffer.Get(),
					instanceBuffer.Data->Buffer.Buffer.Get() 
				};
				Data->Context->IASetVertexBuffers(0, 2, buffs, stride, offset);
			}
			else
			{
				UINT offset = 0;
				UINT stride = buffer.GetTypeSize();
				Data->Context->IASetVertexBuffers(0, 1, (ID3D11Buffer* const*)buffer.Data->Buffer.Buffer.GetAddressOf(), &stride, &offset);
			}
		}

		SfBuffer_Index ib = buffer.GetLinkedIndexBuffer();
		if (ib && ib.GetNumElements() > 0)
		{
			Data->Context->IASetIndexBuffer(ib.Data->Buffer.Buffer.Get(), ib.Data->Buffer.IndexFormat, 0);
		}

		return;
	}
	
	Data->Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	//Context->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
}

void SfContext::BindIndexBuffer(const class SfBuffer_Index& buffer)
{
	if (buffer)
	{
		Data->Context->IASetIndexBuffer(buffer.Data->Buffer.Buffer.Get(), buffer.Data->Buffer.IndexFormat, 0);
		return;
	}
	Data->Context->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
}

void SfContext::BindStructuredBuffer(const SfBuffer_Structured& buffer, UINT slot /*= -1*/, EShaderStage stage /*= EShaderStage::None*/)
{
	BindShaderResource(buffer.Data ? &buffer : nullptr, slot, stage);
}

void SfContext::BindStructuredBuffers(const SfBuffer_Structured* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage)
{
	BindShaderResources((const SfResource**)(&buffers), numBuffers, startSlot, stage);
}

void SfContext::BindConstantBuffer(const SfBuffer_Constant& buffer, UINT slot /*= -1*/, EShaderStage stage /*= EShaderStage::None*/)
{
	if (buffer)
	{
		if (slot == -1) slot = buffer.GetDefaultSlot();
		if (stage == EShaderStage::None) stage = buffer.GetDefaultStage();
	}
	else
	{
		sfAssert(slot != -1 && (stage & EShaderStage::All), 
			"cannot use default slot or shader stage when binding a null constant buffer");
	}
	BindConstantBuffers(&buffer, 1, slot, stage);
}

void SfContext::BindConstantBuffers(const SfBuffer_Constant* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage)
{
	sfAssert(startSlot + numBuffers <= D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT , "cannot bind constant buffers over slot 15");
	for (UINT i = 0; i < numBuffers; i++)
		Data->CBsToBind[i] = (buffers + i) ? buffers[i].Data->Buffer.Buffer.Get() : nullptr;

	if (stage & EShaderStage::Pixel) 
		Data->Context->PSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);
	if (stage & EShaderStage::Vertex) 
		Data->Context->VSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);
	if (stage & EShaderStage::Hull) 
		Data->Context->HSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);
	if (stage & EShaderStage::Domain) 
		Data->Context->DSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);
	if (stage & EShaderStage::Geometry) 
		Data->Context->GSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);
	if (stage & EShaderStage::Compute) 
		Data->Context->CSSetConstantBuffers(startSlot, numBuffers, Data->CBsToBind);	
}

void SfContext::BindRawBuffer(const class SfBuffer_Raw& buffer, UINT slot /*= -1*/, EShaderStage stage /*= EShaderStage::None*/)
{
	BindShaderResource(buffer.Data ? &buffer : nullptr, slot, stage);
}

void SfContext::BindRawBuffers(const class SfBuffer_Raw* buffers, UINT numBuffers, UINT startSlot, EShaderStage stage)
{
	BindShaderResources((const SfResource**)(&buffers), numBuffers, startSlot, stage);
}

void SfContext::BindShaderResource(const class SfResource* resource, UINT slot /*= -1*/, EShaderStage stage /*= EShaderStage::None*/)
{
	if (resource)
	{
		if (slot == -1) slot = resource->GetDefaultSlot();
		if (stage == EShaderStage::None) stage = resource->GetDefaultStage();
	}
	else
	{
		sfAssert(slot != -1 && (stage & EShaderStage::All), 
			"cannot use default slot or shader stage when binding a null structured buffer");
	}
	BindShaderResources(&resource, 1, slot, stage);
}

void SfContext::BindShaderResources(const class SfResource** resources, UINT numBuffers, UINT startSlot, EShaderStage stage)
{
	sfAssert(startSlot + numBuffers <= D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "cannot bind structured buffers over slot 128");
	for (UINT i = 0; i < numBuffers; i++)
		Data->SRVsToBind[i] = resources[i] ? resources[i]->Data->ShaderResource : nullptr;

	if (stage & EShaderStage::Pixel) 
		Data->Context->PSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
	if (stage & EShaderStage::Vertex) 
		Data->Context->VSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
	if (stage & EShaderStage::Hull) 
		Data->Context->HSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
	if (stage & EShaderStage::Domain) 
		Data->Context->DSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
	if (stage & EShaderStage::Geometry) 
		Data->Context->GSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
	if (stage & EShaderStage::Compute) 
		Data->Context->CSSetShaderResources(startSlot, numBuffers, Data->SRVsToBind);
}

void SfContext::UpdateResource(const class SfResource* buffer, void* data, UINT dataSize, UINT bufferOffset)
{
	sfAssert(buffer, "cannot update null resource");
	sfAssert(data && dataSize > 0, "cannot update resource with empty data");
	sfAssert(buffer->Data->Usage.Value != SfUsage::Immutable, "cannot update immutable resource");

	if (buffer->Data->Usage.Value == SfUsage::Static)
	{
		// TODO allow offset
		sfAssert(bufferOffset == 0, "cannot update offset of static resource");

		Data->Context->UpdateSubresource(buffer->Data->Resource, 0, NULL, data, 0, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mapped = MapResource(*buffer);
		memcpy((char*)(mapped.pData) + bufferOffset, data, dataSize);
		UnmapResource(*buffer);
	}
}

void SfContext::UpdateVertexBuffer(const SfBuffer_Vertex& buffer, void* data, UINT numVertices /*= 0*/, UINT vertexOffset /*= 0*/)
{
	if (numVertices == 0) numVertices = buffer.GetNumElements();
	UpdateResource(&buffer, data, buffer.GetTypeSize() * numVertices, vertexOffset * buffer.GetTypeSize());
}

void SfContext::UpdateIndexBuffer(const SfBuffer_Index& buffer, void* data, UINT numIndices /*= 0*/, UINT indexOffset /*= 0*/)
{
	if (numIndices == 0) numIndices = buffer.GetNumElements();
	UpdateResource(&buffer, data, buffer.GetTypeSize() * numIndices, indexOffset * buffer.GetTypeSize());
}

void SfContext::UpdateConstantBuffer(const SfBuffer_Constant& buffer, void* data, UINT bufferOffset /*= 0*/, UINT dataSize /*= 0*/)
{
	if (dataSize == 0) dataSize = buffer.GetTypeSize();
	UpdateResource(&buffer, data, dataSize, bufferOffset);
}

void SfContext::UpdateStructuredBuffer(const SfBuffer_Structured& buffer, void* data, UINT numElements /*= 0*/, UINT startIndexOffset /*= 0*/)
{	
	if (numElements == 0) numElements = buffer.GetNumElements();
	UINT dataSize = numElements * buffer.GetTypeSize();
	UpdateResource(&buffer, data, dataSize, startIndexOffset * buffer.GetTypeSize());
}

void SfContext::UpdateInstanceBuffer(const SfBuffer_Instance& buffer, void* data, UINT numInstances /*= 0*/, UINT instanceOffset /*= 0*/)
{
	if (numInstances == 0) numInstances = buffer.GetNumElements();
	UINT dataSize = numInstances * buffer.GetTypeSize();
	UpdateResource(&buffer, data, dataSize, instanceOffset * buffer.GetTypeSize());
}

void SfContext::UpdateRawBuffer(const class SfBuffer_Raw& buffer, void* data, UINT numElements /*= 0*/, UINT startIndexOffset /*= 0*/)
{
	if (numElements == 0) numElements = buffer.GetNumElements();
	UINT dataSize = numElements * buffer.GetTypeSize();
	UpdateResource(&buffer, data, dataSize, startIndexOffset * buffer.GetTypeSize());
}

void SfContext::CopyResource(const SfResource& dst, const SfResource& src)
{
	Data->Context->CopyResource(dst.Data->Resource, src.Data->Resource);
}

D3D11_MAPPED_SUBRESOURCE SfContext::MapResource(const SfResource& res)
{
	sfAssert(res.Data.get(), "cannot map null resource");
	sfAssert(!res.Data->IsMapped, "cannot map a resource that is already mapped");

	res.Data->IsMapped = true;
	D3D11_MAP type = res.IsDynamic() ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_READ_WRITE;
	D3D11_MAPPED_SUBRESOURCE mapped;
	Data->Context->Map(res.Data->Resource, 0, type, 0, &mapped);
	return mapped;
}

void SfContext::UnmapResource(const SfResource& res)
{
	sfAssert(res.Data.get(), "cannot unmap null resource");
	sfAssert(res.Data->IsMapped, "cannot unmap a resource that isn't already mapped");

	Data->Context->Unmap(res.Data->Resource, 0);
	res.Data->IsMapped = false;
}

void SfContext::BindTexture(const SfTexture* tex, UINT slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	BindTextures(&tex, 1, slot, stage);
}

void SfContext::BindTextures(const SfTexture** tex, UINT count, UINT slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	BindShaderResources((const SfResource**)tex, count, slot, stage);
}

void SfContext::BindRenderTarget(const SfRenderTarget& target, const SfDepthBuffer& depth /*= SF_NULL*/)
{
	BindRenderTargets(&target, 1, depth);
}

void SfContext::BindRenderTargets(const SfRenderTarget* targets, UINT count, const SfDepthBuffer& depth /*= SF_NULL*/)
{
	sfAssert(count <= 8, "cannot bind more than 8 render targets");
	
	for (UINT i = 0; i < count; i++)
		Data->RTsToBind[i] = targets[i] ? targets[i].Data->Texture.RenderTargetView.Get() : nullptr;
		
	Data->Context->OMSetRenderTargets(count, Data->RTsToBind, depth ? depth.Data->Texture.DepthStencilView.Get() : nullptr);
}

void SfContext::UnbindAllRenderTargets()
{
	memset(Data->RTsToBind, 0, sizeof(size_t) * D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	Data->Context->OMSetRenderTargets(8, Data->RTsToBind, nullptr);
}

void SfContext::BindRenderTargetsAndUnorderedAccessViews(
	const SfRenderTarget* targets, 
	UINT rtCount, 
	const SfTexture* UAVs, 
	UINT uavCount, UINT 
	uavStartSlot, 
	const SfDepthBuffer& depth)
{
	sfAssert(rtCount <= 8, "cannot bind more than 8 render targets");
	sfAssert(uavCount <= 8, "cannot bind more than 8 unordered access views");

	for (UINT i = 0; i < rtCount; i++)
		Data->RTsToBind[i] = targets[i].Data->Texture.RenderTargetView.Get();

	for (UINT i = 0; i < uavCount; i++)
		Data->PipelineUAVsToBind[i] = UAVs[i].Data->UnorderedAccess;

	Data->Context->OMSetRenderTargetsAndUnorderedAccessViews(
		rtCount,
		(ID3D11RenderTargetView* const*)(&Data->RTsToBind),
		depth ? depth.Data->Texture.DepthStencilView.Get() : nullptr,
		uavStartSlot,
		uavCount,
		(ID3D11UnorderedAccessView* const*)(&Data->PipelineUAVsToBind),
		nullptr);
}

void SfContext_Deferred::FinishCommandList(bool clearState)
{
	Data->Context->FinishCommandList(!clearState, &CommandList);
}

}
