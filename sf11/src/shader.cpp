#include "shader.h"
#include "sfassert.h"
#include "instance.h"
#include <d3dcompiler.h>
#include <map>

// ugly macro but prevents all shaders from copy pasting this
#define CREATE_SHADER(type)                                   \
sfAssertHR(Data->Instance->GetDevice()->Create##type##Shader( \
	blob->GetBufferPointer(),                                 \
	blob->GetBufferSize(),                                    \
	NULL,                                                     \
	&Data->##type##Shader),                                   \
	"could not create shader");

namespace sf11
{

std::map<BYTE, std::string> Profiles = 
{
	{ EShaderStage::Vertex,   "vs_" },
	{ EShaderStage::Hull,     "hs_" },
	{ EShaderStage::Domain,   "ds_" },
	{ EShaderStage::Geometry, "gs_" },
	{ EShaderStage::Pixel,    "ps_" },
	{ EShaderStage::Compute,  "cs_" },
};

void SfShader::CompileFromFile(EShaderStage stage, const std::string& path, const std::string& entryPoint /*= "main"*/)
{
	CompileText(stage, path, entryPoint, true);
}

void SfShader::CompileFromString(EShaderStage stage, const std::string& code, const std::string& entryPoint /*= "main"*/)
{
	CompileText(stage, code, entryPoint, false);
}

void SfShader::FromPrecompiled(EShaderStage stage, void* data, size_t size)
{

}

void SfShader::InitFromBlob(ID3DBlob* blob)
{
	Data->ShaderCode = blob->GetBufferPointer();
	Data->CodeSize = (UINT)blob->GetBufferSize();
	switch (Data->Stage.Index)
	{
		case EShaderStage::Vertex:
			CREATE_SHADER(Vertex);
			break;
		case EShaderStage::Pixel:
			CREATE_SHADER(Pixel);
			break;
		case EShaderStage::Hull:
			CREATE_SHADER(Hull);
			break;
		case EShaderStage::Domain:
			CREATE_SHADER(Domain);
			break;
		case EShaderStage::Compute:
			CREATE_SHADER(Compute);
			break;
		case EShaderStage::Geometry:
			CREATE_SHADER(Geometry);
			break;
		default:
			sfAssert(false, "");
			break;
	}
}

void SfShader::CompileText(EShaderStage stage, const std::string& fileOrString, const std::string& entryPoint, bool isFile)
{
	sfAssert(Profiles.contains(stage), "cannot compile shader of invalid stage");

	std::string profile = Profiles[stage];
	profile += "5_0";

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !NDEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* err = nullptr;

	// dont wrap this in sfAssertHR because we need the compiler errors
	HRESULT hr = 0;
	if (isFile)
	{
		HRESULT hr = D3DCompileFromFile(
			std::wstring(fileOrString.begin(), fileOrString.end()).c_str(),
			NULL,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			profile.c_str(),
			flags,
			0,
			&Data->Blob,
			&err
		);
	}
	else
	{
		HRESULT hr = D3DCompile(
			std::wstring(fileOrString.begin(), fileOrString.end()).c_str(),
			fileOrString.size(),
			NULL,
			NULL,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			profile.c_str(),
			flags,
			0,
			&Data->Blob,
			&err
		);
	}

	// provide error messages
	//if (FAILED(hr))
	{
		char* error = nullptr;
		if (err)
		{
			error = (char*)err->GetBufferPointer();
			std::string e(error);
			if (e.find("error") != std::string::npos)
			{
				sfAssert(false, (std::string("failed to compile shader ") + fileOrString + std::string("\n\n") + e).c_str());
				err->Release();
			}
		}
		else
		{
			sfAssertHR(hr, std::string("failed find shader\n \"" + fileOrString + "\"\n").c_str());
		}
	}

	sfAssert(Data->Blob, "blob was null after compiling shader");
	InitFromBlob(Data->Blob.Get());
}

void SfShader_Vertex::LinkInputLayout(const class SfInputLayout& layout)
{
	layout.LinkWithVertexShader(*this);
}

}
