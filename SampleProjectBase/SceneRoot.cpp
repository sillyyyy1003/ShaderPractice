#include "SceneRoot.h"
#include <stdio.h>
#include "CameraDCC.h"
#include "MoveLight.h"
#include "Model.h"
#include "Input.h"
#include "Geometory.h"

#include "SceneBlank.h"
#include "SceneAnimation.h"

#include "DebugLog.h"
#include "SceneBumpmap.h"
#include "SceneLight.h"
#include "ScenePBR.h"
#include "SceneShading.h"
#include "SceneToon.h"

#define STR(var) #var

//--- �萔��`
enum SceneKind
{
	SCENE_TOON,
	//SCENE_PBR,
	//SCENE_BUMPMAP,
	//SCENE_LIGHTING,
	//SCENE_SHADING,
	//SCENE_ANIMATION,
	SCENE_BLANK,
	SCENE_MAX
};

/// <summary>
/// �V�[���؂�ւ�
/// �f�o�b�O�o�͂̎������_�T����C++����enum�̖��O��
/// ������Ƃ��Ď擾����̂���ԂȂ̂ō���͂��̂܂�
/// </summary>
void SceneRoot::ChangeScene()
{
	switch (m_index)
	{
	default:
	case SCENE_TOON:AddSubScene<SceneToon>();
		DebugLog::Log("SceneName = SCENE_PBR");
		break;
	case SCENE_BLANK:AddSubScene<SceneBlank>();
		DebugLog::Log("SceneName = SCENE_BLANK");
		break;
	/*case SCENE_ANIMATION:AddSubScene<SceneAnimation>();
		DebugLog::Log("SceneName = SCENE_ANIMATION");*/
		break;
	}
}


//--- �\����
// @brief �V�[�����ۑ�
struct ViewSetting
{
	DirectX::XMFLOAT3 camPos;
	DirectX::XMFLOAT3 camLook;
	DirectX::XMFLOAT3 camUp;
	float lightRadXZ;
	float lightRadY;
	float lightH;
	float lightSV;
	int index;
};
const char* SettingFileName = "Assets/setting.dat";

void SceneRoot::Init()
{
	// �ۑ��f�[�^�̓ǂݍ���
	ViewSetting setting =
	{
		DirectX::XMFLOAT3(0.0f, 1.0f, -5.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		0.0f, DirectX::XM_PIDIV4,
		0.0f, 1.0f,
		SCENE_BLANK
	};
	FILE* fp;
	fopen_s(&fp, SettingFileName, "rb");
	if (fp)
	{
		fread(&setting, sizeof(ViewSetting), 1, fp);
		fclose(fp);
	}

	// �J�����̍쐬
	CameraBase* pCamera = CreateObj<CameraDCC>("Camera");
	pCamera->SetPos(setting.camPos);
	pCamera->SetLook(setting.camLook);
	pCamera->SetUp(setting.camUp);

	// ���C�g�̍쐬
	MoveLight* pLight = CreateObj<MoveLight>("Light");
	pLight->SetRot(setting.lightRadXZ, setting.lightRadY);
	pLight->SetHSV(setting.lightH, setting.lightSV);
	pLight->UpdateParam();

	// ���f���̓ǂݍ���
	CreateObj<Model>("Model");
	GetObj<Model>("Model")->Load("Assets/Model/spot/spot.fbx", 1.0f, true);
	
	// �A�j���[�V�����p���f���ǂݍ���
	Model* pAnimModel = CreateObj<Model>("Akai");
	if (pAnimModel->Load("Assets/Model/Akai/Akai.fbx", 0.01f, true))
	{
		pAnimModel->LoadAnimation("Assets/Model/Akai/Akai_Run.fbx", "Run", true);
		pAnimModel->LoadAnimation("Assets/Model/Akai/Akai_Idle.fbx", "Idle", true);
		pAnimModel->LoadAnimation("Assets/Model/Akai/Akai_Walk.fbx", "Walk", true);
	}
	Model* pPlane = CreateObj<Model>("ModelPlane");
	pPlane->Load("Assets/Model/plane/plane.fbx");

	Model* pField = CreateObj<Model>("FieldModel");
	pField->Load("Assets/Model/field/field.fbx", 1.0f, false, true);

	// �V�[���̍쐬
	m_index = setting.index;
	ChangeScene();
}

void SceneRoot::Uninit()
{
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	MoveLight* pLight = GetObj<MoveLight>("Light");
	ViewSetting setting =
	{
		pCamera->GetPos(),
		pCamera->GetLook(),
		pCamera->GetUp(),
		pLight->GetRotXZ(), pLight->GetRotY(),
		pLight->GetH(), pLight->GetSV(),
		m_index
	};
	FILE* fp;
	fopen_s(&fp, SettingFileName, "wb");
	if (fp)
	{
		fwrite(&setting, sizeof(ViewSetting), 1, fp);
		fclose(fp);
	}
}

void SceneRoot::Update(float tick)
{
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	LightBase* pLight = GetObj<LightBase>("Light");
	if (!IsKeyPress(VK_SHIFT))
	{
		pCamera->Update();
		pLight->Update();
		return;
	}

	// SHIFT�L�[��������Ă�΁A�V�[���̐؂�ւ�����
	int idx = m_index;
	if (IsKeyTrigger(VK_LEFT)) --idx;
	if (IsKeyTrigger(VK_RIGHT)) ++idx;
	if (idx < 0) idx = SCENE_MAX - 1;
	if (idx >= SCENE_MAX) idx = 0;

	if (idx != m_index)
	{
		m_index = idx;
		RemoveSubScene();
		ChangeScene();
	}

	// �J����������
	if (IsKeyTrigger('R'))
	{
		pCamera->SetPos(DirectX::XMFLOAT3(0.0f, 1.0f, -5.0f));
		pCamera->SetLook(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
		pCamera->SetUp(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	}
}
void SceneRoot::Draw()
{
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	LightBase* pLight = GetObj<LightBase>("Light");

	DirectX::XMFLOAT4X4 fmat;
	DirectX::XMStoreFloat4x4(&fmat, DirectX::XMMatrixIdentity());
	Geometory::SetWorld(fmat);
	Geometory::SetView(pCamera->GetView());
	Geometory::SetProjection(pCamera->GetProj());

	// �Ԋ|���`��
	const int GridSize = 10;
	Geometory::SetColor(DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f));
	for (int i = 1; i <= GridSize; ++i)
	{
		float g = (float)i;
		Geometory::AddLine(DirectX::XMFLOAT3(g, 0.0f, -GridSize), DirectX::XMFLOAT3(g, 0.0f, GridSize));
		Geometory::AddLine(DirectX::XMFLOAT3(-g, 0.0f, -GridSize), DirectX::XMFLOAT3(-g, 0.0f, GridSize));
		Geometory::AddLine(DirectX::XMFLOAT3(-GridSize, 0.0f, g), DirectX::XMFLOAT3(GridSize, 0.0f, g));
		Geometory::AddLine(DirectX::XMFLOAT3(-GridSize, 0.0f, -g), DirectX::XMFLOAT3(GridSize, 0.0f, -g));
	}
	// ���`��
	Geometory::SetColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	Geometory::AddLine(DirectX::XMFLOAT3(-GridSize, 0.0f, 0.0f), DirectX::XMFLOAT3(GridSize, 0.0f, 0.0f));
	Geometory::SetColor(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	Geometory::AddLine(DirectX::XMFLOAT3(0.0f, -GridSize, 0.0f), DirectX::XMFLOAT3(0.0f, GridSize, 0.0f));
	Geometory::SetColor(DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	Geometory::AddLine(DirectX::XMFLOAT3(0.0f, 0.0f, -GridSize), DirectX::XMFLOAT3(0.0f, 0.0f, GridSize));

	Geometory::DrawLines();

	// �I�u�W�F�N�g�`��
	pCamera->Draw();
	pLight->Draw();
}
