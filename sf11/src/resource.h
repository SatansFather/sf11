#pragma once

#include "d3d11_include.h"
#include "texture_params.h"
#include "shader.h"
#include <memory>
#include "usage.h"

namespace sf11
{

// base resource class for textures and buffers
class SfResource
{
	friend class SfInstance;
	friend class SfContext;
	friend class SfWindow;
	
protected:

	struct ResourceData
	{
		struct BufferData
		{
			ComPtr<ID3D11Buffer> Buffer;
			ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
			ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;
			UINT TypeSize = 0;
			UINT NumElements = 0;

			D3D11_BUFFER_DESC BufferDesc;
			DXGI_FORMAT IndexFormat;

			// associated index or vertex buffer to be bound at the same time as this
			std::weak_ptr<ResourceData> LinkedBuffer;

			BufferData() = default;
			BufferData(UINT typeSize, UINT numElements, 
				UINT defaultSlot, EShaderStage stage) : 
				TypeSize(typeSize),
				NumElements(numElements)
			{}
		};

		struct TextureData
		{

			ComPtr<ID3D11ShaderResourceView> ShaderResource;
			ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;
			ComPtr<ID3D11DepthStencilView> DepthStencilView;
			ComPtr<ID3D11RenderTargetView> RenderTargetView;

			UINT Width = 0;
			UINT Height = 0;
			UINT Depth = 0;
			UINT Dimensions = 0;

			// cannot put ComPtrs in a union
			ComPtr<ID3D11Texture1D> Texture1D;
			ComPtr<ID3D11Texture2D> Texture2D;
			ComPtr<ID3D11Texture3D> Texture3D;

			union
			{
				D3D11_TEXTURE1D_DESC TextureDesc1D;
				D3D11_TEXTURE2D_DESC TextureDesc2D;
				D3D11_TEXTURE3D_DESC TextureDesc3D;
			};

			//union
			//{
				TextureParams1D Params1D;
				TextureParams2D Params2D;
				TextureParams3D Params3D;
			//};

			TextureData() = default;
			~TextureData() {}
		};

		class SfInstance* Instance = nullptr;

		ID3D11ShaderResourceView* ShaderResource = nullptr;
		ID3D11UnorderedAccessView* UnorderedAccess = nullptr;
		ID3D11Resource* Resource = nullptr;
		D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
		D3D11_UNORDERED_ACCESS_VIEW_DESC UavDesc;
		SfUsage Usage = SfUsage::Static;
		EShaderStage DefaultStage = EShaderStage::None;
		UINT DefaultSlot = 0;
		bool IsMapped = false;
		
	//	union
	//	{
			BufferData Buffer;
			TextureData Texture;
	//	};

		ResourceData(SfInstance* instance, SfUsage usage = SfUsage::Static, UINT defaultSlot = -1, EShaderStage defaultStage = EShaderStage::Pixel) 
			: Instance(instance), Usage(usage), DefaultSlot(defaultSlot), DefaultStage(defaultStage) {}
		~ResourceData() {}
	};

	std::shared_ptr<ResourceData> Data;
	SfResource() = default;
	SfResource(SfInstance* instance, SfUsage usage = SfUsage::Static, UINT defaultSlot = -1, EShaderStage defaultStage = EShaderStage::Pixel) 
		: Data(std::make_shared<ResourceData>(instance, usage, defaultSlot, defaultStage)) {}

	// returns the default shader slot this buffer should bind to
	// only applies to constant and structured buffers
	UINT GetDefaultSlot() const { return Data->DefaultSlot; }

	// set the default slot for a structured or constant buffer
	void SetDefaultSlot(UINT slot) { Data->DefaultSlot = slot; }

	// returns the default shader stage this buffer should bind to
	// only applies to constant and structured buffers
	EShaderStage GetDefaultStage() const { return Data->DefaultStage; }

	// sets the default shader stage this buffer should bind to
	void SetDefaultStage(EShaderStage stage) { Data->DefaultStage = stage; }

	bool IsDynamic() const { return Data->Usage.Value == SfUsage::Dynamic; }
	bool IsStaging() const { return Data->Usage.Value == SfUsage::Staging; }
	bool IsStatic() const { return Data->Usage.Value == SfUsage::Static; }
	bool IsImmutable() const { return Data->Usage.Value == SfUsage::Immutable; }
	SfUsage GetUsage() const { return Data->Usage; }

	ID3D11Resource* GetResource() const { return Data->Resource; }

	typedef ResourceData::TextureData TextureData;
	typedef ResourceData::BufferData BufferData;
};

}