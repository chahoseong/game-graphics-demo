// Windows
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

// ImGui
#include "imgui.h"

import <iostream>;
import <memory>;

import platform.windows;
import utility;
import core;
import pipeline;
import vertex;
import resource.shader;

class Box : public Game
{
private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 WorldViewProjection;
	};

public:
	using Game::Game;

	bool Startup(HWND window) override
	{
		if (!Game::Startup(window)) {
			return false;
		}

		CreateBox();
		InitGraphicsPipeline();
		CreateConstantBuffer();
		CreateRasterizerStates();

		return true;
	}

	void OnRender(ID3D11DeviceContext* context) override
	{
		if (wireframeMode_) {
			pipeline_->SetRasterizerState(wireframeRasterizerState_);
		}
		else {
			pipeline_->SetRasterizerState(solidRasterizerState_);
		}
		pipeline_->Apply(context);

		// Update box transform
		DirectX::XMFLOAT3 boxRotationRadians(
			DirectX::XMConvertToRadians(boxRotation_.x),
			DirectX::XMConvertToRadians(boxRotation_.y),
			DirectX::XMConvertToRadians(boxRotation_.z)
		);
		DirectX::XMMATRIX W = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&boxScale_)) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&boxRotationRadians)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&boxPosition_));

		// Make view matrix
		DirectX::XMFLOAT3 cameraRotationRadians(
			DirectX::XMConvertToRadians(cameraRotation_.x),
			DirectX::XMConvertToRadians(cameraRotation_.y),
			DirectX::XMConvertToRadians(cameraRotation_.z)
		);
		DirectX::XMMATRIX cameraRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&cameraRotationRadians));
		DirectX::XMVECTOR cameraForward = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), cameraRotation);
		DirectX::XMVECTOR cameraFocus = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&cameraPosition_), cameraForward);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&cameraPosition_), cameraFocus, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		// Make projection matrix
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView_), AspectRatio(), 0.1f, 1000.0f);

		// Update box transform
		DirectX::XMStoreFloat4x4(&boxTransform_.WorldViewProjection, DirectX::XMMatrixTranspose(W * V * P));
		
		// Update transform buffer
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(transformBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &boxTransform_, sizeof(boxTransform_));
		context->Unmap(transformBuffer_.Get(), 0);
		
		// Bind constant buffer to vertex shader
		ID3D11Buffer* constantBuffers[] = { transformBuffer_.Get()};
		context->VSSetConstantBuffers(0, 1, constantBuffers);

		// Bind vertex/index buffers
		UINT stride = sizeof(Vertex::PosColor);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(36, 0, 0);

		// UI
		ImGui::Begin("Controls");
		ImGui::BeginGroup();
		ImGui::Text("Box Transform");
		ImGui::Separator();
		ImGui::DragFloat3("Box Position", reinterpret_cast<float*>(&boxPosition_), 0.1f);
		ImGui::DragFloat3("Box Rotation", reinterpret_cast<float*>(&boxRotation_), 0.1f);
		ImGui::DragFloat3("Box Scale", reinterpret_cast<float*>(&boxScale_), 0.1f);
		ImGui::Checkbox("Wireframe", &wireframeMode_);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		ImGui::Text("Camera");
		ImGui::Separator();
		ImGui::DragFloat3("Camera Position", reinterpret_cast<float*>(&cameraPosition_), 0.1f);
		ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float*>(&cameraRotation_), 0.1f);
		ImGui::InputFloat("Field of View", &fieldOfView_, 0.1f);
		ImGui::EndGroup();
		ImGui::End();
	}
	
private:
	void CreateBox()
	{
		std::vector<Vertex::PosColor> vertices(8);
		vertices[0].Position = DirectX::XMFLOAT3(-0.5f, +0.5f, +0.5f);
		vertices[0].Color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		vertices[1].Position = DirectX::XMFLOAT3(+0.5f, +0.5f, +0.5f);
		vertices[1].Color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		vertices[2].Position = DirectX::XMFLOAT3(+0.5f, +0.5f, -0.5f);
		vertices[2].Color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
		vertices[3].Position = DirectX::XMFLOAT3(-0.5f, +0.5f, -0.5f);
		vertices[3].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		vertices[4].Position = DirectX::XMFLOAT3(-0.5f, -0.5f, +0.5f);
		vertices[4].Color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
		vertices[5].Position = DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f);
		vertices[5].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertices[6].Position = DirectX::XMFLOAT3(+0.5f, -0.5f, -0.5f);
		vertices[6].Color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
		vertices[7].Position = DirectX::XMFLOAT3(+0.5f, -0.5f, +0.5f);
		vertices[7].Color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		{
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.ByteWidth = sizeof(Vertex::PosColor) * vertices.size();
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA initialData;
			initialData.pSysMem = vertices.data();
			initialData.SysMemPitch = 0;
			initialData.SysMemSlicePitch = 0;

			ThrowIfFailed(GraphicsDevice()->CreateBuffer(&desc, &initialData, vertexBuffer_.GetAddressOf()));
		}

		std::vector<UINT> indices(36);
		// top
		indices[0] = 0; indices[1] = 1; indices[2] = 2;
		indices[3] = 0; indices[4] = 2; indices[5] = 3;

		// bottom
		indices[6] = 5; indices[7] = 6; indices[8] = 7;
		indices[9] = 5; indices[10] = 7; indices[11] = 4;

		// front
		indices[12] = 3; indices[13] = 2; indices[14] = 6;
		indices[15] = 3; indices[16] = 6; indices[17] = 5;

		// back
		indices[18] = 1; indices[19] = 0; indices[20] = 4;
		indices[21] = 1; indices[22] = 4; indices[23] = 7;

		// left
		indices[24] = 0; indices[25] = 3; indices[26] = 5;
		indices[27] = 0; indices[28] = 5; indices[29] = 4;

		// right
		indices[30] = 2; indices[31] = 1; indices[32] = 7;
		indices[33] = 2; indices[34] = 7; indices[35] = 6;

		{
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.ByteWidth = sizeof(UINT) * indices.size();
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA initialData;
			initialData.pSysMem = indices.data();
			initialData.SysMemPitch = 0;
			initialData.SysMemSlicePitch = 0;

			ThrowIfFailed(GraphicsDevice()->CreateBuffer(&desc, &initialData, indexBuffer_.GetAddressOf()));
		}
	}

	void InitGraphicsPipeline()
	{
		GraphicsPipeline::Description desc;
		desc.InputLayout = { Vertex::PosColor::Layout.begin(), Vertex::PosColor::Layout.end() };
		desc.VertexShader = ShaderLoader::Default()->LoadVertexShader(GetAssetPath(L"Shaders/ColorVertexShader.hlsl"));
		desc.PixelShader = ShaderLoader::Default()->LoadPixelShader(GetAssetPath(L"Shaders/ColorPixelShader.hlsl"));
		pipeline_ = GraphicsPipeline::Create(GraphicsDevice(), desc);
	}

	void CreateRasterizerStates()
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;
		GraphicsDevice()->CreateRasterizerState(&desc, solidRasterizerState_.GetAddressOf());

		desc.CullMode = D3D11_CULL_NONE;
		desc.FillMode = D3D11_FILL_WIREFRAME;
		GraphicsDevice()->CreateRasterizerState(&desc, wireframeRasterizerState_.GetAddressOf());
	}

	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Transform);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		GraphicsDevice()->CreateBuffer(&desc, nullptr, transformBuffer_.GetAddressOf());
	}

private:
	std::unique_ptr<GraphicsPipeline> pipeline_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> solidRasterizerState_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> wireframeRasterizerState_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;

	DirectX::XMFLOAT3 boxPosition_;
	DirectX::XMFLOAT3 boxRotation_;
	DirectX::XMFLOAT3 boxScale_ = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	Transform boxTransform_;

	DirectX::XMFLOAT3 cameraPosition_ = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
	DirectX::XMFLOAT3 cameraRotation_;
	float fieldOfView_ = 90.0f;

	bool wireframeMode_ = false;
};

int main()
{
	Box game(L"Box", 1280, 720, true);
	return Application::Run(&game);
}
