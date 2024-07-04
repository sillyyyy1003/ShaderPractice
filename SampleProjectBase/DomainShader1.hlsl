// ドメインシェーダーからピクセルシェーダーに送るデータの定義
struct DS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// 出力制御点
// ハルシェーダーからドメインシェーダーに渡されるデータの定義
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// 出力パッチ定数データ
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]	: SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};


// 定数バッファ
cbuffer Matrix : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);


#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	// 四角形の計算
	// 配列の順番は頂点バッファ内の頂点の順番に依存する
	float4 edge[2] = {
		lerp(patch[1].pos, patch[2].pos, domain.x), // 上の辺
		lerp(patch[0].pos, patch[3].pos, domain.x)	// 下の辺
	};
	Output.pos = lerp(edge[0], edge[1], domain.y);
	float2 edgeUV[2] = {
		lerp(patch[1].uv, patch[2].uv, domain.x),
		lerp(patch[0].uv, patch[3].uv, domain.x)
	};
	Output.uv = lerp(edgeUV[0], edgeUV[1], domain.y);

	/* 三角形の計算
	各頂点に重みを付けて、合算した値が分割後の頂点になる
	Output.vPosition = float4(
		patch[0].vPosition*domain.x + 
		patch[1].vPosition*domain.y + 
		patch[2].vPosition*domain.z,1);
		*/

	// テクスチャに描きこまれた値に応じて、頂点の高さを変更
	// ドメインシェーダーではSampleが利用できない
	// 代わりにSampleLevelを使用する
	float height = tex.SampleLevel(samp, Output.uv, 0).r;
	Output.pos.y += height * 0.3f;

	// 分割後の頂点を変換行列で変換
	Output.pos = mul(Output.pos, world);
	Output.pos = mul(Output.pos, view);
	Output.pos = mul(Output.pos, proj);


	return Output;
}
