#include "SceneManager.h"
#include "Input/Input.h"
#include "SceneFactory/SceneFactory.h"

#include "Engine/EngineUtils/FrameInfo/FrameInfo.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"

#include "imgui.h"

std::unique_ptr<SceneManager> SceneManager::pInstance_;

void SceneManager::Initialize()
{
	pInstance_ = std::make_unique<SceneManager>();
}

void SceneManager::InstanceReset()
{
	pInstance_.reset();
}

SceneManager* const SceneManager::GetInstance()
{
	return pInstance_.get();
}

void SceneManager::Initialize(std::optional<BaseScene::ID> firstScene, std::optional<BaseScene::ID> finishID) {
	finishID_ = finishID;
	preSceneID_ = firstScene.value();


	fade_ = std::make_unique<Fade>();

	frameInfo_ = FrameInfo::GetInstance();
	input_ = Input::GetInstance();

	SceneFactory* const sceneFactory = SceneFactory::GetInstance();

	scene_.reset(sceneFactory->CreateBaseScene(firstScene));
	scene_->SceneInitialize(this);
	scene_->Load();
	scene_->Initialize();


	load_ = std::make_unique<SceneLoad>();


#ifdef USE_DEBUG_CODE
	sceneName_[BaseScene::ID::Title] = "Title";
	sceneName_[BaseScene::ID::Game] = "Game";
	sceneName_[BaseScene::ID::StageSelect] = "Select";
	sceneName_[BaseScene::ID::Result] = "Result";
	sceneName_[BaseScene::ID::Test] = "Test";
#endif // USE_DEBUG_CODE
	sceneNum_;
	sceneNum_[BaseScene::ID::Title] = DIK_1;
	sceneNum_[BaseScene::ID::Game] = DIK_2;
	sceneNum_[BaseScene::ID::StageSelect] = DIK_3;
	sceneNum_[BaseScene::ID::Result] = DIK_4;
	sceneNum_[BaseScene::ID::Test] = DIK_5;

	// テクスチャデータのアップロード
	UploadTextureData();

}

void SceneManager::SceneChange(std::optional<BaseScene::ID> next) {
	if (next_ || fade_->InEnd()
		|| fade_->OutEnd() || fade_->IsActive()
		)
	{
		return;
	}
	SceneFactory* const sceneFactory = SceneFactory::GetInstance();

	next_.reset(sceneFactory->CreateBaseScene(next));
	next_->SceneInitialize(this);


	fade_->OutStart();
}

void SceneManager::Update() {
	if (input_->GetGamepad()->PushAnyKey()) {
		isPad_ = true;
	}
	else if (input_->GetMouse()->PushAnyKey() || input_->GetKey()->PushAnyKey()) {
		isPad_ = false;
	}


	if (scene_ && !next_) {
#ifdef USE_DEBUG_CODE
		scene_->ChangeCamera();
#endif // USE_DEBUG_CODE

		scene_->Update();
		Debug();
	}

	// フェードの更新処理
	fade_->Update();



	if (fade_->OutEnd()) {
		// ロード中の描画を開始
		load_->Start();

#pragma region シーン切り替え
		// 前のシーンのIDを保存
		preSceneID_ = scene_->GetID();

		// シーン終わり処理
		scene_->Finalize();
		// 次のシーンへ
		scene_.reset(next_.release());
		// 次のシーンを格納するユニークポインタをリセット
		next_.reset();
#pragma endregion

#pragma region ロード中
		scene_->Load();

		// シーンの初期化
		scene_->Initialize();

		// ロード中の描画を終了
		load_->Stop();
#pragma endregion

#pragma region その後の処理
		// フェードスタート
		fade_->InStart();

		// シーンの更新処理
		scene_->Update();
#pragma endregion
	}

	// テクスチャデータのアップロード
	UploadTextureData();
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

bool SceneManager::IsEnd() const {
	if (!scene_) {
		return true;
	}

	return scene_->GetID() == finishID_ &&
		(input_->GetKey()->Pushed(DIK_ESCAPE) /*||
			input_->GetGamepad()->Pushed(Gamepad::Button::START)*/);
}

const Camera& SceneManager::GetCurrentSceneCamera() const
{
	return scene_->GetCamera();
}

BaseScene::ID SceneManager::GetCurrentSceneID() const
{
	return scene_->GetID();
}

BaseScene::ID SceneManager::GetPreSceneID() const
{
	return preSceneID_.value();
}

void SceneManager::Debug()
{
	if (input_->GetKey()->LongPush(DIK_SEMICOLON)) {
		for (auto& i : sceneNum_) {
			if (input_->GetKey()->Pushed(i.second)) {
				SceneChange(i.first);
				return;
			}
		}
	}
#ifdef USE_DEBUG_CODE
	ImGui::Begin("SceneManager");
	if (ImGui::TreeNode("シーン変更")) {
		for (auto& i : sceneName_) {
			if (ImGui::Button(i.second.c_str())) {
				SceneChange(i.first);
				break;
			}
		}
		ImGui::TreePop();
	}

	ImGui::Text((std::string("currentScene : ") + sceneName_[scene_->GetID()]).c_str());
	ImGui::Text((std::string("preScene : ") + sceneName_[preSceneID_.value()]).c_str());
	ImGui::End();
#endif // USE_DEBUG_CODE
}

void SceneManager::UploadTextureData()
{
	auto textureManager = TextureManager::GetInstance();
	// このフレームで画像読み込みが発生していたらTextureをvramに送る
	textureManager->UploadTextureData();
	// dramから解放
	textureManager->ReleaseIntermediateResource();
}

void SceneManager::Finalize() {
	if (load_) {
		load_.reset();
	}


	fade_.reset();
	if (scene_) {
		scene_->Finalize();
	}
	scene_.reset();
	if (next_) {
		next_->Finalize();
	}
	next_.reset();
}

