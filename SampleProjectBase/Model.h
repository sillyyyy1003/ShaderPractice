#ifndef __MODEL_H__
#define __MODEL_H__

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Shader.h"
#include "MeshBuffer.h"
#include "Texture.h"

#include "assimp/Importer.hpp"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"

#include "DirectXTex/SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Model
{

public:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		int		boneIndex[4];
		float	boneWeight[4];
		int		boneCount = 0;
	};
	struct Material
	{
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 specular;
		std::shared_ptr<Texture> texture;
	};
	using Materials = std::vector<Material>;
	struct Mesh
	{
		std::shared_ptr<MeshBuffer> mesh;
		unsigned int materialID;
	};
	using Meshes = std::vector<Mesh>;

	struct RemakeInfo
	{
		UINT vtxNum;
		void* dest;
		const void* source;
		UINT idxNum;
		const void* idx;
	};

	// �E�F�C�g���@20231225 �ǉ�
	struct WEIGHT {
		std::string bonename;						// �{�[����
		std::string meshname;						// ���b�V����
		float weight;								// �E�F�C�g�l
		int	vertexindex;							// ���_�C���f�b�N�X
	};

	//�{�[���\����
	struct BONE
	{
		std::string Bonename;						// �{�[����
		std::string Meshname;						// ���b�V����
		std::string Armaturename;					// �A�[�}�`���A��
		SimpleMath::Matrix Matrix;
		SimpleMath::Matrix AnimationMatrix;
		SimpleMath::Matrix OffsetMatrix;
		int			idx;
		std::vector<WEIGHT> weights;				// ���̃{�[�����e����^���钸�_�C���f�b�N�X�E�E�F�C�g�l
	};

	struct CBBoneCombMatrix {
		XMFLOAT4X4 BoneCombMtx[400];						// �{�[���R���r�l�[�V�����s��
	};
public:
	Model();
	~Model();
	void SetVertexShader(Shader* vs);
	void SetPixelShader(Shader* ps);
	const Mesh* GetMesh(unsigned int index) const;
	unsigned int GetMeshNum() const;

public:
	bool Load(const char* file, float scaleBase = 1.0f, bool flip = false, bool simple = false);
	void LoadAnimation(const char* FileName, const char* Name, bool flip);
	void Draw(int texSlot = 0);
	
	void UpdateAnimation(const char* AnimationName, int Frame);
	void UpdateBoneMatrix(aiNode* node, Matrix matrix);

	void RemakeVertex(int vtxSize, std::function<void(RemakeInfo& data)> func);
	void CreateBone(const aiNode* node);
	void BoneUpdate();
	SimpleMath::Matrix GetScaleBaseMatrix();
	float GetScaleBase();
private:
	void MakeDefaultShader();
	bool CreateConstantBufferWrite(
		ID3D11Device* device,					// �f�o�C�X�I�u�W�F�N�g
		unsigned int bytesize,					// �R���X�^���g�o�b�t�@�T�C�Y
		ID3D11Buffer** pConstantBuffer			// �R���X�^���g�o�b�t�@
	);
	std::vector<BONE> GetBoneInfo(const aiMesh* mesh);
	DirectX::SimpleMath::Matrix aiMtxToDxMtx(aiMatrix4x4 aimatrix);

private:
	Assimp::Importer* importer = nullptr;								// assimp�̐ݒ�
	const aiScene* m_pScene = nullptr;						// ���[�h�ς݃��f�����
	static std::shared_ptr<VertexShader> m_defVS;			// ���_�V�F�[�_�[
	static std::shared_ptr<PixelShader> m_defPS;			// �s�N�Z���V�F�[�_�[
	std::vector<Matrix> m_bonecombmtxcontainer;				// �{�[���R���r�l�[�V�����s��̔z��

	std::unordered_map<std::string, BONE> m_Bone;			// �{�[���f�[�^�i���O�ŎQ�Ɓj
	ID3D11Buffer* m_BoneCombMtxCBuffer = nullptr;						// �萔�o�b�t�@
	std::unordered_map<std::string, const aiScene*> m_Animation;

private:
	Meshes m_meshes;
	Materials m_materials;
	VertexShader* m_pVS;
	PixelShader* m_pPS;
	float m_scaleBase = 1.0f;		// ���f���̃X�P�[���{�� ���W�ϊ����Ɉ����̂�model�N���X���ł͌��󈵂�Ȃ�
};


#endif // __MODEL_H__