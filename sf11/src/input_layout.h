#pragma once

#include "d3d11_include.h"
#include <map>
#include <string>
#include <vector>
#include "format.h"

namespace sf11
{

enum class EInputSlotType
{
	PerVertex,
	PerInstance
};

struct SfInputElement
{
	SfInputElement() = default;
	SfInputElement(const std::string& name, SfFormat format, 
		EInputSlotType slotType = EInputSlotType::PerVertex, UINT semanticIndex = 0)
		: SemanticName(name), Format(format), SlotType(slotType), SemanticIndex(semanticIndex) {}

	std::string SemanticName;
	SfFormat Format;
	EInputSlotType SlotType;
	UINT SemanticIndex;
};

class SfInputLayout
{
	friend struct SfShader_Vertex;
	friend class SfShaderProgram;

	// a description of each vertex
	std::vector<SfInputElement> Elements;
public:
	
	SfInputLayout() = default;
	SfInputLayout(const std::vector<SfInputElement>& elements) : Elements(elements) {}
	SfInputLayout(const std::initializer_list<SfInputElement>& elements) : Elements(elements) {}

	void LinkWithVertexShader(const struct SfShader_Vertex& shader) const;
	void AddElement(const SfInputElement& element);
};

}