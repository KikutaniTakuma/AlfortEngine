/// =========================
/// ==  Modelクラスの宣言  ==
/// =========================

#pragma once
#include "../BaseDrawer.h"

/// <summary>
/// 3Dモデルの描画
/// </summary>
class Model : public BaseDrawer {
public:
	static constexpr uint32_t kMaxDrawCount = 512;

public:
	struct Data {
		Mat4x4 worldMatrix = Mat4x4::kIdentity;
		Mat4x4 camera = Mat4x4::kIdentity;
		uint32_t color = 0xffffffff;
		BlendType blend = BlendType::kNone;
		bool isLighting = true;
	};
	struct Instance {
		QuaternionTransform transform;
		uint32_t color = 0xffffffff;
		bool isLighting = true;
	};

public:
	Model() = default;
	Model(const std::string& fileName);
	Model(const Model&) = default;
	Model(Model&& right) noexcept = default;
	virtual ~Model() = default;

	Model& operator=(const Model& right) = default;
	Model& operator=(Model&& right) noexcept = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	virtual void Load(const std::string& fileName);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="blend">ブレンドタイプ</param>
	/// <param name="isLighting">ライティングするか(ブレンドタイプがkNoneだったらDeferredRenderingの設定依存)</param>
	virtual void Draw(
		const Mat4x4& worldMatrix,
		const Mat4x4& camera,
		uint32_t color,
		BlendType blend,
		bool isLighting = true
	);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="data">データ</param>
	void Draw(const Data& data);

public:
	/// <summary>
	/// ノード
	/// </summary>
	/// <returns></returns>
	const Node& GetNode() const;
	
	/// <summary>
	/// モデルのデータ
	/// </summary>
	/// <returns></returns>
	const ModelData& GetModelData() const;
};

/// <summary>
/// モデルインスタンス
/// </summary>
class ModelInstance {
public:
	ModelInstance() = default;
	~ModelInstance() = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);
	
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="cameraMat">カメラ行列</param>
	void Draw(const Mat4x4& cameraMat);

	/// <summary>
	/// 親のモデルを設定
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(ModelInstance* parent);

	const Mat4x4& GetWorldMatrix() const;

public:
	Vector3 scale = Vector3::kIdentity;
	Vector3 rotate;
	Vector3 pos;

	uint32_t color = 0xffffffff;
	BlendType blend = BlendType::kNone;
	bool isLighting = true;

private:
	Lamb::SafePtr<Model> model_;
	Lamb::SafePtr<ModelInstance> parent_;
	Mat4x4 worldMat_ = Mat4x4::kIdentity;
};