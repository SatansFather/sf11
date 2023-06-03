#include "input_layout.h"
#include "shader.h"
#include "instance.h"
#include "sfassert.h"

namespace sf11
{

void SfInputLayout::LinkWithVertexShader(const SfShader_Vertex& shader) const
{
	// replace mat4x4 with floats
	// also make sure instance data is not followed by vertex data
	bool foundInstance = false;
	std::vector<SfInputElement> elements;
	for (const SfInputElement& element : Elements)
	{
		if (element.SlotType == EInputSlotType::PerVertex && foundInstance)
			sfAssert(false, "input layout cannot contain vertex data after instance data");
		
		if (element.SlotType == EInputSlotType::PerInstance)
			foundInstance = true;

		if (element.Format.Type != SfFormat::Mat4x4)
		{
			elements.push_back(element);
			continue;
		}

		for (int i = 0; i < 4; i++)
		{
			SfInputElement e;
			e.SemanticName = element.SemanticName;
			e.SemanticIndex = i;
			e.Format.Type = SfFormat::Float;
			e.Format.Channels = 4;
			e.SlotType = element.SlotType;
			elements.push_back(e);
		}
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputlayout;

	UINT instanceSize = 0;

	for (int i = 0; i < elements.size(); i++)
	{
		const SfInputElement& element = elements[i];
		const bool perInstance = element.SlotType == EInputSlotType::PerInstance;

		D3D11_INPUT_ELEMENT_DESC e;
		e.SemanticName = element.SemanticName.c_str();
		e.SemanticIndex = element.SemanticIndex;
		e.InputSlot = perInstance ? 1 : 0;
		e.AlignedByteOffset = perInstance ? instanceSize : D3D11_APPEND_ALIGNED_ELEMENT;

		e.Format = element.Format.GetFormat();

		if (perInstance)
			instanceSize += element.Format.GetTypeSize() * element.Format.Channels;

		e.InputSlotClass = perInstance ?
			D3D11_INPUT_PER_INSTANCE_DATA :
			D3D11_INPUT_PER_VERTEX_DATA;

		e.InstanceDataStepRate = perInstance ? 1 : 0;

		inputlayout.push_back(e);
	}

	shader.GetInstance()->GetDevice()->CreateInputLayout(
		inputlayout.data(), 
		UINT(inputlayout.size()),
		shader.GetShaderCode(),
		shader.GetCodeSize(),
		&(shader.Data->InputLayout)
	);
}

void SfInputLayout::AddElement(const SfInputElement& element)
{
	Elements.push_back(element);
}

}