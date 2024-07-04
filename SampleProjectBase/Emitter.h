#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <vector>
#include <random>

#include "Particle.h"
#include "DirectXTex/SimpleMath.h"

#pragma once
// ���N���X
class Emitter
{
public:
	Emitter(DirectX::XMFLOAT3 pos);
	void Init();
	void Update(float tick);
	void Drow();
	void CreateParticle();	//�p�[�e�B�N������
	bool RecycleParticle();	//�p�[�e�B�N���ė��p
	void SetVector(float minx, float maxx, float miny, float maxy, float minz, float maxz); // �p�[�e�B�N�����x�x�N�g���ݒ�
	void SetGeneratNum(float min, float max);
	void SetGeneratTime(float time);
private:
	const int m_particleMax = 1000;		// �ő吶����
	DirectX::XMFLOAT4X4 m_mat;			// ���[���h�s��

	float m_time = 0.0f;				// �o�ߎ���
	std::vector<Particle> m_particles;	// �p�[�e�B�N���Ǘ��z��
	int m_generatNum_min = 1;			// ������
	int m_generatNum_max = 1;
	float m_generatTime = 0.1f;		// �����Ԋu
	float m_generatTimeCount = 0.0f; 

	// �����֌W
	std::random_device rd;
	std::mt19937 mt;
	float Rand_f(float min,float max);	// min~max�̊Ԃ̒l��Ԃ����������֐�
public:
	DirectX::XMFLOAT3 m_pos;		// �������W
	// �����p�p�����[�^�[ ------------------------------------------------------
	float lifeTime_min = 1.0f;		// �������ԍŒZ
	float lifeTime_max = 1.0f;		// �������ԍŒ�
	float gravity = 9.8f;			// �d��
	DirectX::XMFLOAT4 color_start = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// �J�n���̐F
	DirectX::XMFLOAT4 color_end = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);;	// �I�����̐F
	float colorChangeStartRate = 0.8f;		// �F�ω��J�n 0.0~1.0

	float vecx_min = 0.0f;		// �ˏo�x�N�g��
	float vecx_max = 0.0f;
	float vecy_min = 0.0f;
	float vecy_max = 0.0f;
	float vecz_min = 0.0f;
	float vecz_max = 0.0f;

	float deg_min = 0.0f;		//�ˏo�p�x
	float deg_max = 0.0f;

	float mass = 1.0f;			// ����
	float refRate = 1.0f;		// �����W��
	// --------------------------------------------------------------------------
};

#endif // __EMITTER_H__