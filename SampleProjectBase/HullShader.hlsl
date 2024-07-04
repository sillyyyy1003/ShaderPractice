// 頂点シェーダーからの受け取り
struct HS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
// テッセレーションステージへの出力
struct HS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// パッチ定数関数出力
// パッチ...メッシュを構成する面（サーフェス）
struct HS_CONSTANT_OUT
{
	// 辺の分割数の指定、この配列の数はパッチの形状(線,三角,四角)に応じた数となる 
	// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422455(v=vs.85)?redirectedfrom=MSDN
	float EdgeTessFactor[4]	: SV_TessFactor;
	// 内部の分割数
	// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422453(v=vs.85)?redirectedfrom=MSDN
	float InsideTessFactor[2]	: SV_InsideTessFactor;
};
// パッチ定数関数
// @param[in] input 入力として使用できる制御点の配列（入力の型情報、パッチを構成する頂点数）
// @param[in] id パッチID
// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422338(v=vs.85)?redirectedfrom=MSDN
HS_CONSTANT_OUT CalcHSPatchConstants(
	InputPatch<HS_IN, 4> input,
	uint id : SV_PrimitiveID
) {
	HS_CONSTANT_OUT cout;
	cout.EdgeTessFactor[0] =
	cout.EdgeTessFactor[1] =
	cout.EdgeTessFactor[2] =
	cout.EdgeTessFactor[3] =
	cout.InsideTessFactor[0] =
	cout.InsideTessFactor[1] = 20.0f;
	return cout;
}



// パッチタイプの指定
// https://learn.microsoft.com/ja-jp/previous-versions/direct-x/ee422313(v=vs.85)
[domain("quad")]
// 分割方法
[partitioning("integer")]
// 分割後のプリミティブ形状
[outputtopology("triangle_cw")]
// 出力後のコントロールポイント数
[outputcontrolpoints(4)]
// パッチ定数関数の指定
[patchconstantfunc("CalcHSPatchConstants")]
// ハルシェーダーのエントリポイント（事前にハルシェーダーに必要な属性を指定しておく
HS_OUT main( 
	InputPatch<HS_IN, 4> hin,
	uint id : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID
) {
	HS_OUT hout;
	hout.pos = hin[id].pos;
	hout.uv = hin[id].uv;
	return hout;
}
