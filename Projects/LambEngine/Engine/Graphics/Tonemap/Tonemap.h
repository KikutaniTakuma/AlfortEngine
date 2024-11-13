#pragma once
#include "Math/Vector3.h"
#include "Math/Vector2.h"

#include "Math/MathCommon.h"

struct TonemapParams
{
	float32_t3 mToe;
	float32_t  pad0 = 0.0f;
	float32_t2 mMid;
	float32_t2 pad1;
	float32_t3 mShoulder;
	float32_t  pad2 = 0.0f;
	float32_t2 mBx;
	float32_t2 mBy;	// only used for InvTonemap
};

float32_t3 Tonemap(const TonemapParams& tc, const float32_t3& x);

TonemapParams PrepareTonemapParams(const float32_t2 p1, const float32_t2 p2, const float32_t2 p3);

float32_t TonemapToe(const float32_t x, const float32_t2 p1, const float32_t2 p2);
float32_t TonemapSholder(const float32_t x, const float32_t2 p1, const float32_t2 p2, const float32_t2 p3);
