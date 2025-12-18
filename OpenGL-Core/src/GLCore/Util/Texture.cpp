#include "glpch.h"
#include "Texture.h"
#include "stb_image.h"

Texture CreateTexture(int width, int height)
{
	Texture result;
	result.Width = width;
	result.Height = height;

	glCreateTextures(GL_TEXTURE_2D, 1, &result.TextureID);

	glTextureStorage2D(result.TextureID, 1, GL_RGBA32F, width, height);

	glTextureParameteri(result.TextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(result.TextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(result.TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(result.TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return result;
}


Texture LoadTexture(const std::string filepath)
{
	int width, height, channels;
	
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

	if (!data)
	{
		std::cerr << "Failed to load texture: " << filepath << "\n";
		return {};
	}

	GLenum format = channels == 4 ? GL_RGBA :
		channels == 3 ? GL_RGB :
		channels == 1 ? GL_RED : 0;

	Texture result;
	result.Width = width;
	result.Height = height;

	glCreateTextures(GL_TEXTURE_2D, 1, &result.TextureID);

	glTextureStorage2D(result.TextureID, 1, (format == GL_RGBA ? GL_RGBA8 : GL_RGB8), width, height);

	glTextureSubImage2D(result.TextureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(result.TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(result.TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(result.TextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(result.TextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateTextureMipmap(result.TextureID);
	stbi_image_free(data);

	return result;
}
