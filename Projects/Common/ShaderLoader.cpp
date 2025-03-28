module;
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

export module resource.shader;

import <iostream>;
import <span>;
import <string>;

import utility;

export class ShaderLoader
{
public:
	static ShaderLoader* Default();

public:
	Microsoft::WRL::ComPtr<ID3DBlob> LoadVertexShader(std::wstring_view filename,
		std::span<const D3D_SHADER_MACRO> macros = {});
	Microsoft::WRL::ComPtr<ID3DBlob> LoadPixelShader(std::wstring_view filename,
		std::span<const D3D_SHADER_MACRO> macros = {});

private:
	Microsoft::WRL::ComPtr<ID3DBlob> LoadShader(std::wstring_view filename,
		std::string_view entrypoint, std::string_view target,
		std::span<const D3D_SHADER_MACRO> macros);
};

module :private;

ShaderLoader* ShaderLoader::Default()
{
	static ShaderLoader loader;
	return &loader;
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderLoader::LoadVertexShader(std::wstring_view filename,
	std::span<const D3D_SHADER_MACRO> macros)
{
	return LoadShader(filename, "main", "vs_5_0", macros);
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderLoader::LoadPixelShader(std::wstring_view filename,
	std::span<const D3D_SHADER_MACRO> macros)
{
	return LoadShader(filename, "main", "ps_5_0", macros);
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderLoader::LoadShader(std::wstring_view filename,
	std::string_view entrypoint, std::string_view target,
	std::span<const D3D_SHADER_MACRO> macros)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> compiledShader;
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;

	HRESULT hr = D3DCompileFromFile(
		filename.data(),
		macros.empty() ? nullptr : macros.data(),
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.data(),
		target.data(),
		compileFlags, 0,
		compiledShader.GetAddressOf(),
		errorMessage.GetAddressOf()
	);

	if (FAILED(hr)) {
		if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
			std::cerr << "File not found.\n";
		}

		if (errorMessage) {
			std::cerr << "Shader compile error: " << static_cast<char*>(errorMessage->GetBufferPointer()) << "\n";
		}
	}

	return compiledShader;
}
