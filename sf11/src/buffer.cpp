#include "buffer.h"
#include "instance.h"
#include "sfassert.h"

namespace sf11
{

SfBuffer::SfBuffer(class SfInstance* instance, UINT bindFlags, UINT miscFlags, UINT typeSize, 
	UINT numElements, UINT defaultSlot, EShaderStage stage, SfUsage usage, void* initialData, SfFormat format) : 
	//Data(std::make_shared<BufferData>(instance, typeSize, numElements, defaultSlot, stage, dynamic))
	SfResource(instance, usage, defaultSlot, stage)
{
	Data->Buffer = ResourceData::BufferData(typeSize, numElements, defaultSlot, stage);

	DXGI_FORMAT dxFormat = format.Type == SfFormat::NullFormat ? DXGI_FORMAT_UNKNOWN : format.GetFormat();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = bindFlags;
	desc.ByteWidth = typeSize * numElements;
	desc.Usage = usage.GetUsage();
	desc.CPUAccessFlags =
		desc.Usage == D3D11_USAGE_DYNAMIC ? 
			D3D11_CPU_ACCESS_WRITE : 
		desc.Usage == D3D11_USAGE_STAGING ? 
			D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE : 
		0;
	desc.MiscFlags = miscFlags;
	desc.StructureByteStride = typeSize;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srv.Format = dxFormat;
	srv.Buffer.FirstElement = 0;
	srv.Buffer.NumElements = numElements;
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
	ZeroMemory(&uav, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uav.Format = dxFormat;
	uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav.Buffer.Flags = (miscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) ? 0 : D3D11_BUFFER_UAV_FLAG_RAW;
	uav.Buffer.NumElements = numElements;
	
	Data->Buffer.BufferDesc = desc;
	Data->SrvDesc = srv;
	Data->UavDesc = uav;

	Reallocate(typeSize, numElements, usage, initialData);
}

void SfBuffer::Reallocate(UINT typeSize, UINT numElements, SfUsage usage, void* data /*= nullptr*/)
{
	if (Data->Buffer.BufferDesc.BindFlags == D3D11_BIND_CONSTANT_BUFFER)
		sfAssert(numElements == 1, "cannot reallocate constant buffer with anything other than 1 element");

	Data->Buffer.BufferDesc.ByteWidth = typeSize * numElements;
	Data->Buffer.BufferDesc.StructureByteStride = typeSize;
	Data->Buffer.BufferDesc.CPUAccessFlags = usage.Value == SfUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	Data->Buffer.BufferDesc.Usage = usage.Value == SfUsage::Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = data;

	sfAssertHR(Data->Instance->GetDevice()->CreateBuffer(&Data->Buffer.BufferDesc, data ? &sd : NULL, &Data->Buffer.Buffer), 
		"could not create buffer");

	Data->Resource = Data->Buffer.Buffer.Get();

	if (Data->Buffer.BufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE && usage.Value != SfUsage::Staging)
	{
		Data->SrvDesc.Buffer.NumElements = numElements;
		sfAssertHR(Data->Instance->GetDevice()->CreateShaderResourceView(Data->Buffer.Buffer.Get(), &Data->SrvDesc, &Data->Buffer.ShaderResourceView), 
			"could not create SRV for buffer");
		Data->ShaderResource = Data->Buffer.ShaderResourceView.Get();
	}

	if (Data->Buffer.BufferDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		Data->UavDesc.Buffer.NumElements = numElements;
		sfAssertHR(Data->Instance->GetDevice()->CreateUnorderedAccessView(Data->Buffer.Buffer.Get(), &Data->UavDesc, &Data->Buffer.UnorderedAccessView), 
			"could not create UAV for buffer");
		Data->UnorderedAccess = Data->Buffer.UnorderedAccessView.Get();
	}
}

SfBuffer_Structured::SfBuffer_Structured(SfInstance* instance, UINT typeSize, UINT numElements, 
	EShaderStage defaultShaderStage, UINT defaultSlot, SfUsage usage, void* initialData, bool unorderedAccess)
	: SfBuffer(
		instance, 
		D3D11_BIND_SHADER_RESOURCE | (unorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0), 
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, 
		typeSize, 
		numElements, 
		defaultSlot, 
		defaultShaderStage, 
		usage,
		initialData)
{

}

SfBuffer_Constant::SfBuffer_Constant(SfInstance* instance, UINT typeSize, UINT numElements, 
	EShaderStage defaultShaderStage, UINT defaultSlot, SfUsage usage, void* initialData)
	: SfBuffer(
		instance,
		D3D11_BIND_CONSTANT_BUFFER,
		0, 
		typeSize, 
		numElements, 
		defaultSlot, 
		defaultShaderStage, 
		usage,
		initialData)
{

}

SfBuffer_Instance::SfBuffer_Instance(SfInstance* instance, UINT typeSize, 
	UINT numElements, SfUsage usage, void* initialData)
	: SfBuffer(
		instance,
		D3D11_BIND_VERTEX_BUFFER,
		0, 
		typeSize, 
		numElements, 
		0, EShaderStage::None, 
		usage,
		initialData)
{
	
}

SfBuffer_Vertex::SfBuffer_Vertex(SfInstance* instance, UINT vertexSize, 
	UINT numVertices, SfUsage usage, void* initialData)
	: SfBuffer(
		instance,
		D3D11_BIND_VERTEX_BUFFER,
		0, 
		vertexSize, 
		numVertices, 
		0, EShaderStage::None,
		usage,
		initialData)
{
	
}

SfBuffer_Index::SfBuffer_Index(SfInstance* instance, UINT indexSize, 
	UINT numIndices, SfUsage usage, void* initialData)
	: SfBuffer(
		instance,
		D3D11_BIND_INDEX_BUFFER,
		0, 
		indexSize, 
		numIndices, 
		0, EShaderStage::None,
		usage,
		initialData)
{
	Data->Buffer.IndexFormat = indexSize == 1 ? DXGI_FORMAT_R8_UINT : indexSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}


void SfBuffer_Vertex::LinkIndexBuffer(const class SfBuffer_Index& buffer)
{
	Data->Buffer.LinkedBuffer = buffer.Data;
}

SfBuffer_Index SfBuffer_Vertex::GetLinkedIndexBuffer() const
{
	return { Data->Buffer.LinkedBuffer.lock() };
}

void SfBuffer_Vertex::ClearIndexBuffer()
{
	Data->Buffer.LinkedBuffer.reset();
}

SfBuffer_Raw::SfBuffer_Raw(SfInstance* instance, SfFormat format, UINT numElements, 
	EShaderStage defaultShaderStage, UINT defaultSlot, SfUsage usage, bool unorderedAccess, void* initialData)
	: SfBuffer(
		instance,
		D3D11_BIND_SHADER_RESOURCE | (unorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0),
		D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
		format.GetTypeSize() * format.Channels,
		numElements,
		defaultSlot,
		defaultShaderStage,
		usage,
		initialData,
		format)
{

}

};
