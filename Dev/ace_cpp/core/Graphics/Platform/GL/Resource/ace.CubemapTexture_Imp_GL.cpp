﻿
#include "ace.CubemapTexture_Imp_GL.h"
#include "../ace.Graphics_Imp_GL.h"

namespace ace
{
	CubemapTexture_Imp_GL::CubemapTexture_Imp_GL(Graphics* graphics, GLuint cubemapTexture, Vector2DI size, int32_t mipmapCount)
		: CubemapTexture_Imp(graphics)
		, m_cubemapTexture(cubemapTexture)
		, size(size)
		, mipmapCount(mipmapCount)
	{

	}

	CubemapTexture_Imp_GL::~CubemapTexture_Imp_GL()
	{

	}

	CubemapTexture_Imp* CubemapTexture_Imp_GL::Create(Graphics_Imp* graphics, const achar* front, const achar* left, const achar* back, const achar* right, const achar* top, const achar* bottom)
	{
		auto loadFile = [](const achar* path, std::vector<uint8_t>& dst)-> bool
		{
#if _WIN32
			auto fp = _wfopen(path, L"rb");
			if (fp == nullptr) return false;
#else
			auto fp = fopen(ToUtf8String(path).c_str(), "rb");
			if (fp == nullptr) return false;
#endif
			fseek(fp, 0, SEEK_END);
			auto size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			dst.resize(size);
			fread(dst.data(), 1, size, fp);
			fclose(fp);

			return true;
		};

		int32_t width = 0;
		int32_t height = 0;
		GLuint cubemapTexture = 0;
		std::vector<uint8_t> nulldata;

		int32_t widthes[6];
		int32_t heights[6];
		std::vector<uint8_t> fileBuffers[6];
		uint8_t* buffers[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

		static const GLenum target[] = {
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		};

		const achar* pathes[] = {
			left,
			top,
			back,			
			right,
			bottom,
			front,
		};

		for (int32_t i = 0; i < 6; i++)
		{
			if (!loadFile(pathes[i], fileBuffers[i]))
			{
				return nullptr;
			}
		}

		for (int32_t i = 0; i < 6; i++)
		{
			void* result = nullptr;
			if (ImageHelper::LoadPNGImage(fileBuffers[i].data(), fileBuffers[i].size(), true, widthes[i], heights[i], result))
			{
				buffers[i] = (uint8_t*) result;
			}
			else
			{
				goto End;
			}
		}


		width = widthes[0];
		height = heights[0];

		for (int32_t i = 0; i < 6; i++)
		{
			if (widthes[i] != width) goto End;
			if (heights[i] != height) goto End;
		}

		int32_t mipmapCount;
		mipmapCount = ImageHelper::GetMipmapCount(width, height);
		nulldata.resize(width * height * 4);
		for (size_t i = 0; i < nulldata.size(); i++)
		{
			nulldata[i] = 0;
		}

		glGenTextures(1, &cubemapTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		for (int i = 0; i < 6; i++)
		{
			for (auto m = 0; m < mipmapCount; m++)
			{
				auto w = width;
				auto h = height;
				ImageHelper::GetMipmapSize(m, w, h);
				if (m == 0)
				{
					glTexImage2D(target[i], m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffers[i]);
				}
				else
				{
					glTexImage2D(target[i], m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nulldata.data());
				}
				GLCheckError();
			}
		}

		for (int32_t i = 0; i < 6; i++)
		{
			SafeDeleteArray(buffers[i]);
		}

		GLCheckError();

		return new CubemapTexture_Imp_GL(graphics, cubemapTexture, Vector2DI(width, height), mipmapCount);

	End:;

		for (int32_t i = 0; i < 6; i++)
		{
			SafeDeleteArray(buffers[i]);
		}
		GLCheckError();

		return nullptr;
	}
}