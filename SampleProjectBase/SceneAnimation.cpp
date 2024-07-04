#include "SceneAnimation.h"
#include "Model.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "Input.h"

void SceneAnimation::Init()
{
	// �V�F�[�_�[�̓ǂݍ��ݏ���
	Shader* shader[] = {
		CreateObj<VertexShader>("VS_SkinMeshAnimation"),
		CreateObj<PixelShader>("PS_TexColor"),
	};
	const char* file[] = {
		"Assets/Shader/VS_SkinMeshAnimation.cso",
		"Assets/Shader/PS_TexColor.cso",
	};
	int shaderNum = _countof(shader);
	for (int i = 0; i < shaderNum; ++i)
	{
		if (FAILED(shader[i]->Load(file[i])))
		{
			MessageBox(NULL, file[i], "Shader Error", MB_OK);
		}
	}
	pCamera = GetObj<CameraBase>("Camera");
}
void SceneAnimation::Uninit()
{
}
void SceneAnimation::Update(float tick)
{
	m_Frame++;

	// �J������������Ɉړ��������s��
	if (!IsKeyPress(VK_RBUTTON))
	{
		if (IsKeyPress('W'))
		{
			DirectX::XMFLOAT3 lookVec = pCamera->GetLook() - pCamera->GetPos();
			DirectX::SimpleMath::Vector3 simpleVec = { lookVec.x,lookVec.y,lookVec.z };
			simpleVec.Normalize();
			simpleVec.y = 0.0f;
			modelPos = modelPos + simpleVec * 0.1f;
		}
		if (IsKeyPress('A'))
		{
			DirectX::XMFLOAT3 lookVec = pCamera->GetLook() - pCamera->GetPos();
			DirectX::SimpleMath::Vector3 simpleVec = { lookVec.x,lookVec.y,lookVec.z };
			simpleVec.Normalize();
			DirectX::SimpleMath::Vector3 sideVec = simpleVec.Cross(pCamera->GetUp());
			sideVec.y = 0.0f;
			modelPos = modelPos + sideVec * 0.1f;
		}
		if (IsKeyPress('S'))
		{
			DirectX::XMFLOAT3 lookVec = pCamera->GetLook() - pCamera->GetPos();
			DirectX::SimpleMath::Vector3 simpleVec = { lookVec.x,lookVec.y,lookVec.z };
			simpleVec.Normalize();
			simpleVec *= -1;
			simpleVec.y = 0.0f;
			modelPos = modelPos + simpleVec * 0.1f;
		}
		if (IsKeyPress('D'))
		{
			DirectX::XMFLOAT3 lookVec = pCamera->GetLook() - pCamera->GetPos();
			DirectX::SimpleMath::Vector3 simpleVec = { lookVec.x,lookVec.y,lookVec.z };
			simpleVec.Normalize();
			DirectX::SimpleMath::Vector3 sideVec = simpleVec.Cross(pCamera->GetUp());
			sideVec *= -1;
			sideVec.y = 0.0f;
			modelPos = modelPos + sideVec * 0.1f;
		}
	}
}
void SceneAnimation::Draw()
{
	// �e��擾
	Model* pAnimModel		= GetObj<Model>("Akai");
	CameraBase* pCamera		= GetObj<CameraBase>("Camera");
	LightBase* pLight		= GetObj<LightBase>("Light");
	Texture* pRampTex		= GetObj<Texture>("RampTex");
	Texture* pDitherTex		= GetObj<Texture>("DitherTex");

	// �萔�o�b�t�@�ɓn���s��̏��
	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();

	// �萔�o�b�t�@�ɓn�����C�g�̏��
	DirectX::XMFLOAT3 lightDir = pLight->GetDirection();
	DirectX::XMFLOAT4 light[] = {
		pLight->GetDiffuse(),
		pLight->GetAmbient(),
		{lightDir.x, lightDir.y, lightDir.z, 0.0f}
	};

	// �萔�o�b�t�@�ɓn���J�����̏��
	DirectX::XMFLOAT3 camPos = pCamera->GetPos();
	DirectX::XMFLOAT4 camera[] = {
		{camPos.x, camPos.y, camPos.z, 0.0f}
	};

	// �V�F�[�_�[�̎擾
	Shader* shader[] = {
		GetObj<Shader>("VS_SkinMeshAnimation"),
		GetObj<Shader>("PS_TexColor"),
	};

	// ���W�ړ�
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z)
	));
	// �X�P�[������
	mat[0] = mat[0] * pAnimModel->GetScaleBaseMatrix();

	shader[0]->WriteBuffer(0, mat);
	shader[1]->WriteBuffer(0, camera);
	shader[1]->SetTexture(1, pDitherTex);
	pAnimModel->UpdateAnimation("Run", m_Frame);
	pAnimModel->SetVertexShader(shader[0]);
	pAnimModel->SetPixelShader(shader[1]);
	pAnimModel->Draw();
}