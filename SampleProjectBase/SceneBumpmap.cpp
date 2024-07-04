#include "SceneBumpmap.h"
#include "Model.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "Input.h"
#include "Geometory.h"

// �o���v�}�b�v�p�̒��_�f�[�^
struct TangentVtx
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	// �V�������f���ɒǉ����钸�_�f�[�^
	DirectX::XMFLOAT3 tangent; // ���[���h��ԏ�Ńe�N�X�`����\��t���鉡����
};

void SceneBumpmap::Init()
{
	// �V�F�[�_�[�̓ǂݍ��ݏ���
	Shader* shader[] = {
		CreateObj<VertexShader>("VS_Object"),
		CreateObj<PixelShader>("PS_TexColor"),
		CreateObj<VertexShader>("VS_Bumpmap"),
		CreateObj<PixelShader>("PS_Bumpmap"),
		CreateObj<PixelShader>("PS_ParallaxMapping"),
		CreateObj<PixelShader>("PS_ParallaxOcclusion"),
	};
	const char* file[] = {
		"Assets/Shader/VS_Object.cso",
		"Assets/Shader/PS_TexColor.cso",
		"Assets/Shader/VS_Bumpmap.cso",
		"Assets/Shader/PS_Bumpmap.cso",
		"Assets/Shader/PS_ParallaxMapping.cso",
		"Assets/Shader/PS_ParallaxOcclusion.cso"
	};

	int shaderNum = _countof(shader);
	for (int i = 0; i < shaderNum; ++i)
	{
		if (FAILED(shader[i]->Load(file[i])))
		{
			MessageBox(NULL, file[i], "Shader Error", MB_OK);
		}
	}


	// �^���W�F���g�x�N�g�������f���ɕt�^���邽�߂̌v�Z
	auto calcTangent = [](Model::RemakeInfo& data)
	{
		// ��x�����̃f�[�^��V�����f�[�^�փR�s�[����
		TangentVtx* destVtx = reinterpret_cast<TangentVtx*>(data.dest);
		const Model::Vertex* sourceVtx =
			reinterpret_cast<const Model::Vertex*>(data.source);
		for (UINT i = 0; i < data.vtxNum; ++i)
		{
			destVtx[i].pos = sourceVtx[i].pos;
			destVtx[i].normal = sourceVtx[i].normal;
			destVtx[i].uv = sourceVtx[i].uv;
			destVtx[i].tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		}

		// �ڃx�N�g���̌v�Z
		using TanVecAry = std::vector<DirectX::XMFLOAT3>;
		std::vector<TanVecAry> tangentVtx;
		tangentVtx.resize(data.vtxNum);
		const UINT* idx = reinterpret_cast<const UINT*>(data.idx);
		for (UINT i = 0; i < data.idxNum; i += 3) // 1��(�O���_,�O�p�`)������
		{
			// �O�p�`���\�����钸�_���̎擾
			UINT idx0 = idx[i + 0];
			UINT idx1 = idx[i + 1];
			UINT idx2 = idx[i + 2];
			DirectX::XMFLOAT3 p[3] = {
				destVtx[idx0].pos, destVtx[idx1].pos, destVtx[idx2].pos,
			};
			DirectX::XMFLOAT2 uv[3] = {
				destVtx[idx0].uv, destVtx[idx1].uv, destVtx[idx2].uv
			};
			// �O�p�`���\������Q�ӂ̍��W��UV�̕ω���\���x�N�g�����v�Z
			DirectX::XMFLOAT3 V1(p[1].x - p[0].x, p[1].y - p[0].y, p[1].z - p[0].z);
			DirectX::XMFLOAT3 V2(p[2].x - p[0].x, p[2].y - p[0].y, p[2].z - p[0].z);
			DirectX::XMFLOAT2 ST1(uv[1].x - uv[0].x, uv[1].y - uv[0].y);
			DirectX::XMFLOAT2 ST2(uv[2].x - uv[0].x, uv[2].y - uv[0].y);

			// �ӂ̏�񂩂�ڃx�N�g�����v�Z
			float factor = ST1.x * ST2.y - ST2.x * ST1.y;
			DirectX::XMFLOAT3 T(
				(ST2.y * V1.x - ST1.y * V2.x) / factor,
				(ST2.y * V1.y - ST1.y * V2.y) / factor,
				(ST2.y * V1.z - ST1.y * V2.z) / factor
			);
			// �ڃx�N�g���̐��K��
			float length = sqrtf(T.x * T.x + T.y * T.y + T.z * T.z);
			T.x /= length;
			T.y /= length;
			T.z /= length;
			// ���_�ɐڃx�N�g���̏���ǉ�
			tangentVtx[idx0].push_back(T);
			tangentVtx[idx1].push_back(T);
			tangentVtx[idx2].push_back(T);
		}

		// ���_�Ɋ��蓖�Ă�ꂽ�ڃx�N�g���̕��ς����߂�
		for (int i = 0; i < data.vtxNum; ++i)
		{
			DirectX::XMFLOAT3 total(0.0f, 0.0f, 0.0f);
			auto it = tangentVtx[i].begin();
			while (it != tangentVtx[i].end())
			{
				total.x += it->x;
				total.y += it->y;
				total.z += it->z;
				++it;
			}
			total.x /= tangentVtx[i].size();
			total.y /= tangentVtx[i].size();
			total.z /= tangentVtx[i].size();
			destVtx[i].tangent = total;
		}
	};

	// �����̃��f���f�[�^����ڃx�N�g�����̃��f���f�[�^����
	Model* pTangentP = CreateObj<Model>("TangentPlane");
	Model* pTangent = CreateObj<Model>("TangentModel");
	Model* pPlane = GetObj<Model>("ModelPlane");
	Model* pModel = GetObj<Model>("Model");
	*pTangent = *pModel;
	*pTangentP = *pPlane;
	pTangent->RemakeVertex(sizeof(TangentVtx), calcTangent);
	pTangentP->RemakeVertex(sizeof(TangentVtx), calcTangent);

	// �@���}�b�v�̓ǂݍ���
	Texture* pTex = CreateObj<Texture>("NormalMap");
	pTex->Create("Assets/Model/plane/normal.png");
	// �n�C�g�}�b�v�̓ǂݍ���
	Texture* pHeightMap = CreateObj<Texture>("HeightMap");
	pHeightMap->Create("Assets/Model/plane/height.png");
	
}
void SceneBumpmap::Uninit()
{
}
void SceneBumpmap::Update(float tick)
{
}
void SceneBumpmap::Draw()
{
	// ���擾
	Model* pModel[] = {
		GetObj<Model>("TangentModel"),
		GetObj<Model>("TangentPlane")
	};
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
		GetObj<Shader>("PS_TexColor"),
		GetObj<Shader>("VS_Bumpmap"),
		GetObj<Shader>("PS_Bumpmap"),
		GetObj<Shader>("PS_ParallaxMapping"),
		GetObj<Shader>("PS_ParallaxOcclusion"),
	};
	int shaderPair[][2] = {
		{0, 1}, // �ʏ�\��
		{2, 3}, // �o���v�}�b�s���O
		{2, 4}, // �����}�b�s���O
		{2, 5}, // �����Օ��}�b�s���O
	};
	// �@���}�b�v�擾
	Texture* pNormalMap = GetObj<Texture>("NormalMap");
	// �n�C�g�}�b�v�擾
	Texture* pHeightMap = GetObj<Texture>("HeightMap");


	// �`��
	int drawNum = _countof(shaderPair);
	for (int i = 0; i < drawNum; ++i)
	{
		for(int j = 0; j < 2; ++ j)
		{
			// ���W�̍X�V
			DirectX::XMStoreFloat4x4(&mat[0],
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixScaling(0.98f, 0.98f, 0.98f) *
					DirectX::XMMatrixTranslation(
						(drawNum - 1) * 0.5f - i, 0.0f,
						(j - 1) * 1.5f
					)));
			mat[0] = mat[0] * pModel[j]->GetScaleBaseMatrix();

			// �萔�o�b�t�@�̍X�V
			shader[shaderPair[i][0]]->WriteBuffer(0, mat);
			shader[shaderPair[i][0]]->WriteBuffer(1, light);
			shader[shaderPair[i][0]]->WriteBuffer(2, camera);
			shader[shaderPair[i][1]]->WriteBuffer(0, light);
			shader[shaderPair[i][1]]->WriteBuffer(1, camera);
			// �e�N�X�`���ݒ�
			shader[shaderPair[i][1]]->SetTexture(1, pNormalMap);
			shader[shaderPair[i][1]]->SetTexture(2, pHeightMap);

			// �`��
			pModel[j]->SetVertexShader(shader[shaderPair[i][0]]);
			pModel[j]->SetPixelShader(shader[shaderPair[i][1]]);
			pModel[j]->Draw();
		}

#ifdef _DEBUG
		// �ڃx�N�g���̃f�o�b�O�\��
		if (i != 0) continue;

		Geometory::SetView(pCamera->GetView());
		Geometory::SetProjection(pCamera->GetProj());
		DirectX::XMFLOAT4 color[] = {
			{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}
		};
		DirectX::XMFLOAT3 basePos(
			(drawNum - 1) * 0.5f - i, 0.0f,
			(0 - 1) * 1.5f
		);		
		int meshNum = pModel[0]->GetMeshNum();
		for (int j = 0; j < meshNum; ++j)
		{
			const Model::Mesh* pMesh = pModel[0]->GetMesh(j);
			MeshBuffer::Description desc = pMesh->mesh->GetDesc();
			const TangentVtx* pVtx = reinterpret_cast<const TangentVtx*>(desc.pVtx);
			for (int k = 0; k < desc.vtxCount; ++k)
			{
				DirectX::XMFLOAT3 p = pVtx[k].pos;
				DirectX::XMFLOAT3 t = pVtx[k].tangent;
				p.x += basePos.x;
				p.y += basePos.y;
				p.z += basePos.z;
				const float l = 0.03f;
				Geometory::AddLine(
					p, color[0],
					DirectX::XMFLOAT3(p.x + t.x * l, p.y + t.y * l, p.z + t.z * l),
					color[1]
				);
			}
		}
		Geometory::DrawLines();
#endif

	}
}