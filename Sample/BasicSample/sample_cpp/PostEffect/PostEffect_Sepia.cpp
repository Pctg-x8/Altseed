﻿#include <Base.h>

/**
	@brief	レイヤーにセピアカラーのポストエフェクトを適用する。
*/
void PostEffect_Sepia()
{
	// Altseedを初期化する
	asd::Engine::Initialize(asd::ToAString("PostEffect_Sepia").c_str(), 640, 480, asd::EngineOption());

	auto texture = asd::Engine::GetGraphics()->CreateTexture2D(asd::ToAString("Data/Texture/Object.png").c_str());

	// シーンのコンストラクタを呼び出す。
	auto scene = std::make_shared<asd::Scene>();

	// レイヤーのコンストラクタを呼び出す。
	auto layer = std::make_shared<asd::Layer2D>();

	// オブジェクトのコンストラクタを呼び出す。
	auto obj = std::make_shared<asd::TextureObject2D>();

	// オブジェクトの位置とテクスチャを設定。
	obj->SetPosition(asd::Vector2DF(100, 100));
	obj->SetTexture(texture);

	// シーンを変更し、そのシーンにレイヤーを追加し、そのレイヤーにオブジェクトを追加する。
	asd::Engine::ChangeScene(scene);
	scene->AddLayer(layer);
	layer->AddObject(obj);

	// レイヤーにセピアカラーのポストエフェクトを適用。
	layer->AddPostEffect(std::make_shared<asd::PostEffectSepia>());

	while (asd::Engine::DoEvents())
	{
		asd::Engine::Update();
	}

	asd::Engine::Terminate();
}