struct PS_IN
{
	float4 pos : SV_POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 worldPos : POSITION0;
	float4 screenPos : POSITION1;
};

cbuffer Camera : register(b0)
{
	float4 camPos;
};

Texture2D tex : register(t0);
Texture2D ditherTex : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = tex.Sample(samp, pin.uv);

	// �����ɉ����ē����x��ύX
	float len = length(pin.worldPos.xyz - camPos.xyz);
	float alpha = color.a * saturate(len * 0.2f);

	// �X�N���[���̍��W�ɉ����āA
	// �f�B�U�p�^�[����K�p����ʒu���v�Z
	float2 screenUV = pin.screenPos.xy / pin.screenPos.w;
	screenUV = screenUV * 0.5f + 0.5f;
	screenUV *= float2(1280.0f, 720.f) / 4.0f;

	// �f�B�U�p�^�[���̎擾
	float dither = ditherTex.Sample(samp, screenUV).r;

	if (alpha < dither)
	{
		// �����x0�Ō����Ȃ����ǂ����ɂ���A�ł͂Ȃ����S�ɕ\�����s��Ȃ�
		discard;
	}

	return color;
}



