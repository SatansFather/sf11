#include "shader_program.h"
#include "instance.h"
#include "sfassert.h"

namespace sf11
{

void SfShaderProgram::InitFromParams(const ShaderProgramCreateParams& params)
{
	ProcessShader(&Data->Vertex, params.VertexShader, EShaderStage::Vertex);
	if (params.InputLayout.Elements.size() > 0)
		Data->Vertex.LinkInputLayout(params.InputLayout);
	ProcessShader(&Data->Pixel, params.PixelShader, EShaderStage::Pixel);
	ProcessShader(&Data->Geometry, params.GeometryShader, EShaderStage::Geometry);
	ProcessShader(&Data->Hull, params.HullShader, EShaderStage::Hull);
	ProcessShader(&Data->Domain, params.DomainShader, EShaderStage::Domain);

	sfAssert(Data->Hull.IsCompiled() == Data->Domain.IsCompiled(), "hull and domain shaders must occur together");
}

void SfShaderProgram::ProcessShader(SfShader* shader, const ShaderCompileInfo& info, EShaderStage stage)
{
	shader->Data = std::make_shared<SfShader::ShaderData>();
	shader->Data->Instance = Data->Instance;
	shader->Data->Stage = stage;
	switch (info.LoadMethod)
	{
		case EShaderLoadMethod::Invalid:
		{
			sfAssert(shader->GetStage() != EShaderStage::Vertex &&
			         shader->GetStage() != EShaderStage::Pixel,
					 "pipeline shaders must have a vertex and pixel shader");
			break;
		}
		case EShaderLoadMethod::Precompiled:
		{
			break;
		}
		case EShaderLoadMethod::FromFile:
		{
			shader->CompileFromFile(shader->Data->Stage, info.FilePath, info.EntryPoint);
			break;
		}
		case EShaderLoadMethod::FromString:
		{
			shader->CompileFromString(shader->Data->Stage, info.CodeString, info.EntryPoint);
			break;
		}
	}
}

}
