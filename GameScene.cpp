#include "GameScene.h"

GameScene::GameScene() {
	
}
GameScene::~GameScene() {
	delete input;
	delete title;
	delete player;
	for (int i = 0; i < 10; i++) {
		delete enemy[i];
	}
}

///-----�ϐ��̏�����-----///
void GameScene::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
	//���͂̏�����
	input = new Input();
	input->Initialize(winApp);
	//�X�v���C�g�̏�����
	title->SetSize(XMFLOAT2(1280, 720));
	title->Initialize(dxCommon, WinApp::window_width, WinApp::window_height);
	title->LoadTexture(1, L"Resources/title.png", dxCommon);
	clear->SetSize(XMFLOAT2(1280, 720));
	clear->Initialize(dxCommon, WinApp::window_width, WinApp::window_height);
	clear->LoadTexture(1, L"Resources/clear.png", dxCommon);
	over->SetSize(XMFLOAT2(1280, 720));
	over->Initialize(dxCommon, WinApp::window_width, WinApp::window_height);
	over->LoadTexture(1, L"Resources/over.png", dxCommon);
	sceneNum = TITLE;
	player->SetPosition(XMFLOAT3(0, 0, -25));
	player->SetEye(XMFLOAT3(0, 3, -30));
	player->SetTarget(XMFLOAT3(0, 0, -15));
	for (int i = 0; i < 10; i++) {
		enemy[i] = Object3d::Create();
		isDead[i] = true;
	}
	//����
	srand(time(NULL));
}

///-----�X�V����-----///
void GameScene::Update() {
	input->Update();

	switch (sceneNum) {
	case TITLE:
		if (input->TriggerKey(DIK_SPACE)) {
			sceneNum = GAME;
		}
		break;
		//�Q�[�����[�v
	case GAME:


		///--------------�X�V�������L�q-------------///
		//�����[�v������
		playerVec = { 0.0f,0.0f,0.0f };
		move = { 0,0,0 };

		//�ړ�

		if (input->PushKey(DIK_A)) {
			if (player->GetPositionX() > -4) {
				playerVec.x = -0.4f;
				player->SetPosition(AddXMFLOAT3(player->GetPosition(), playerVec));
			}
		}
		else if (input->PushKey(DIK_D)) {
			if (player->GetPositionX() < 4) {
				playerVec.x = 0.4f;
				player->SetPosition(AddXMFLOAT3(player->GetPosition(), playerVec));
			}
		}
		

		for (int i = 0; i < 10; i++) {
			if (isDead[i] == false) {
				enemy[i]->SetPosition(SubXMFLOAT3(enemy[i]->GetPosition(), XMFLOAT3(0, 0, 1)));
				if (enemy[i]->GetPositionZ() < player->GetPositionZ()) {
					countNum++;
					isDead[i] = true;
				}
			}
			if (CheckHit(enemy[i]) == true) {
				sceneNum = OVER;
			}
		}

		if (countNum == 10) {
			sceneNum = CLEAR;
		}

		if (spornTimer == 0) {
			for (int i = 0; i < 10; i++) {
				if (isDead[i] == true) {
					Sporn(enemy[i]);
					isDead[i] = false;
					break;
				}
			}
		}

		//3d�I�u�W�F�N�g�X�V
		player->Update();
		for (int i = 0; i < 10; i++) {
			enemy[i]->Update();
		}

		spornTimer--;
		break;

		//�N���A���
	case CLEAR:
		if (input->TriggerKey(DIK_SPACE)) {
			Reset();
		}
		break;
		//�Q�[���I�[�o�[
	case OVER:
		if (input->TriggerKey(DIK_SPACE)) {
			Reset();
		}
		break;
	}
}

void GameScene::Draw(DirectXCommon* dxCommon) {
	//�`��O����
	dxCommon->PreDraw();

#pragma region �ŏ��̃V�[���̕`��
	//3D�I�u�W�F�N�g�`��O����
	Object3d::PreDraw(dxCommon->GetCommandList());

	switch (sceneNum) {
	case TITLE:
		break;
	case GAME:
		player->Draw();
		for (int i = 0; i < 10; i++) {
			if (isDead[i] == false) {
				enemy[i]->Draw();
			}
		}
		break;
	case CLEAR:
		break;
	case OVER:
		break;
	}

	//3D�I�u�W�F�N�g�`��O����
	Object3d::PostDraw();

	//�����Ƀ|���S���Ȃǂ̕`�揈��������
	switch (sceneNum) {
	case TITLE:
		title->Draw(dxCommon);
		break;
	case GAME:
		break;
	case CLEAR:
		clear->Draw(dxCommon);
		break;
	case OVER:
		over->Draw(dxCommon);
		break;
	}

#pragma endregion �ŏ��̃V�[���̕`��
	// �`��㏈��
	dxCommon->PostDraw();
}

XMFLOAT3 GameScene::AddXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b) {
	XMFLOAT3 tmp;
	tmp.x = a.x + b.x;
	tmp.y = a.y + b.y;
	tmp.z = a.z + b.z;
	return tmp;
}

XMFLOAT3 GameScene::SubXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b) {
	XMFLOAT3 tmp;
	tmp.x = a.x - b.x;
	tmp.y = a.y - b.y;
	tmp.z = a.z - b.z;
	return tmp;
}

int GameScene::CheckHit(Object3d* object) {
	//�e���W
	float olx = object->GetPositionX() - object->GetScaleX();
	float orx = object->GetPositionX() + object->GetScaleX();
	float oty = object->GetPositionY() + object->GetScaleY();
	float oby = object->GetPositionY() - object->GetScaleY();
	float ofz = object->GetPositionZ() - object->GetScaleZ();
	float obz = object->GetPositionZ() + object->GetScaleZ();
	//�v���C���[���W
	float plx = player->GetPositionX() - player->GetScaleX();
	float prx = player->GetPositionX() + player->GetScaleX();
	float pty = player->GetPositionY() + player->GetScaleY();
	float pby = player->GetPositionY() - player->GetScaleY();
	float pfz = player->GetPositionZ() - player->GetScaleZ();
	float pbz = player->GetPositionZ() + player->GetScaleZ();

	if (olx < prx && plx < orx) {
		if (oby < pty && pby < oty) {
			if (ofz < pbz && pfz < ofz) {
				return true;
			}
		}
	}
}

void GameScene::Reset() {
	for (int i = 0; i < 10; i++) {
		delete enemy[i];
	}
	sceneNum = TITLE;
	player->SetPosition(XMFLOAT3(0, 0, -25));
	player->SetEye(XMFLOAT3(0, 3, -30));
	player->SetTarget(XMFLOAT3(0, 0, -15));
	for (int i = 0; i < 10; i++) {
		enemy[i] = Object3d::Create();
		isDead[i] = true;
	}
	spornTimer = 50;
	countNum = 0;
}

void GameScene::Sporn(Object3d* object) {

	//�����������u
	std::random_device seed_gen;
	std::mt19937_64 engine(seed_gen());
	std::uniform_real_distribution<float>dist(player->GetPositionX() - 1.5, player->GetPositionX() + 1.5);

	//����
	float value = dist(engine);
	//
	object->SetPosition(XMFLOAT3(value, 0, 0));
	spornTimer = 50;
}