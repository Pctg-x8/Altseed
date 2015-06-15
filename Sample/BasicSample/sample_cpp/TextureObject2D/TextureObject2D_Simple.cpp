﻿
#include <Base.h>

/**
@brief	テクスチャを表示するサンプル。
*/
void TextureObject2D_Simple()
{
	// aceを初期化する
	asd::Engine::Initialize(asd::ToAString("TextureObject2D_Simple").c_str(), 640, 480, asd::EngineOption());

	// テクスチャを読み込む。
	auto texture = asd::Engine::GetGraphics()->CreateTexture2D(asd::ToAString("Data/Texture/Sample1.png").c_str());

	//TextureObject2Dを生成する
	auto obj = std::make_shared<asd::TextureObject2D>();

	//描画に使うテクスチャを設定する
	obj->SetTexture(texture);

	//描画位置を指定する
	obj->SetPosition(asd::Vector2DF(100, 100));

	// オブジェクトのインスタンスをエンジンに追加する。
	asd::Engine::AddObject2D(obj);
	
	// aceが進行可能かチェックする。
	while (asd::Engine::DoEvents())
	{
		// aceを更新する。
		asd::Engine::Update();
	}

	// aceを終了する。
	asd::Engine::Terminate();
}