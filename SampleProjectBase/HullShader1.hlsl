// ハルシェーダー　分割方法を定義するシェーダー

// 入力制御点
// ControlPoint ... 制御点（頂点のこと）
// 頂点シェーダーから渡されるデータの定義
struct VS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// 出力制御点
// ハルシェーダーからドメインシェーダーに
// 渡されるデータの定義
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// 出力パッチ定数データ
// パッチ ... 分割を行う形状（線、三角、四角
// 形状ごとに用意するデータが決まってる（ドキュメント参照
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]	: SV_TessFactor; // たとえば、クワド ドメインの [4] になります
	float InsideTessFactor[2] : SV_InsideTessFactor; // たとえば、クワド ドメインの Inside[2] になります
};

// １パッチの頂点数
#define NUM_CONTROL_POINTS 4

// パッチ定数関数
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// 外側の分割数
	Output.EdgeTessFactor[0] = 
	Output.EdgeTessFactor[1] = 
	Output.EdgeTessFactor[2] = 
	Output.EdgeTessFactor[3] = 100.0f;
	// 内側の分割数
	Output.InsideTessFactor[0] =
	Output.InsideTessFactor[1] = 100.0f;

	return Output;
}


[domain("quad")] // 分割形状の設定(isoline,tri,quad
[partitioning("fractional_odd")] // 分割方法(アルゴリズム
[outputtopology("triangle_cw")] // 表面の設定
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// 頂点シェーダーのデータをドメインシェーダーへ渡す
	// ここでパラメータの変更を行なってもよい
	Output.pos = ip[i].pos;
	Output.uv = ip[i].uv;

	return Output;
}
