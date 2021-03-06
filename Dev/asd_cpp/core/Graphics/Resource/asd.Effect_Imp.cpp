﻿
#include "asd.Effect_Imp.h"
#include "../asd.Graphics_Imp.h"

namespace asd
{
	static void GetParentDir(EFK_CHAR* dst, const EFK_CHAR* src)
	{
		int i, last = -1;
		for (i = 0; src[i] != L'\0'; i++)
		{
			if (src[i] == L'/' || src[i] == L'\\')
				last = i;
		}
		if (last >= 0)
		{
			memcpy(dst, src, last * sizeof(EFK_CHAR));
			dst[last] = L'\0';
		}
		else
		{
			dst[0] = L'\0';
		}
	}

	Effect_Imp::Effect_Imp(Graphics* graphics, Effekseer::Effect* effect)
		: graphics(graphics)
		, m_effect(effect)
	{
		SafeAddRef(graphics);
	}

	Effect_Imp::~Effect_Imp()
	{
		SafeRelease(m_effect);

		auto g = (Graphics_Imp*)graphics;
		g->EffectContainer->Unregister(this);

		SafeRelease(graphics);
	}

	Effect_Imp* Effect_Imp::CreateEffect(Graphics* graphics, Effekseer::Effect* effect)
	{
		return new Effect_Imp(graphics, effect);
	}

	void Effect_Imp::Reload(const achar* path, Effekseer::Setting* setting, void* data, int32_t size)
	{
		EFK_CHAR parentDir[512];
		GetParentDir(parentDir, (const EFK_CHAR*) path);

		auto effect = Effekseer::Effect::Create(setting, data, size, 1.0, parentDir);
		if (effect == nullptr) return;

		SafeRelease(m_effect);
		m_effect = effect;
	}

	void Effect_Imp::ReloadResources(const achar* path)
	{
		EFK_CHAR parentDir[512];
		GetParentDir(parentDir, (const EFK_CHAR*) path);

		m_effect->ReloadResources((const EFK_CHAR*) parentDir);
	}
}