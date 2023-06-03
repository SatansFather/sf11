#pragma once

#include "shader.h"
#include "input_layout.h"

namespace sf11
{

enum class EShaderLoadMethod
{
	Invalid,
	Precompiled,
	FromFile,
	FromString
};

struct ShaderCompileInfo
{
	EShaderLoadMethod LoadMethod = EShaderLoadMethod::Invalid;
	std::string EntryPoint = "main"; // main function in shader

	// if method is FromFile
	std::string FilePath; 

	// if method is FromString
	std::string CodeString;	

	// if method is Precompiled
	void* Data;
	size_t Size;
};

struct ShaderProgramCreateParams
{
	SfInputLayout InputLayout;
	ShaderCompileInfo VertexShader;
	ShaderCompileInfo PixelShader;
	ShaderCompileInfo HullShader;
	ShaderCompileInfo DomainShader;
	ShaderCompileInfo GeometryShader;
};

class SfShaderProgram
{
	friend class SfInstance;
	friend class SfContext;

	struct ShaderProgramData
	{
		EShaderStage ActiveShaders = 0;
		class SfInstance* Instance = nullptr;
		SfShader_Vertex Vertex;
		SfShader_Hull Hull;
		SfShader_Domain Domain;
		SfShader_Geometry Geometry;
		SfShader_Pixel Pixel;
	};

	std::shared_ptr<ShaderProgramData> Data;

	void InitFromParams(const ShaderProgramCreateParams& params);
	void ProcessShader(SfShader* shader, const ShaderCompileInfo& info, EShaderStage stage);

public:
	EShaderStage GetActiveShaders() const { return Data->ActiveShaders; }

	SfShader_Vertex&   GetVertexShader()   const { return Data->Vertex; }
	SfShader_Hull&     GetHullShader()     const { return Data->Hull; }
	SfShader_Domain&   GetDomainShader()   const { return Data->Domain; }
	SfShader_Geometry& GetGeometryShader() const { return Data->Geometry; }
	SfShader_Pixel&    GetPixelShader()    const { return Data->Pixel; }

	SF_DEF_OPERATORS_AND_DEFAULT(SfShaderProgram)
};

}