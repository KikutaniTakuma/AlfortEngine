#pragma once
#include "../Object.h"
#include "Transform/Transform.h"

#include <unordered_set>
#ifdef USE_DEBUG_CODE
#include "ImGuizmo.h"
#endif // USE_DEBUG_CODE


class TransformComp : public IComp {
public:
	TransformComp(Object* const object);

	~TransformComp();

	void Init() override;

	void UpdateMatrix();

	void UpdateChildrenMatrix();
	void UpdateParentMatrix();

	void SetParent(Lamb::SafePtr<TransformComp> parent);

	const Mat4x4& GetWorldMatrix() const {
		return worldMatrix_;
	}

	Mat4x4 GetLocalMatrix() const {
		return Mat4x4::MakeAffin(scale, rotate, translate);
	}

	bool IsRootTransForm() const {
		return parent_.empty();
	}

	bool HaveChildren() const {
		return not children_.empty();
	}

	bool HaveParent() const {
		return parent_.have();
	}

	void Debug(const std::string& guiName) override;

#ifdef USE_DEBUG_CODE
	void SetGuizmoID(uint32_t id);

	void Guizmo(class CameraComp* cameraComp);
#endif // USE_DEBUG_CODE

	void Save(nlohmann::json& json) override;
	void Load(nlohmann::json& json) override;

public:
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
#ifdef USE_DEBUG_CODE
	Vector3 eulerRotate;
#endif // USE_DEBUG_CODE

private:
#ifdef USE_DEBUG_CODE
	uint32_t guizmoID_ = 0;
	uint32_t guimoType_ = 0;

	bool isGuizmo_ = false;

	static const std::array<std::pair<std::string, ImGuizmo::OPERATION>, 5> kGuizmoMode_;

#endif // USE_DEBUG_CODE


	Mat4x4 worldMatrix_;
	Lamb::SafePtr<TransformComp> parent_;
	std::unordered_set<Lamb::SafePtr<TransformComp>> children_;
};