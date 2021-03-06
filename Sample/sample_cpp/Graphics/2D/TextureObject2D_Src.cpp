
#include <Base.h>

/**
@brief	エフェクトを表示する。
*/
void TextureObject2D_Src()
{
	// aceを初期化する
	asd::Engine::Initialize(asd::ToAString("TextureObject2D_Src").c_str(), 640, 480, asd::EngineOption());

	auto obj2 = std::make_shared<asd::TextureObject2D>();
	{

		auto tex2 = asd::Engine::GetGraphics()->CreateTexture2D(asd::ToAString("Data/Texture/Character.png").c_str());

		obj2->SetTexture(tex2);

		obj2->SetPosition(asd::Vector2DF(500, 50));

		asd::Engine::AddObject2D(obj2);
	}

	//クリッピング元画像
	{
		auto obj4 = std::make_shared<asd::TextureObject2D>();

		auto tex4 = asd::Engine::GetGraphics()->CreateTexture2D(asd::ToAString("Data/Texture/Character.png").c_str());

		obj4->SetTexture(tex4);

		obj4->SetPosition(asd::Vector2DF(50, 50));

		asd::Engine::AddObject2D(obj4);
	}

	int count = 0;
	int index = 0;
	// aceが進行可能かチェックする。
	while (asd::Engine::DoEvents())
	{

		int xi = index % 3;
		int yi = (index / 3) % 4;

		//テクスチャの切り出し範囲を指定する。
		obj2->SetSrc(asd::RectF(32 * xi, 32 * yi, 32, 32));

		if (count % 5 == 0)
		{
			index = (++index) % 12;
		}
		++count;

		// aceを更新する。
		asd::Engine::Update();
	}

	// aceを終了する。
	asd::Engine::Terminate();
}