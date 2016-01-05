#pragma once

#include "D3DObject.h"

class RenderProgram
{
	d3d::PipelineState m_pipelineState;
	d3d::RootSignature m_rootSignature;
};