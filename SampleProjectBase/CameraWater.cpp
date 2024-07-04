#include "CameraWater.h"

CameraWater::CameraWater()
	: m_pTarget(nullptr)
	, m_waterHeight(0.0f)
{
}
CameraWater::~CameraWater()
{
}
void CameraWater::Update()
{
	if(!m_pTarget) { return; }

	m_pos = m_pTarget->GetPos();
	// �Q�Ƃ��Ă���J�����̐��ʂ���̍�����
	// ���ʉ��̃J�����̍������������Ȃ�悤�Ɍv�Z
	m_pos.y = m_waterHeight - (m_pos.y - m_waterHeight);

	// ���ʂɉf�����G�͏㉺���]���Ă���̂ŁA
	// �J�����̊G���㉺���]����悤�ɁA�A�b�v�x�N�g���𔽓]������
	m_up = m_pTarget->GetUp();
	m_up.x *= -1.0f;
	m_up.y *= -1.0f;
	m_up.z *= -1.0f;

}
void CameraWater::SetTargetCamera(CameraBase* pCamera)
{
	m_pTarget = pCamera;
}
void CameraWater::SetWaterPlaneHeight(float height)
{
	m_waterHeight = height;
}