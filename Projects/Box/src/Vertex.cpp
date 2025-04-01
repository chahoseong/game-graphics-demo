module;
#include <DirectXMath.h>
#include <d3d11.h>

export module vertex;

import <array>;

export namespace Vertex
{
	struct PosColor
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;

		static constexpr const std::array<const D3D11_INPUT_ELEMENT_DESC, 2> Layout = { {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		} };
	};
}

module :private;
