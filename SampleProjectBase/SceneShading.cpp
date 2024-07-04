#include "SceneShading.h"
#include "Model.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "Input.h"

void SceneShading::Init()
{
	// �V�F�[�_�[�̓ǂݍ��ݏ���
	Shader* shader[] = {
		CreateObj<VertexShader>("VS_Object"),
		CreateObj<VertexShader>("VS_GouraudShading"),
		CreateObj<PixelShader>("PS_TexColor"),
		CreateObj<PixelShader>("PS_PhongShading"),
		CreateObj<PixelShader>("PS_GouraudShading"),
		CreateObj<PixelShader>("PS_PhongSpecular"),
		CreateObj<VertexShader>("VS_WorldPosition"),
	};
	const char* file[] = {
		"Assets/Shader/VS_Object.cso",
		"Assets/Shader/VS_GouraudShading.cso",
		"Assets/Shader/PS_TexColor.cso",
		"Assets/Shader/PS_PhongShading.cso",
		"Assets/Shader/PS_GouraudShading.cso",
		"Assets/Shader/PS_PhongSpecular.cso",
		"Assets/Shader/VS_WorldPosition.cso",
	};

	int shaderNum = _countof(shader);
	for (int i = 0; i < shaderNum; ++i)
	{
		if (FAILED(  shader[i]->Load(file[i])  ))
		{
			MessageBox(NULL, file[i], "Shader Error", MB_OK);
		}
	}
}
void SceneShading::Uninit()
{
}
void SceneShading::Update(float tick)
{
}
void SceneShading::Draw()
{
	// Root�N���X�œǂݍ��݂���Ă���Model���擾
	// GetObj �����Ŏw�肳�ꂽ���O�̃f�[�^��
	//        �e���v���[�g�����Ɏw�肳�ꂽ�^�ɕϊ����Ď擾
	Model* pModel = GetObj<Model>("Model");
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	LightBase* pLight = GetObj<LightBase>("Light");

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
		GetObj<Shader>("VS_Object"),
		GetObj<Shader>("VS_GouraudShading"),
		GetObj<Shader>("PS_TexColor"),
		GetObj<Shader>("PS_PhongShading"),
		GetObj<Shader>("PS_GouraudShading"),
		GetObj<Shader>("PS_PhongSpecular"),
		GetObj<Shader>("VS_WorldPosition"),
	};
	int shaderPair[][2] = {
		{0, 2}, // �ʏ�\��
		{1, 4}, // �O���[�V�F�[�f�B���O
		{0, 3}, // �t�H���V�F�[�f�B���O
		{6, 5}, // �t�H�����ʔ���
	};

	// �`��
	int drawNum = _countof(shaderPair);
	for (int i = 0; i < drawNum; ++i)
	{
		// ���W�̍X�V
		DirectX::XMStoreFloat4x4(&mat[0],
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixTranslation(
					(drawNum - 1) * 0.5f - i, 0.0f, 0.0f
			)));
		mat[0] = mat[0] * pModel->GetScaleBaseMatrix();

		// �萔�o�b�t�@�̍X�V
		shader[shaderPair[i][0]]->WriteBuffer(0, mat);
		shader[shaderPair[i][0]]->WriteBuffer(1, light);
		shader[shaderPair[i][1]]->WriteBuffer(0, light);
		shader[shaderPair[i][1]]->WriteBuffer(1, camera);

		// �`��
		pModel->SetVertexShader(shader[shaderPair[i][0]]);
		pModel->SetPixelShader(shader[shaderPair[i][1]]);
		pModel->Draw();
	}
}