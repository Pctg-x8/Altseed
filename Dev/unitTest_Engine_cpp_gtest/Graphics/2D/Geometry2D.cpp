﻿#include "../../EngineTest.h"

class CustomLayer : public ace::Layer2D
{
	std::shared_ptr<ace::Texture2D> texture;

public:
	CustomLayer()
	{
		texture = ace::Engine::GetGraphics()->CreateTexture2D(ace::ToAString("Data/Texture/Sample1.png").c_str());
	}
protected:

	virtual void OnDrawAdditionally()
	{
		DrawRectangle(ace::RectF(10, 10, 200, 200), ace::Color(255, 255, 255, 255), ace::RectF(0, 0, 1, 1), nullptr, ace::AlphaBlend::Blend, 0);

		DrawRotatedRectangle(ace::RectF(300, 10, 200, 200), ace::Color(255, 255, 255, 255),
			ace::Vector2DF(100, 100), 45, ace::RectF(0, 0, 1, 1), texture, ace::AlphaBlend::Blend, 0);

		DrawTriangle(ace::Vector2DF(320, 240), ace::Vector2DF(250, 300), ace::Vector2DF(500, 300), ace::Color(255, 255, 255, 255),
			ace::Vector2DF(0, 0), ace::Vector2DF(0, 1), ace::Vector2DF(1, 1), texture, ace::AlphaBlend::Blend, 0);
	}
};

class Graphics_Geometry2D : public EngineTest
{
	std::shared_ptr<ace::Layer2D> layer;
public:

	Graphics_Geometry2D(bool isOpenGLMode) :
		EngineTest(ace::ToAString("Geometry2D"), isOpenGLMode, 60)
	{}

protected:
	void OnStart() override
	{
		auto layer = std::make_shared<CustomLayer>();

		auto scene = std::make_shared<ace::Scene>();

		ace::Engine::ChangeScene(scene);

		scene->AddLayer(layer);
	}
};

ENGINE_TEST(Graphics, Geometry2D)