#include "SceneToon.h"
#include "Model.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "Input.h"

void SceneToon::Init()
{
	// �V�F�[�_�[�̓ǂݍ��ݏ���
	Shader* shader[] = {
		CreateObj<VertexShader>("VS_Object"),
		CreateObj<PixelShader>("PS_TexColor"),
		CreateObj<PixelShader>("PS_Toon"),
		CreateObj<VertexShader>("VS_Outline"),
		CreateObj<PixelShader>("PS_Outline"),
		CreateObj<VertexShader>("VS_WorldPosition"),
		CreateObj<PixelShader>("PS_Hatching"),
		CreateObj<VertexShader>("VS_AlphaDither"),
		CreateObj<PixelShader>("PS_AlphaDither"),
	};
	const char* file[] = {
		"Assets/Shader/VS_Object.cso",
		"Assets/Shader/PS_TexColor.cso",
		"Assets/Shader/PS_Toon.cso",
		"Assets/Shader/VS_Outline.cso",
		"Assets/Shader/PS_Outline.cso",
		"Assets/Shader/VS_WorldPosition.cso",
		"Assets/Shader/PS_Hatching.cso",
		"Assets/Shader/VS_AlphaDither.cso",
		"Assets/Shader/PS_AlphaDither.cso",
	};
	int shaderNum = _countof(shader);
	for (int i = 0; i < shaderNum; ++i)
	{
		if (FAILED(shader[i]->Load(file[i])))
		{
			MessageBox(NULL, file[i], "Shader Error", MB_OK);
		}
	}

	// �����v�e�N�X�`���̓Ǎ�
	Texture* pRamp =
		CreateObj<Texture>("RampTex");
	pRamp->Create("Assets/Texture/RampTex.bmp");
	// �f�B�U�p�^�[���̓Ǎ�
	Texture* pDither =
		CreateObj<Texture>("DitherTex");
	pDither->Create("Assets/Texture/BayerMatrix.png");


}
void SceneToon::Uninit()
{
}
void SceneToon::Update(float tick)
{
	m_Frame++;
}
void SceneToon::Draw()
{
	// �e��擾
	Model* pModel = GetObj<Model>("Model");
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	LightBase* pLight = GetObj<LightBase>("Light");
	Texture* pRampTex =
		GetObj<Texture>("RampTex");
	Texture* pDitherTex =
		GetObj<Texture>("DitherTex");

	// �萔�o�b�t�@�ɓn���s��̏��
	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();

	// �萔�o�b�t�@�ɓn�����C�g�̏��
	DirectX::XMFLOAT3 lightDir =
		pLight->GetDirection();
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
		GetObj<Shader>("VS_Object"),		// 0
		GetObj<Shader>("PS_TexColor"),		// 1
		GetObj<Shader>("PS_Toon"),			// 2
		GetObj<Shader>("VS_Outline"),		// 3
		GetObj<Shader>("PS_Outline"),		// 4
		GetObj<Shader>("VS_WorldPosition"),	// 5
		GetObj<Shader>("PS_Hatching"),		// 6
		GetObj<Shader>("VS_AlphaDither"),	// 7
		GetObj<Shader>("PS_AlphaDither"),	// 8
	};

	// �`��
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f)
	));
	// �X�P�[������
	mat[0] = mat[0] * pModel->GetScaleBaseMatrix();
	shader[0]->WriteBuffer(0, mat);
	shader[0]->WriteBuffer(1, light);
	shader[2]->WriteBuffer(0, light);
	shader[2]->WriteBuffer(1, camera);
	shader[2]->SetTexture(1, pRampTex);
	pModel->SetVertexShader(shader[0]);
	pModel->SetPixelShader(shader[2]);
	pModel->Draw();

	//�֊s���̕\��
	SetCullingMode(D3D11_CULL_BACK);
	shader[3]->WriteBuffer(0, mat);
	pModel->SetVertexShader(shader[3]);
	pModel->SetPixelShader(shader[4]);
	pModel->Draw();
	SetCullingMode(D3D11_CULL_NONE);

	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(3.f, 0.f, 0.f)));

	shader[5]->WriteBuffer(0, mat);
	shader[6]->WriteBuffer(0, light);
	pModel->SetVertexShader(shader[5]);
	pModel->SetPixelShader(shader[6]);
	pModel->Draw();

	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(2.f, 0.f, 0.f)));

	mat[0] = mat[0] * pModel->GetScaleBaseMatrix();

	shader[7]->WriteBuffer(0, mat);
	shader[8]->WriteBuffer(0,camera);
	shader[8]->SetTexture(1, pDitherTex);

	pModel->SetVertexShader(shader[7]);
	pModel->SetPixelShader(shader[8]);
	pModel->Draw();

}