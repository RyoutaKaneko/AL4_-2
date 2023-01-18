#define DIRECTINPUT_VERSION    0x0800 //DirectInputのバージョン指定
#include"WinApp.h"
#include"DirectXCommon.h"
#include "Object3d.h"

#include<windows.h>
#include<cassert>
#include<vector>
#include<string>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<dinput.h>
#include<DirectXTex.h>
#include "GameScene.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")

using namespace Microsoft::WRL;

// ウィンドウプロシージャ
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//定数バッファ用データ構造体(マテリアル)
struct ConstBufferDataMaterial {
	XMFLOAT4 color;//色(RGBA)
};

//定数バッファ用データ構造体(3D変換行列)
struct ConstBufferDataTransform {
	XMMATRIX mat; //3D変換行列
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

#pragma region 基盤システムの初期化
	//WindowsAPIのポインタ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();
	//DirectXのポインタ
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);
	//ゲームシーン
	GameScene* gameScene = nullptr;
	//スプライトのポインタ
	Sprite* sprite = new Sprite;
	//スプライトの初期化
	sprite->Initialize(dxCommon, WinApp::window_width, WinApp::window_height);
	//3Dオブジェクト静的初期化
	Object3d::StaticInitialize(dxCommon->GetDevice(), WinApp::window_width, WinApp::window_height);
#pragma endregion 基盤システムの初期化

	//ゲームシーン
	gameScene = new GameScene();
	gameScene->Initialize(winApp, dxCommon);
///-------------ゲームループ---------------///
	while (true)
	{
#pragma region 基盤システムの更新

		//Windowsのメッセージ処理
		if (winApp->ProcessMessage())
		{
			//ゲームループを抜ける
			break;
		}

		//入力の更新
		sprite->Update();
		gameScene->Update();

#pragma endregion 基盤システムの更新
		
		gameScene->Draw(dxCommon);
	}

#pragma region 最初のシーンの終了

	//ここにポリゴンの初期化処理を書く

#pragma endregion 最初のシーンの終了

#pragma region 基盤システムの終了

	//3Dオブジェクト解放
	delete gameScene;

	//WindowsAPIの終了処理
	winApp->Finalize();

	//DirectX解放
	delete dxCommon;

	//sprite
	delete sprite;

	//WindowsAPI解放
	delete winApp;

#pragma endregion 基盤システムの終了

	return 0;
}