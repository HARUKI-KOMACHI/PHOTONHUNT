#pragma once
#include <string>
#include <glad/glad.h>


unsigned int LoadTexture(std::string FileName);
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture_(std::string FileName);
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture_(
	const float r,
	const float g,
	const float b,
	const float a);
unsigned int LoadTexture(std::string FileName, Object_2D::Vec2& size);
unsigned int LoadTexture_GL(std::string FileName, Object_2D::Vec2& size);
void UnloadTexture(unsigned int Texture);
ID3D11ShaderResourceView* SetTexture(unsigned int Texture);
std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTexture();