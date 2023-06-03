#pragma once

#include "d3d11_include.h"
#include "resource.h"

namespace sf11
{

class SfBuffer : public SfResource
{
	friend class SfInstance;
	friend class SfContext;

protected:


	SfBuffer(class SfInstance* instance, UINT bindFlags, UINT miscFlags, UINT typeSize, UINT numElements, 
		UINT defaultSlot, EShaderStage stage, SfUsage usage, void* initialData, SfFormat format = SfFormat::NullFormat);

	SfBuffer() = default;

public:

	// returns the number of elements (of TypeSize) that this buffer is allocated to hold
	// does not apply to constant buffers
	UINT GetNumElements() const { return Data->Buffer.NumElements; }

	// returns the size of the data type this buffer is intended to contain
	UINT GetTypeSize() const { return Data->Buffer.TypeSize; }

	// reallocate this buffer with new parameters
	// optionally initialize it with data
	void Reallocate(UINT typeSize, UINT numElements, SfUsage usage, void* data = nullptr);
};

class SfBuffer_Structured : public SfBuffer
{
	friend class SfInstance;
	friend class SfContext;

	SfBuffer_Structured(
		SfInstance* instance, 
		UINT typeSize, 
		UINT numElements, 
		EShaderStage defaultShaderStage,
		UINT defaultSlot,
		SfUsage usage,
		void* initialData,
		bool unorderedAccess);

public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Structured)
};

class SfBuffer_Constant : public SfBuffer
{
	friend class SfInstance;

	SfBuffer_Constant(
		SfInstance* instance, 
		UINT typeSize, 
		UINT numElements, 
		EShaderStage defaultShaderStage,
		UINT defaultSlot,
		SfUsage usage,
		void* initialData);
public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Constant)
};

class SfBuffer_Instance : public SfBuffer
{
	friend class SfInstance;

	SfBuffer_Instance(
		SfInstance* instance, 
		UINT typeSize, 
		UINT numElements,
		SfUsage usage,
		void* initialData);
public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Instance)
};

class SfBuffer_Vertex : public SfBuffer
{
	friend class SfBuffer_Index;
	friend class SfInstance;

	SfBuffer_Vertex(
		SfInstance* instance, 
		UINT vertexSize, 
		UINT numVertices,
		SfUsage usage,
		void* initialData);

public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Vertex)
	void LinkIndexBuffer(const class SfBuffer_Index& buffer);
	class SfBuffer_Index GetLinkedIndexBuffer() const;
	void ClearIndexBuffer();
};

class SfBuffer_Index : public SfBuffer
{
	friend class SfBuffer_Vertex;
	friend class SfInstance;

	SfBuffer_Index(
		SfInstance* instance, 
		UINT indexSize, 
		UINT numIndices,
		SfUsage usage,
		void* initialData);

	// used for GetAssociatedBuffer functions
	SfBuffer_Index(std::shared_ptr<SfResource::ResourceData> data) { Data = data; }

public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Index)
};

class SfBuffer_Raw : public SfBuffer
{
	friend class SfBuffer_Vertex;
	friend class SfInstance;

	SfBuffer_Raw(
		SfInstance* instance, 
		SfFormat format,
		UINT numElements,
		EShaderStage defaultShaderStage,
		UINT defaultSlot,
		SfUsage usage,
		bool unorderedAccess,
		void* initialData);

public:
	SF_DEF_OPERATORS_AND_DEFAULT(SfBuffer_Raw)
};


}