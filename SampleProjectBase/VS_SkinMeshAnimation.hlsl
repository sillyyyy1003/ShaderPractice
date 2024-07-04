struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
    int4 boneIndex : BONEINDEX;
    float4 boneWeight : BONEWEIGHT;
};
struct VS_OUT
{
	float4 pos : SV_POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	// �X�N���[�����W�Ƃ͕ʂɁA���[���h���W�̃f�[�^��p��
	// �s�N�Z���V�F�[�_�[�ɓn��ƁA�s�N�Z�����Ƃ̃��[���h���W�̃f�[�^�ɂȂ�
	float4 worldPos : POSITION0;
	float4 screenPos : POSITION1;
};

cbuffer WVP : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

cbuffer BoneMatrixBuffer : register(b5)
{
    matrix BoneMatrix[400];
}

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	
	// �����X�L�����_�u�����h�̏���
	float4x4 Comb = (float4x4) 0;
	for (int i = 0; i < 4; i++)
	{
		// �d�݂��v�Z���Ȃ���s�񐶐�
		Comb += BoneMatrix[vin.boneIndex[i]] * vin.boneWeight[i];
    }
	
	float4 Pos;
    Pos = float4(vin.pos, 1.0f);
    Pos = mul(Comb, Pos);
	vout.pos = Pos;
	
	// �@���x�N�g����␳
	float4 Normal;
	Normal = float4(vin.normal.xyz, 0.0);
 
	Comb._41 = 0.0f; // �ړ�����������
	Comb._42 = 0.0f;
	Comb._43 = 0.0f;
	Comb._44 = 1.0f;

    Normal = mul(Comb, float4(vin.normal, 0.0));
	normalize(Normal);
	vin.normal = Normal;
	
	//vout.pos = float4(vin.pos, 1.0f);
	//vout.pos = vin.pos;
	vout.pos = mul(vout.pos, world);
	// �s�N�Z���V�F�[�_�[�ɓn�����߂ɁA�r���ŕϐ��ɃR�s�[
	vout.worldPos = vout.pos;

	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	vout.screenPos = vout.pos;

	vout.uv = vin.uv;
	vout.normal = mul(vin.normal, (float3x3)world);
	return vout;
}