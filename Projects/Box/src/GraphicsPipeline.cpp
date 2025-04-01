module;
#include <d3d11.h>
#include <wrl.h>

export module pipeline;

import <memory>;
import <span>;
import <vector>;

import utility;

export class GraphicsPipeline
{
public:
	struct Description
	{
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayout;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState;
		Microsoft::WRL::ComPtr<ID3DBlob> VertexShader;
		Microsoft::WRL::ComPtr<ID3DBlob> PixelShader;
	};

public:
	static std::unique_ptr<GraphicsPipeline> Create(ID3D11Device* device, const Description& desc);

public:
	void Apply(ID3D11DeviceContext* context);

	void SetRasterizerState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState);

private:
	GraphicsPipeline() = default;

private:
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
};

module :private;

std::unique_ptr<GraphicsPipeline> GraphicsPipeline::Create(ID3D11Device* device, const Description& desc)
{
	std::unique_ptr<GraphicsPipeline> pipeline = std::unique_ptr<GraphicsPipeline>(new GraphicsPipeline());
	pipeline->primitiveTopology_ = desc.PrimitiveTopology;
	ThrowIfFailed(device->CreateInputLayout(desc.InputLayout.data(), desc.InputLayout.size(), desc.VertexShader->GetBufferPointer(), desc.VertexShader->GetBufferSize(), &pipeline->inputLayout_));
	ThrowIfFailed(device->CreateVertexShader(desc.VertexShader->GetBufferPointer(), desc.VertexShader->GetBufferSize(), nullptr, &pipeline->vertexShader_));
	ThrowIfFailed(device->CreatePixelShader(desc.PixelShader->GetBufferPointer(), desc.PixelShader->GetBufferSize(), nullptr, &pipeline->pixelShader_));
	pipeline->rasterizerState_ = desc.RasterizerState;
	return pipeline;
}

void GraphicsPipeline::Apply(ID3D11DeviceContext* context)
{
	context->IASetPrimitiveTopology(primitiveTopology_);
	context->IASetInputLayout(inputLayout_.Get());
	context->VSSetShader(vertexShader_.Get(), nullptr, 0);
	context->PSSetShader(pixelShader_.Get(), nullptr, 0);
	context->RSSetState(rasterizerState_.Get());
}

void GraphicsPipeline::SetRasterizerState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState)
{
	rasterizerState_ = rasterizerState;
}
