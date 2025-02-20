#pragma once
#include "Drawer/Particle/Particle.h"
#include "./Camera/Camera.h"

/// <summary>
/// パーティクルエディタ
/// </summary>
class ParticleEditor {
private:
	ParticleEditor() = default;
	ParticleEditor(const ParticleEditor&) = delete;
	ParticleEditor(ParticleEditor&&) = delete;
	~ParticleEditor() = default;

	ParticleEditor& operator=(const ParticleEditor&) = delete;
	ParticleEditor& operator=(ParticleEditor&&) = delete;

public:
	static void Initialize();

	static void Finalize();

	static ParticleEditor* GetInstance();

private:
	static ParticleEditor* pInstance_;

public:
	void Editor();

	void Draw(const Camera& camera);

private:
	std::string inputLoadString_;
	std::string currentLoadString_;
	Particle particle_;

	bool isOpen_ = false;

	Camera camera_;

	bool is3DCamera_ = false;
	bool isLocalCamera_ = true;
};