﻿
#include <Altseed.h>

// PackFileを用いて、パッケージから画像を表示するサンプルです。

void PackFile()
{
	// Altseedを初期化する。
	asd::Engine::Initialize(asd::ToAString("PackFile").c_str(), 640, 480, asd::EngineOption());

	// ルートディレクトリを追加する。
	asd::Engine::GetFile()->AddRootDirectory(asd::ToAString("Data/Texture").c_str());

	// オブジェクトを生成する。
	auto obj = std::make_shared<asd::TextureObject2D>();

	// 画像を読み込む。
	auto texture = asd::Engine::GetGraphics()->CreateTexture2D(asd::ToAString("Surface/Tile_Normal.png").c_str());

	// オブジェクトに画像を設定する。
	obj->SetTexture(texture);

	// オブジェクトの位置を設定する。
	obj->SetPosition(asd::Vector2DF(320, 240));

	// エンジンにオブジェクトを追加する。
	asd::Engine::AddObject2D(obj);

	// Altseedのウインドウが閉じられていないか確認する。
	while (asd::Engine::DoEvents())
	{
		// Altseedを更新する。
		asd::Engine::Update();
	}

	//Altseedの終了処理をする。
	asd::Engine::Terminate();

	return;
}
