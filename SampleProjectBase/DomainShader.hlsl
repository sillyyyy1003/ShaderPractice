// 
struct DS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
struct DS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// 出力パッチ定数データ。
struct HS_CONSTANT_OUT
{
	float EdgeTessFactor[4]	: SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};


cbuffer Matrix : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

[domain("quad")]
DS_OUT main(
	HS_CONSTANT_OUT hout,
	const OutputPatch<DS_IN, 4> din,
	float2 domain : SV_DomainLocation)
{
	DS_OUT dout;

	dout.pos = lerp(
		lerp(din[0].pos, din[1].pos, domain.x),
		lerp(din[3].pos, din[2].pos, domain.x),
		domain.y);

	dout.uv = lerp(
		lerp(din[0].uv, din[1].uv, domain.x),
		lerp(din[3].uv, din[2].uv, domain.x),
		domain.y);
	//	din[0].pos * domain.x +
	//	din[1].pos * domain.y +
	//	din[2].pos * domain.z;
	//	din[0].uv * domain.x +
	//	din[1].uv * domain.y +
	//	din[2].uv * domain.z;

	float height = tex.SampleLevel(samp, dout.uv, 0).r;

	dout.pos.y += height * 0.2f;
	dout.pos = mul(dout.pos, world);
	dout.pos = mul(dout.pos, view);
	dout.pos = mul(dout.pos, proj);

	return dout;
}
