#pragma once

#include <vxLib/math/Vector.h>
#include <d3d12.h>

namespace d3d
{
	struct ResourceDesc : public D3D12_RESOURCE_DESC
	{
		operator D3D12_RESOURCE_DESC&()
		{
			return *this;
		}

		operator const D3D12_RESOURCE_DESC&() const
		{
			return *this;
		}

		static ResourceDesc createDescTexture2D(const vx::uint3 &dim,DXGI_FORMAT format, u16 mipLevels = 1, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
		{
			ResourceDesc desc;
			desc.Alignment = 64 KBYTE;
			desc.Width = dim.x;
			desc.Height = dim.y;
			desc.DepthOrArraySize = (u16)dim.z;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = flags;
			desc.Format = format;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = mipLevels;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;

			return desc;
		}

		static ResourceDesc createDescBuffer(u64 size, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
		{
			ResourceDesc desc;
			desc.Alignment = 64 KBYTE;
			desc.Width = size;
			desc.Height = 1;
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Flags = flags;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;

			return desc;
		}
	};
}