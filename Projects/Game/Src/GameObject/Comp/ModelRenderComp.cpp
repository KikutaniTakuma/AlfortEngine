#include "ModelRenderComp.h"
#include "Drawer/DrawerManager.h"
#include "ModelRenderDataComp.h"

#include "TransformComp.h"
#include "CameraComp.h"

#ifdef USE_DEBUG_CODE
#include "Utils/FileUtils.h"
#endif // USE_DEBUG_CODE

void ModelRenderComp::Init() {
	transformComp_ = object_.AddComp<TransformComp>();
	renderDataComp_ = object_.AddComp<ModelRenderDataComp>();

#ifdef USE_DEBUG_CODE
    filePaths_ = Lamb::GetFilePathFormDir("./", ".obj");
    auto temp = Lamb::GetFilePathFormDir("./", ".gltf");
    filePaths_.insert(filePaths_.end(), temp.begin(), temp.end());
    temp = Lamb::GetFilePathFormDir("./", ".glb");
    filePaths_.insert(filePaths_.end(), temp.begin(), temp.end());
#endif // USE_DEBUG_CODE
}

void ModelRenderComp::Draw(CameraComp* cameraComp) {
    if (model_.have() and renderDataComp_->isDraw) {
        model_->Draw(
            transformComp_->GetWorldMatrix(),
            cameraComp->GetCameraMatrix(),
            renderDataComp_->color.GetColorRGBA(),
            renderDataComp_->type,
            renderDataComp_->isLighting
        );
    }
}

void ModelRenderComp::Load() {
	Lamb::SafePtr drawerManager = DrawerManager::GetInstance();
	drawerManager->LoadModel(renderDataComp_->fileName);
	model_ = drawerManager->GetModel(renderDataComp_->fileName);
}

void ModelRenderComp::Debug([[maybe_unused]]const std::string& guiName) {
#ifdef USE_DEBUG_CODE
    if (ImGui::TreeNode(guiName.c_str())) {
        ImGui::Checkbox("描画有効", &(renderDataComp_->isDraw));

        // コンボボックスを使ってenumの値を選択する
        if (ImGui::BeginCombo("BlendType", kBlendTypeStrs[static_cast<uint32_t>(renderDataComp_->type)].c_str()))
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(BlendType::kNum); ++i)
            {
                bool isSelected = (renderDataComp_->type == static_cast<BlendType>(i));
                if (ImGui::Selectable(kBlendTypeStrs[i].c_str(), isSelected))
                {
                    renderDataComp_->type = static_cast<BlendType>(i);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::ColorEdit4("color", renderDataComp_->color.data());
        if (renderDataComp_->type != BlendType::kNone) {
            ImGui::Checkbox("Lighting", &renderDataComp_->isLighting);
        }

        ImGui::Text("モデル %s", renderDataComp_->fileName.c_str());

        if (ImGui::Button("ファイルパス再読み込み")) {
            size_t size = filePaths_.size();
            filePaths_.clear();
            filePaths_.reserve(size);
            filePaths_ = Lamb::GetFilePathFormDir("./", ".obj");
            auto temp = Lamb::GetFilePathFormDir("./", ".gltf");
            filePaths_.insert(filePaths_.end(), temp.begin(), temp.end());
            temp = Lamb::GetFilePathFormDir("./", ".glb");
            filePaths_.insert(filePaths_.end(), temp.begin(), temp.end());
        }

        if (ImGui::TreeNode("モデル読み込み")) {
            for (auto itr = filePaths_.begin(); itr != filePaths_.end(); itr++) {
                if (ImGui::Button(itr->string().c_str())) {
                    renderDataComp_->fileName = itr->string();
                    Load();
                }
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

#endif // USE_DEBUG_CODE
}

void ModelRenderComp::Save(nlohmann::json& json)
{
    SaveCompName(json);
}

void ModelRenderComp::Load([[maybe_unused]]nlohmann::json& json)
{
}
