#include <d3d11.h>
#include <wrl.h>

import <memory>;

import platform.windows;
import core;
import utility;
import pipeline;
import vertex;
import resource.shader;

class Triangle : public Game
{
public:
	Triangle(std::wstring_view title, int width, int height, bool windowed)
		: Game(title, width, height, windowed)
	{

	}

	bool Startup(HWND window) override
	{
		if (!Game::Startup(window))
		{
			return false;
		}

		GraphicsPipeline::Description pipelineDesc;
		pipelineDesc.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		pipelineDesc.InputLayout = { Vertex::PosColor::Layout.begin(), Vertex::PosColor::Layout.end() };
		pipelineDesc.VertexShader = ShaderLoader::Default()->LoadVertexShader(GetAssetPath(L"Shaders/ColorVertexShader.hlsl"));
		pipelineDesc.PixelShader = ShaderLoader::Default()->LoadPixelShader(GetAssetPath(L"Shaders/ColorPixelShader.hlsl"));
		pipeline_ = GraphicsPipeline::Create(GraphicsDevice(), pipelineDesc);

		std::vector<Vertex::PosColor> triangle(3);
		triangle[0].Position = DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f);
		triangle[0].Color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		triangle[1].Position = DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f);
		triangle[1].Color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		triangle[2].Position = DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f);
		triangle[2].Color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(Vertex::PosColor) * triangle.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = triangle.data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		GraphicsDevice()->CreateBuffer(&vertexBufferDesc, &initData, vertexBuffer_.GetAddressOf());

		return true;
	}

	void Shutdown() override
	{
		Game::Shutdown();
	}

	void Render(ID3D11DeviceContext* immediateContext) override
	{
		UINT stride = sizeof(Vertex::PosColor);
		UINT offset = 0;
		immediateContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

		pipeline_->Apply(immediateContext);

		immediateContext->Draw(3, 0);
	}

private:
	std::unique_ptr<GraphicsPipeline> pipeline_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
	std::vector<Vertex::PosColor> vertices_;
};

int main()
{
	Triangle game(L"Triangle", 1280, 720, true);
	return Application::Run(&game);
}