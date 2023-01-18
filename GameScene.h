#pragma once
#include "Sprite.h"
#include "Object3d.h"
#include"WinApp.h"
#include"DirectXCommon.h"
#include "Object3d.h"
#include "Input.h"
#include "time.h"
#include <random>
using namespace DirectX;

class GameScene {
public:
	//�����o�֐�
	GameScene();

	~GameScene();

	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

	void Update();

	void Draw(DirectXCommon* dxCommon);

	XMFLOAT3 AddXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b);

	XMFLOAT3 SubXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b);

	int CheckHit(Object3d* object);

	void Reset();

	void Sporn(Object3d* object);

private:
	//���͂̃|�C���^
	Input* input = nullptr;
	//�X�v���C�g�̃|�C���^
	Sprite* title = new Sprite;
	Sprite* clear = new Sprite;
	Sprite* over = new Sprite;
	//�I�u�W�F�N�g�̃|�C���^
	//3D�I�u�W�F�N�g����
	Object3d* player = Object3d::Create();
	Object3d* enemy[10];
	
	
	/////-----�Q�[�����[�v�̕ϐ�-----/////
	XMFLOAT3 playerVec;

	XMFLOAT3 move;
	//�V�[��
	typedef enum scene {
		TITLE,
		GAME,
		CLEAR,
		OVER
	};
	enum scene sceneNum;
	int spornTimer = 50;
	int isDead[10];
	int countNum = 0;
};