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
	// ŽQÆ‚µ‚Ä‚¢‚éƒJƒƒ‰‚Ì…–Ê‚©‚ç‚Ì‚‚³‚Æ
	// …–Ê‰º‚ÌƒJƒƒ‰‚Ì‚‚³‚ª“™‚µ‚­‚È‚é‚æ‚¤‚ÉŒvŽZ
	m_pos.y = m_waterHeight - (m_pos.y - m_waterHeight);

	// …–Ê‚É‰f‚Á‚½ŠG‚Íã‰º”½“]‚µ‚Ä‚¢‚é‚Ì‚ÅA
	// ƒJƒƒ‰‚ÌŠG‚ªã‰º”½“]‚·‚é‚æ‚¤‚ÉAƒAƒbƒvƒxƒNƒgƒ‹‚ð”½“]‚³‚¹‚é
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