#include "ScenePBR.h"
#include "Model.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "Input.h"
#include "Sprite.h"

// PBR用の頂点データ
struct TangentVtx
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	// 新しくモデルに追加する頂点データ
	DirectX::XMFLOAT3 tangent; // ワールド空間上でテクスチャを貼り付ける横方向
};

void ScenePBR::Init()
{
	m_time = 0.0f;

	// シェーダーの読み込み処理
	Shader* shader[] = {
		CreateObj<VertexShader>("VS_Object"),
		CreateObj<PixelShader>("PS_TexColor"),
		CreateObj<VertexShader>("VS_PBR"),
		CreateObj<PixelShader>("PS_PBR"),
		CreateObj<VertexShader>("VS_Bumpmap"),
		CreateObj<PixelShader>("PS_Bumpmap"),
	};
	const char* file[] = {
		"Assets/Shader/VS_Object.cso",
		"Assets/Shader/PS_TexColor.cso",
		"Assets/Shader/VS_PBR.cso",
		"Assets/Shader/PS_PBR.cso",
		"Assets/Shader/VS_Bumpmap.cso",
		"Assets/Shader/PS_Bumpmap.cso",
	};
	int shaderNum = _countof(shader);
	for (int i = 0; i < shaderNum; ++i)
	{
		if (FAILED(shader[i]->Load(file[i])))
		{
			MessageBox(NULL, file[i], "Shader Error", MB_OK);
		}
	}

	// タンジェントベクトルをモデルに付与するための計算
	auto calcTangent = [](Model::RemakeInfo& data)
	{
		// 一度既存のデータを新しいデータへコピーする
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

		// 接ベクトルの計算
		using TanVecAry = std::vector<DirectX::XMFLOAT3>;
		std::vector<TanVecAry> tangentVtx;
		tangentVtx.resize(data.vtxNum);
		const UINT* idx = reinterpret_cast<const UINT*>(data.idx);
		for (UINT i = 0; i < data.idxNum; i += 3) // 1面(三頂点,三角形)ずつ処理
		{
			// 三角形を構成する頂点情報の取得
			UINT idx0 = idx[i + 0];
			UINT idx1 = idx[i + 1];
			UINT idx2 = idx[i + 2];
			DirectX::XMFLOAT3 p[3] = {
				destVtx[idx0].pos, destVtx[idx1].pos, destVtx[idx2].pos,
			};
			DirectX::XMFLOAT2 uv[3] = {
				destVtx[idx0].uv, destVtx[idx1].uv, destVtx[idx2].uv
			};
			// 三角形を構成する２辺の座標とUVの変化を表すベクトルを計算
			DirectX::XMFLOAT3 V1(p[1].x - p[0].x, p[1].y - p[0].y, p[1].z - p[0].z);
			DirectX::XMFLOAT3 V2(p[2].x - p[0].x, p[2].y - p[0].y, p[2].z - p[0].z);
			DirectX::XMFLOAT2 ST1(uv[1].x - uv[0].x, uv[1].y - uv[0].y);
			DirectX::XMFLOAT2 ST2(uv[2].x - uv[0].x, uv[2].y - uv[0].y);

			// 辺の情報から接ベクトルを計算
			float factor = ST1.x * ST2.y - ST2.x * ST1.y;
			DirectX::XMFLOAT3 T(
				(ST2.y * V1.x - ST1.y * V2.x) / factor,
				(ST2.y * V1.y - ST1.y * V2.y) / factor,
				(ST2.y * V1.z - ST1.y * V2.z) / factor
			);
			// 接ベクトルの正規化
			float length = sqrtf(T.x * T.x + T.y * T.y + T.z * T.z);
			T.x /= length;
			T.y /= length;
			T.z /= length;
			// 頂点に接ベクトルの情報を追加
			tangentVtx[idx0].push_back(T);
			tangentVtx[idx1].push_back(T);
			tangentVtx[idx2].push_back(T);
		}

		// 頂点に割り当てられた接ベクトルの平均を求める
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

	Model* pTangent = CreateObj<Model>("TangentModel");
	Model* pModel = GetObj<Model>("Model");
	*pTangent = *pModel;
	pTangent->RemakeVertex(sizeof(TangentVtx), calcTangent);
	// 法線マップの読み込み
	Texture* pTex = CreateObj<Texture>("NormalMap");
	pTex->Create("Assets/Model/plane/normal.png");
}
void ScenePBR::Uninit()
{
}
void ScenePBR::Update(float tick)
{
	m_time += tick;
}
void ScenePBR::Draw()
{
	// 事前情報の取得
	//Model* pModel = GetObj<Model>("Model");
	Model* pModel = GetObj<Model>("TangentModel");
	CameraBase* pCamera = GetObj<CameraBase>("Camera");
	LightBase* pLight = GetObj<LightBase>("Light");
	// 法線マップ取得
	Texture* pNormalMap = GetObj<Texture>("NormalMap");

	// 定数バッファに渡す行列の情報
	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();
	// 定数バッファに渡すライトの情報
	DirectX::XMFLOAT3 lightDir =
		pLight->GetDirection();
	DirectX::XMFLOAT4 light[] = {
		pLight->GetDiffuse(),
		pLight->GetAmbient(),
		{lightDir.x, lightDir.y, lightDir.z, 0.0f}
	};
	// 定数バッファに渡すカメラの情報
	DirectX::XMFLOAT3 camPos = pCamera->GetPos();
	DirectX::XMFLOAT4 camera[] = {
		{camPos.x, camPos.y, camPos.z, 0.0f}
	};
	// 定数バッファに渡すスぺキュラ/メタリック/スムースを設定
	float specularMetallicSmooth[] = { 1.0f,0.1f,0.1f,0.0f };

	// シェーダーの取得
	Shader* shader[] = {
		GetObj<Shader>("VS_Object"),
		GetObj<Shader>("PS_TexColor"),
		GetObj<Shader>("VS_PBR"),
		GetObj<Shader>("PS_PBR"),
		GetObj<Shader>("VS_Bumpmap"),
		GetObj<Shader>("PS_Bumpmap"),
	};
	int shaderPair[][2] = {
		{0, 1}, // 通常表示
		{2, 3}, // PBR
		{4, 5},
	};

	// 描画
	int drawNum = _countof(shaderPair);
	for (int i = 0; i < drawNum; ++i)
	{
		// 座標の更新
		DirectX::XMStoreFloat4x4(&mat[0],
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixTranslation(
				(drawNum - 1) * 0.5f - i * 3, 0.0f, 0.0f
				)));

		// 描画
		mat[0] = mat[0] * pModel->GetScaleBaseMatrix();
		// 定数バッファの更新
		shader[shaderPair[i][0]]->WriteBuffer(0, mat);
		shader[shaderPair[i][0]]->WriteBuffer(1, light);
		shader[shaderPair[i][0]]->WriteBuffer(2, camera);
		shader[shaderPair[i][1]]->WriteBuffer(0, light);
		shader[shaderPair[i][1]]->WriteBuffer(1, camera);
		shader[shaderPair[i][1]]->WriteBuffer(2, specularMetallicSmooth);
		// テクスチャ設定
		shader[shaderPair[i][1]]->SetTexture(1, pNormalMap);

		pModel->SetVertexShader(shader[shaderPair[i][0]]);
		pModel->SetPixelShader(shader[shaderPair[i][1]]);
		pModel->Draw();
	}
}