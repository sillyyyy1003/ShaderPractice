#include "Model.h"
#include "Defines.h"
#include "DebugLog.h"

#include <assimp/postprocess.h>

#if _MSC_VER >= 1920
#ifdef _DEBUG
#pragma comment(lib, "assimp/x64/Debug/assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp/x64/Release/assimp-vc142-mt.lib")
#endif
#elif _MSC_VER >= 1910
#ifdef _DEBUG
#pragma comment(lib, "assimp/x64/Debug/assimp-vc141-mtd.lib")
#endif
#endif

std::shared_ptr<VertexShader> Model::m_defVS = nullptr;
std::shared_ptr<PixelShader> Model::m_defPS = nullptr;

Model::Model()
	: m_pVS(nullptr)
	, m_pPS(nullptr)
{
	if (!m_defVS && !m_defPS) // �ǂ����nullptr
	{
		MakeDefaultShader();
	}
	m_pVS = m_defVS.get();
	m_pPS = m_defPS.get();
	importer = new Assimp::Importer();
}
Model::~Model()
{
}

void Model::SetVertexShader(Shader* vs)
{
	VertexShader* cast = reinterpret_cast<VertexShader*>(vs);
	if(cast)
		m_pVS = cast;
}
void Model::SetPixelShader(Shader* ps)
{
	PixelShader* cast = reinterpret_cast<PixelShader*>(ps);
	if (cast)
		m_pPS = cast;
}

const Model::Mesh* Model::GetMesh(unsigned int index) const
{
	if (index < 0 || m_meshes.size() <= index)
	{
		return nullptr;
	}
	return &m_meshes[index];
}

uint32_t Model::GetMeshNum() const
{
	return static_cast<uint32_t>(m_meshes.size()) ;
}

bool Model::Load(const char* file, float scaleBase, bool flip, bool simpleMode)
{
	DebugLog::Log("���f���ǂݍ��݊J�n");
	m_scaleBase = scaleBase;

	int flag = 0;
	if (simpleMode)
	{
		flag |= aiProcess_Triangulate;					// ��O�p�|���S�����O�p�Ɋ���
		flag |= aiProcess_JoinIdenticalVertices;		// ����ʒu���_����ɓ�������
		flag |= aiProcess_FlipUVs;						//�@UV�l��Y������ɔ��]������
		flag |= aiProcess_PreTransformVertices;			// �m�[�h����ɓ��� !!�A�j���[�V������񂪏����邱�Ƃɒ���!!
		if (flip) flag |= aiProcess_MakeLeftHanded;		// ����n���W�ɕϊ�
	}
	else
	{
		flag |= aiProcessPreset_TargetRealtime_MaxQuality;	// ���A���^�C�� �����_�����O�p�Ƀf�[�^���œK������f�t�H���g�̌㏈���\���B
		flag |= aiProcess_PopulateArmatureData;				// �W���I�ȃ{�[��,�A�[�}�`���A�̐ݒ�
		if (flip) flag |= aiProcess_ConvertToLeftHanded;	// ����n�ύX�I�v�V�������܂Ƃ܂�������
	}

	// assimp�œǂݍ���
	m_pScene = importer->ReadFile(file, flag);
	if (!m_pScene) {
		Error(importer->GetErrorString());
		DebugLog::Log("{} Assimp���f�����[�h���s", file);
		return false;
	}
	assert(m_pScene);
	// �{�[�����z�񏀔�
	DebugLog::Log("�{�[�����z�񏀔�");
	CreateBone(m_pScene->mRootNode);

	// �{�[���̔z��C���f�b�N�X���i�[����
	unsigned int num = 0;
	for (auto& data : m_Bone) {
		data.second.idx = num;
		num++;
	}

	// ���b�V���̍쐬
	aiVector3D zero(0.0f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < m_pScene->mNumMeshes; ++i)
	{
		Mesh mesh = {};

		// ���_�̍쐬
		std::vector<Vertex> vtx;
		vtx.resize(m_pScene->mMeshes[i]->mNumVertices);
		for (unsigned int j = 0; j < vtx.size(); ++j)
		{
			// �l�̋z�o��
			aiVector3D pos = m_pScene->mMeshes[i]->mVertices[j];
			aiVector3D uv = m_pScene->mMeshes[i]->HasTextureCoords(0) ?
				m_pScene->mMeshes[i]->mTextureCoords[0][j] : zero;
			aiVector3D normal = m_pScene->mMeshes[i]->HasNormals() ?
				m_pScene->mMeshes[i]->mNormals[j] : zero;
			// �l��ݒ�
			vtx[j] = {
				DirectX::XMFLOAT3(pos.x, pos.y, pos.z),
				DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
				DirectX::XMFLOAT2(uv.x, uv.y),
				{-1,-1,-1,-1},
				{0.0f,0.0f,0.0f,0.0f}
			};
		}

		// �{�[�����擾
		std::vector<BONE> boneList = GetBoneInfo(m_pScene->mMeshes[i]);

		// ���_���Ƀ{�[�����R�Â�
		for (auto& bone : boneList)
		{
			for (auto& w : bone.weights)
			{
				int& idx = vtx[w.vertexindex].boneCount;
				vtx[w.vertexindex].boneIndex[idx] = m_Bone[w.bonename].idx;		// index���Z�b�g
				vtx[w.vertexindex].boneWeight[idx] = w.weight;					// weight�l���Z�b�g
				//�{�[���̔z��ԍ����Z�b�g
				idx++;
				assert(idx <= 4);
			}
		}

		// �C���f�b�N�X�̍쐬
		std::vector<unsigned int> idx;
		idx.resize(m_pScene->mMeshes[i]->mNumFaces * 3); // face�̓|���S���̐���\��(�P�|���S����3�C���f�b�N�X
		for (unsigned int j = 0; j < m_pScene->mMeshes[i]->mNumFaces; ++j)
		{
			aiFace face = m_pScene->mMeshes[i]->mFaces[j];
			int faceIdx = j * 3;
			idx[faceIdx + 0] = face.mIndices[0];
			idx[faceIdx + 1] = face.mIndices[1];
			idx[faceIdx + 2] = face.mIndices[2];
		}

		// �}�e���A���̊��蓖��
		mesh.materialID = m_pScene->mMeshes[i]->mMaterialIndex;

		// ���b�V�������ɒ��_�o�b�t�@�쐬
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = static_cast<UINT>(vtx.size());
		desc.pIdx = idx.data();
		desc.idxSize = sizeof(unsigned int);
		desc.idxCount = static_cast<UINT>(idx.size());
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		mesh.mesh = std::make_shared<MeshBuffer>(desc);

		// ���b�V���ǉ�
		m_meshes.push_back(mesh);
	}

	// �萔�o�b�t�@�����@20230909-02
	CreateConstantBufferWrite(
		GetDevice(),					// �f�o�C�X�I�u�W�F�N�g
		sizeof(CBBoneCombMatrix),		// �R���X�^���g�o�b�t�@�T�C�Y
		&m_BoneCombMtxCBuffer);			// �R���X�^���g�o�b�t�@

	assert(m_BoneCombMtxCBuffer);

	//--- �}�e���A���̍쐬
	// �t�@�C���̒T��
	std::string dir = file;
	dir = dir.substr(0, dir.find_last_of('/') + 1);
	// �}�e���A��
	aiColor3D color(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 diffuse(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT4 ambient(0.3f, 0.3f, 0.3f, 1.0f);
	for (unsigned int i = 0; i < m_pScene->mNumMaterials; ++i)
	{
		Material material = {};

		// �e��p�����[�^�[
		float shininess;
		material.diffuse = m_pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f) : diffuse;
		material.ambient = m_pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f) : ambient;
		shininess = m_pScene->mMaterials[i]->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS ? shininess : 0.0f;
		material.specular = m_pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, shininess) : DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, shininess);

		// �e�N�X�`��
		aiString path;
		if (m_pScene->mMaterials[i]->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS) {
			HRESULT hr;
			// �t�@�C���`���`�F�b�N
			if (strstr(path.C_Str(), ".psd"))
			{
				DebugLog::LogError("���f���̃e�N�X�`����psd�t�@�C�����w�肳��Ă��܂��Bpsd�ǂݍ��݂ɂ͔�Ή��B");
				Error("���f���̃e�N�X�`����psd�t�@�C�����w�肳��Ă��܂��Bpsd�ǂݍ��݂ɂ͔�Ή��B");
			}
			// ���̂܂ܒT��
			material.texture = std::make_shared<Texture>();
			hr = material.texture->Create(path.C_Str());
			// ���f���Ɠ����K�w��T��
			if (FAILED(hr)) {
				std::string file = dir;
				file += path.C_Str();
				hr = material.texture->Create(file.c_str());
			}
			// �t�@�C�����݂̂ŒT��
			if (FAILED(hr)) {
				std::string file = path.C_Str();
				if (size_t idx = file.find_last_of('\\'); idx != std::string::npos)
				{
					file = file.substr(idx + 1);
					file = dir + file;
					hr = material.texture->Create(file.c_str());
				}

				// �t�@�C���ڂ̃p�X��"\\�ł͂Ȃ�"/"�̏ꍇ�̑Ή�
				if (FAILED(hr)) {
					if (size_t idx = file.find_last_of('/'); idx != std::string::npos)
					{
						file = file.substr(idx + 1);
						file = dir + file;
						hr = material.texture->Create(file.c_str());
					}
				}
			}
			// ���s
			if (FAILED(hr)) {
				Error(path.C_Str());
				material.texture = nullptr;
			}
		}
		else {
			material.texture = nullptr;
		}

		// �}�e���A���ǉ�
		m_materials.push_back(material);
	}

	DebugLog::Log("���f���ǂݍ��݊���");
	return true;
}


void Model::LoadAnimation(const char* FileName, const char* Name, bool flip)
{
	int flag = 0;
	if (flip) flag |= aiProcess_ConvertToLeftHanded;	// ����n�ύX�I�v�V�������܂Ƃ܂�������
	m_Animation[Name] = aiImportFile(FileName, flag);
	assert(m_Animation[Name]);
}

void Model::Draw(int texSlot)
{
	m_pVS->Bind();
	m_pPS->Bind();
	auto it = m_meshes.begin();
	// 20230909-02 ���W�X�^�T�ɃZ�b�g
	GetContext()->VSSetConstantBuffers(5, 1, &m_BoneCombMtxCBuffer);
	while (it != m_meshes.end())
	{
		if(texSlot >= 0)
			m_pPS->SetTexture(texSlot, m_materials[it->materialID].texture.get());
		it->mesh->Draw();
		++it;
	}
}

void Model::RemakeVertex(int vtxSize, std::function<void(RemakeInfo& data)> func)
{
	auto it = m_meshes.begin();
	while (it != m_meshes.end())
	{
		MeshBuffer::Description desc = it->mesh->GetDesc();
		
		char* newVtx = new char[vtxSize * desc.vtxCount];
		RemakeInfo data = {};
		data.vtxNum = desc.vtxCount;
		data.dest = newVtx;
		data.source = desc.pVtx;
		data.idxNum = desc.idxCount;
		data.idx = desc.pIdx;
		func(data);

		desc.pVtx = newVtx;
		desc.vtxSize = vtxSize;
		it->mesh = std::make_shared<MeshBuffer>(desc);

		delete[] newVtx;
		++it;
	}
}

void Model::MakeDefaultShader()
{
	const char* vsCode = R"EOT(
struct VS_IN {
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
};
VS_OUT main(VS_IN vin) {
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos.z *= 0.1f;
	return vout;
})EOT";
	const char* psCode = R"EOT(
struct PS_IN {
	float4 pos : SV_POSITION;
};
float4 main(PS_IN pin) : SV_TARGET
{
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
})EOT";
	m_defVS = std::make_shared<VertexShader>();
	m_defVS->Compile(vsCode);
	m_defPS = std::make_shared<PixelShader>();
	m_defPS->Compile(psCode);
}

// �ċA�I�{�[������
void Model::CreateBone(const aiNode* node)
{
	BONE bone;

	m_Bone[node->mName.C_Str()] = bone;
	DebugLog::Log("BoneName = {}", node->mName.C_Str());

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		CreateBone(node->mChildren[n]);
	}

}

// �A�j���[�V�����X�V
void Model::UpdateAnimation(const char* AnimationName, int Frame)
{
	if (m_Animation.count(AnimationName) == 0) return;
	if (!m_Animation[AnimationName]->HasAnimations()) return;

	Matrix rootMatrix;
	// SRT����s��𐶐�
	Vector3 scale = { 1.0f, 1.0f, 1.0f };		// 20231231 DX��
	Vector3 position = { 0.0f, 0.0f, 0.0f };	// 20231231 DX��
	Quaternion rotation;						// 20231231 DX��
	rotation.x = 0.0f;							// 20231231 DX��
	rotation.y = 0.0f;							// 20231231 DX��
	rotation.z = 0.0f;							// 20231231 DX��
	rotation.w = 1.0f;							// 20231231 DX��

	Matrix scalemtx = Matrix::CreateScale(scale.x, scale.y, scale.z);					// 20231231 DX��
	Matrix rotmtx =	Matrix::CreateFromQuaternion(rotation);								// 20231231 DX��
	Matrix transmtx = Matrix::CreateTranslation(position.x, position.y, position.z);	// 20231231 DX��

	rootMatrix = scalemtx * rotmtx * transmtx;							// 20231231 DX��

	//�A�j���[�V�����f�[�^����{�[���}�g���N�X�Z�o
	aiAnimation* animation = m_Animation[AnimationName]->mAnimations[0];

	for (unsigned int c = 0; c < animation->mNumChannels; c++)
	{
		aiNodeAnim* nodeAnim = animation->mChannels[c];

		BONE* bone = &m_Bone[nodeAnim->mNodeName.C_Str()];
		int f;

		f = Frame % nodeAnim->mNumRotationKeys;				//�ȈՎ���
		aiQuaternion rot = nodeAnim->mRotationKeys[f].mValue;

		f = Frame % nodeAnim->mNumPositionKeys;				//�ȈՎ���
		aiVector3D pos = nodeAnim->mPositionKeys[f].mValue;

		// SRT����s��𐶐�
		Vector3 scale = { 1.0f, 1.0f, 1.0f };				// 20231231 DX��
		Vector3 position = { pos.x, pos.y, pos.z};			// 20231231 DX��
		Quaternion rotation;								// 20231231 DX��
		rotation.x = rot.x;									// 20231231 DX��
		rotation.y = rot.y;									// 20231231 DX��
		rotation.z = rot.z;									// 20231231 DX��
		rotation.w = rot.w;									// 20231231 DX��

		Matrix scalemtx = Matrix::CreateScale(scale.x, scale.y, scale.z);					// 20231231 DX��
		Matrix rotmtx = Matrix::CreateFromQuaternion(rotation);								// 20231231 DX��
		Matrix transmtx = Matrix::CreateTranslation(position.x, position.y, position.z);	// 20231231 DX��

		bone->AnimationMatrix = scalemtx * rotmtx * transmtx;								// 20231231 DX��
	}

	UpdateBoneMatrix(m_pScene->mRootNode, rootMatrix);

	// �萔�o�b�t�@�ɏ������ނ��߂̏��𐶐�
	m_bonecombmtxcontainer.clear();
	m_bonecombmtxcontainer.resize(m_Bone.size());
	for (auto data : m_Bone) {
		m_bonecombmtxcontainer[data.second.idx] = data.second.Matrix;
	}

	// 20230909 �]�u
	for (auto& bcmtx : m_bonecombmtxcontainer)
	{
		// �]�u����
		bcmtx.Transpose();
	}

	BoneUpdate();

}

void Model::UpdateBoneMatrix(aiNode* node, Matrix matrix)
{
	if (node->mName.length <= 0)
	{
		DebugLog::LogError("�m�[�h(�{�[��)�����擾�ł��Ă��Ȃ�or�Ȃ�");
		Error("�m�[�h(�{�[��)�����擾�ł��Ă��Ȃ�or�Ȃ�");
		return;
	}
	// �����œn���ꂽ�m�[�h�����L�[�Ƃ��ă{�[�������擾����
	BONE* bone = &m_Bone[node->mName.C_Str()];

	//�}�g���N�X�̏�Z���Ԃɒ���

	// �{�[���R���r�l�[�V�����s�񐶐�				
	Matrix bonecombinationmtx;
	bonecombinationmtx = bone->OffsetMatrix * bone->AnimationMatrix * matrix;	// �{�[���I�t�Z�b�g�s��~�{�[���A�j�����[�V�����s��~�t�{�[���I�t�Z�b�g�s��

	bone->Matrix = bonecombinationmtx;			// �{�[���R���r�l�[�V�����s����{�[�����ɔ��f������

	// �����̎p����\���s����쐬
	Matrix mybonemtx;
	mybonemtx = bone->AnimationMatrix * matrix;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		UpdateBoneMatrix(node->mChildren[n], mybonemtx);
	}
}

/*----------------------------
�R���X�^���g�o�b�t�@���쐬(MAP�ŏ��������\)
------------------------------*/
bool  Model::CreateConstantBufferWrite(
	ID3D11Device* device,					// �f�o�C�X�I�u�W�F�N�g
	unsigned int bytesize,					// �R���X�^���g�o�b�t�@�T�C�Y
	ID3D11Buffer** pConstantBuffer			// �R���X�^���g�o�b�t�@
) {

	// �R���X�^���g�o�b�t�@����
	D3D11_BUFFER_DESC bd;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;							// �o�b�t�@�g�p���@
	bd.ByteWidth = bytesize;								// �o�b�t�@�̑傫
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				// �R���X�^���g�o�b�t�@
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// CPU�A�N�Z�X�\

	HRESULT hr = device->CreateBuffer(&bd, nullptr, pConstantBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(constant buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

// �{�[���R���r�l�[�V�����s��o�b�t�@�i�[
void Model::BoneUpdate()
{
	// ���b�V������X�V���ꂽ�{�[���R���r�l�[�V�����s��Q���擾
	const std::vector<Matrix> mtxcontainer = m_bonecombmtxcontainer;			// 20231231 DX��

	// �萔�o�b�t�@�X�V
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	CBBoneCombMatrix* pData = nullptr;

	HRESULT hr = GetContext()->Map(
		m_BoneCombMtxCBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&MappedResource);

	if (SUCCEEDED(hr)) {
		pData = (CBBoneCombMatrix*)MappedResource.pData;
		memcpy(pData->BoneCombMtx,
			mtxcontainer.data(),
			sizeof(Matrix) * mtxcontainer.size());									// 20231231 DX��
		GetContext()->Unmap(m_BoneCombMtxCBuffer, 0);
	}
}

// �T�u�Z�b�g�ɕR�Â��Ă���{�[�������擾����
std::vector<Model::BONE> Model::GetBoneInfo(const aiMesh* mesh) {

	std::vector<BONE> bones;		// ���̃T�u�Z�b�g���b�V���Ŏg�p����Ă���{�[���R���e�i

	// �{�[���������[�v
	for (unsigned int bidx = 0; bidx < mesh->mNumBones; bidx++) {

		BONE bone{};

		// �{�[�����擾
		bone.Bonename = std::string(mesh->mBones[bidx]->mName.C_Str());
		// ���b�V���m�[�h��
		if (mesh->mBones[bidx]->mNode != nullptr)
		{
			bone.Meshname = std::string(mesh->mBones[bidx]->mNode->mName.C_Str());
		}else{
			DebugLog::LogWarning("�m�[�h���NULL");
		}

		// �A�[�}�`���A�m�[�h��
		if (mesh->mBones[bidx]->mArmature != nullptr)
		{
			bone.Armaturename = std::string(mesh->mBones[bidx]->mArmature->mName.C_Str());
		}else {
			DebugLog::LogWarning("�A�[�}�`���A���NULL");
		}
		// �{�[���I�t�Z�b�g�s��擾
		bone.OffsetMatrix = aiMtxToDxMtx(mesh->mBones[bidx]->mOffsetMatrix);

		// �E�F�C�g��񒊏o
		bone.weights.clear();
		for (unsigned int widx = 0; widx < mesh->mBones[bidx]->mNumWeights; widx++) {

			WEIGHT w;
			w.meshname = bone.Meshname;										// ���b�V����
			w.bonename = bone.Bonename;										// �{�[����

			w.weight = mesh->mBones[bidx]->mWeights[widx].mWeight;			// �d��
			w.vertexindex = mesh->mBones[bidx]->mWeights[widx].mVertexId;	// ���_�C���f�b�N�X
			bone.weights.emplace_back(w);
		}

		// �R���e�i�ɓo�^
		bones.emplace_back(bone);

		// �{�[�������ɂ����f������
		m_Bone[mesh->mBones[bidx]->mName.C_Str()].OffsetMatrix = aiMtxToDxMtx(mesh->mBones[bidx]->mOffsetMatrix);   // 20231231 DX��

	}

	return bones;
}


SimpleMath::Matrix Model::GetScaleBaseMatrix()
{
	return DirectX::XMMatrixScaling(m_scaleBase, m_scaleBase, m_scaleBase);
}

float Model::GetScaleBase()
{
	return m_scaleBase;
}


// assimp�s���DIRECTX�̍s��ɕϊ�����
DirectX::SimpleMath::Matrix Model::aiMtxToDxMtx(aiMatrix4x4 aimatrix) {

	DirectX::SimpleMath::Matrix dxmtx = {
	   aimatrix.a1,aimatrix.b1,aimatrix.c1,aimatrix.d1,
	   aimatrix.a2,aimatrix.b2,aimatrix.c2,aimatrix.d2,
	   aimatrix.a3,aimatrix.b3,aimatrix.c3,aimatrix.d3,
	   aimatrix.a4,aimatrix.b4,aimatrix.c4,aimatrix.d4
	};

	return dxmtx;
}