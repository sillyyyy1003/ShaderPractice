
#include <DirectXMath.h>

#include "Emitter.h"
#include "math.h"
#include "Geometory.h"
#include "DebugLog.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Emitter::Emitter(DirectX::XMFLOAT3 pos)
{
	m_pos = pos;
	DirectX::XMStoreFloat4x4(&m_mat, DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f)));

	Init();
	mt.seed(rd());
}

void Emitter::Init()
{
}

void Emitter::Update(float tick)
{
	m_time += tick;
	m_generatTimeCount += tick;
	if (m_generatTimeCount >= m_generatTime)
	{
		int num = Rand_f(m_generatNum_min, m_generatNum_max);
		//DebugLog::Log("生成数 = {}",num);
		for (int j = 0; j < num; j++)
		{
			// パーティクル生成最大数チェック
			if (m_particles.size() < m_particleMax)
			{
				// パーティクル生成
				CreateParticle();
			}
			else
			{
				// パーティクル再初期化で使いまわす
				if (!RecycleParticle())
				{
					DebugLog::Log("プール不足");
				}
			}
		}
		m_generatTimeCount = 0.0f;
	}

	for (int i=0; i < m_particles.size(); i++)
	{
		m_particles[i].Update(tick);
	}
	
}

void Emitter::Drow()
{
	for (int i = 0; i < m_particles.size(); i++)
	{
		m_particles[i].Drow();
	}
}

void Emitter::CreateParticle()
{
	// パーティクル初期化
	Particle p = Particle(m_pos,
		Rand_f(lifeTime_min, lifeTime_max),
		gravity,
		color_start,
		color_end,
		colorChangeStartRate,
		DirectX::XMFLOAT3(Rand_f(vecx_min,vecx_max), Rand_f(vecy_min, vecy_max), Rand_f(vecz_min, vecz_max)),
		mass,
		refRate,
		Rand_f(deg_min,deg_max));
	m_particles.push_back(p);
}

bool Emitter::RecycleParticle()
{
	bool b = false;
	// 停止済みパーティクル走査
	for (int i = 0;i< m_particles.size();i++)
	{
		if (!m_particles[i].IsStop()) continue;

		// パーティクル再初期化
		m_particles[i].InitParam(m_pos,
			Rand_f(lifeTime_min, lifeTime_max),
			gravity,
			color_start,
			color_end,
			colorChangeStartRate,
			DirectX::XMFLOAT3(Rand_f(vecx_min, vecx_max), Rand_f(vecy_min, vecy_max), Rand_f(vecz_min, vecz_max)),
			mass,
			refRate,
			Rand_f(deg_min, deg_max));
		b = true;
		break;
	}
	return b;
}

void Emitter::SetVector(float minx, float maxx, float miny, float maxy, float minz, float maxz)
{
	vecx_min = minx;
	vecx_max = maxx;
	vecy_min = miny;
	vecy_max = maxy;
	vecz_min = minz;
	vecz_max = maxz;
}

void Emitter::SetGeneratNum(float min, float max)
{
	m_generatNum_min = min;
	m_generatNum_max = max;
}

void Emitter::SetGeneratTime(float time)
{
	m_generatTime = time;
}

float Emitter::Rand_f(float min, float max)
{
	std::uniform_real_distribution<> urd(min, max);
	return urd(mt);
}

