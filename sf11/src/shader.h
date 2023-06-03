#pragma once

#include "d3d11_include.h"
#include "input_layout.h"
#include <string>

namespace sf11
{

struct EShaderStage
{
	enum 
	{
		None         = 0b00000000,
		Vertex       = 0b00000001,
		Pixel        = 0b00000010,
		Hull         = 0b00000100,
		Domain       = 0b00001000,
		Geometry     = 0b00010000,
		Pipeline     = 0b00011111,
		Compute      = 0b00100000,
		All          = 0b00111111
	};

	BYTE Index = None;

	operator BYTE()
	{
		return Index;
	}

	EShaderStage() = default;
	EShaderStage(BYTE index) : Index(index) {}
	EShaderStage(int index) : Index(index) {}
	EShaderStage(const EShaderStage& stage) : Index(stage.Index) {}
};

class SfShader
{
	friend class SfInstance;
	friend class SfShaderProgram;
	friend class SfInputLayout;

protected:

	struct ShaderData
	{
		class SfInstance* Instance = nullptr;
		EShaderStage Stage = 0;
		void* ShaderCode = nullptr;
		UINT CodeSize = 0;
		ComPtr<ID3DBlob> Blob;

		ComPtr<ID3D11InputLayout> InputLayout;

		// cant use ComPtrs in union
		//union
		//{
			ComPtr<ID3D11VertexShader> VertexShader;
			ComPtr<ID3D11PixelShader> PixelShader;
			ComPtr<ID3D11HullShader> HullShader;
			ComPtr<ID3D11DomainShader> DomainShader;
			ComPtr<ID3D11GeometryShader> GeometryShader;
			ComPtr<ID3D11ComputeShader> ComputeShader;
		//};
	};

	std::shared_ptr<ShaderData> Data;

	void CompileFromFile(EShaderStage stage, const std::string& path, const std::string& entryPoint = "main");
	void CompileFromString(EShaderStage stage, const std::string& code, const std::string& entryPoint = "main");
	void FromPrecompiled(EShaderStage stage, void* data, size_t size);
	void InitFromBlob(ID3DBlob* blob);

	void CompileText(EShaderStage stage, const std::string& fileOrString, const std::string& entryPoint, bool isFile);

public:

	class SfInstance* GetInstance() const { return Data->Instance; }
	EShaderStage GetStage() const { return Data->Stage; }
	void* GetShaderCode() const { return Data->ShaderCode; }
	UINT GetCodeSize() const { return Data->CodeSize; }
	bool IsCompiled() const { return Data->Stage.Index & EShaderStage::All; }
};

struct SfShader_Vertex : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Vertex)
	ID3D11VertexShader* GetShader() const { return Data->VertexShader.Get(); }
	ID3D11InputLayout* GetInputLayout() const { return Data->InputLayout.Get(); }
	void LinkInputLayout(const class SfInputLayout& layout);
};

struct SfShader_Pixel : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Pixel)
	ID3D11PixelShader* GetShader() const { return Data->PixelShader.Get(); }
};

struct SfShader_Hull : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Hull)
	ID3D11HullShader* GetShader() const { return Data->HullShader.Get(); }
};

struct SfShader_Domain : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Domain)
	ID3D11DomainShader* GetShader() const { return Data->DomainShader.Get(); }
};

struct SfShader_Geometry : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Geometry)
	ID3D11GeometryShader* GetShader() const { return Data->GeometryShader.Get(); }
};

// compute shaders are not part of the graphics pipeline
struct SfShader_Compute : public SfShader
{
	SF_DEF_OPERATORS_AND_DEFAULT(SfShader_Compute)

	ID3D11ComputeShader* GetShader() const { return Data->ComputeShader.Get(); }
};

}