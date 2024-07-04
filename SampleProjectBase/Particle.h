#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "DirectXTex/SimpleMath.h"
#include <vector>

#pragma once
// ���N���X
class Particle
{
public:
	enum MoveMode {
		Stop,
		Oblique
	};

public:
	Particle();
	Particle(DirectX::XMFLOAT3 pos,
		float lifeTime,
		float gravity,
		DirectX::XMFLOAT4 color_s,
		DirectX::XMFLOAT4 color_e,
		float color_r,
		DirectX::XMFLOAT3 vec,
		float mass,
		float ref,
		float deg);

	void InitParam(DirectX::XMFLOAT3 pos,
		float lifeTime,
		float gravity,
		DirectX::XMFLOAT4 color_s,
		DirectX::XMFLOAT4 color_e,
		float color_r,
		DirectX::XMFLOAT3 vec,
		float mass,
		float ref,
		float deg);

	void Update(float tick);
	void ColorUpdate();
	void Drow();

	void Reset();
	bool IsStop();

	bool HitSphere(Particle b);
	float GetRadius();
	DirectX::XMFLOAT3 GetPosition();
	float GetMass();
	DirectX::XMFLOAT3 GetSpeed3d();

	bool FindPair(int num);
public:
	std::vector<int> hitPair;		// �Փˏ����d������̂��ߏ����y�A�̓Y�������i�[

private:
	DirectX::XMFLOAT4X4 m_mat;		// ���[���h�s��
	DirectX::XMFLOAT4 m_color;		// ���̐F
	DirectX::XMFLOAT3 m_scale;		// ���̃T�C�Y
	DirectX::XMFLOAT3 m_basePos;	// �������W
	DirectX::XMFLOAT3 m_pos;		// �������W����̍���
	const float m_baseRadius = 0.5f;// 1�{�̎��̋��̔��a���Œ�l
	float m_gravity = 9.8f;			// �d��
	float m_time = 0.0f;			// ����
	DirectX::XMFLOAT3 m_speed3d = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);			// ����3D
	float m_acceleration = 0.0f;	// �����x
	float m_mass = 1.0f;			// ����
	float m_lifeTime = 5.0f;		// ��������
	float m_refRate = 1.0f;			// �����W��
	MoveMode m_mode = MoveMode::Oblique;	// �p�[�e�B�N����ԑJ��
	DirectX::XMFLOAT4 m_color_start = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// �J�n�F
	DirectX::XMFLOAT4 m_color_end = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);		// �I���F
	float m_deg = 60.0f;					// ���ˊp�x
	float m_colorChangeStartRate = 0.8f;
};

#endif // __PARTICLE_H__