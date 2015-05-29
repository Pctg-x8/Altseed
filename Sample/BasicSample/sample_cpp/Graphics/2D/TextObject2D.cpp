
#include <ace.h>

// TextObject2D�̃T���v���BTextObject2D��p���ĕ�����\�����Ă��܂��B
void TextObject2D()
{
	// AC-Engine������������B
	ace::Engine::Initialize(ace::ToAString("TextObject2D").c_str(), 640, 480, ace::EngineOption());

	// �V�[���𐶐�����
	auto scene = std::make_shared<ace::Scene>();

	// ���C���[�𐶐�����
	auto layer = std::make_shared<ace::Layer2D>();

	// �V�[���Ƀ��C���[��ǉ�����
	scene->AddLayer(layer);

	// �V�[����؂�ւ���
	ace::Engine::ChangeScene(scene);

	// �t�H���g�𐶐�����
	auto smallFont = ace::Engine::GetGraphics()->CreateFont(ace::ToAString("Data/Font/small.aff").c_str());

	// TextObject2D�̃C���X�^���X�𐶐�����
	auto smallObj = std::make_shared<ace::TextObject2D>();

	// �`��Ɏg���t�H���g��ݒ肷��
	smallObj->SetFont(smallFont);

	// �`��ʒu���w�肷��
	smallObj->SetPosition(ace::Vector2DF(100, 100));

	// �`�悷�镶������w�肷��
	smallObj->SetText(ace::ToAString("������T���v��1").c_str());

	//���C���[�֒ǉ�����
	layer->AddObject(smallObj);

	// AC-Engine�̃E�C���h�E�������Ă��Ȃ����m�F����B
	while (ace::Engine::DoEvents())
	{
		// AC-Engine���X�V����B
		ace::Engine::Update();
	}

	// AC-Engine�̏I������������B
	ace::Engine::Terminate();

	return;
}