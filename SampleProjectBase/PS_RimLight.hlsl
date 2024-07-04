struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

cbuffer Light : register(b0)
{
    float4 lightDiffuse;
    float4 lightAmbient;
    float4 lightDir;
};
cbuffer Camera : register(b1)
{
    float4 camPos;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	// テクスチャから色を取得
    color = tex.Sample(samp, pin.uv);

    float3 N = normalize(pin.normal); // 法線ベクトルを正規化
    float3 L = normalize(-lightDir.xyz); // ライトの方向を正規化
    float diffuse = saturate(dot(N, L)); // 拡散反射光の計算(法線ベクトルとライトの方向の内積)

    color *= diffuse * lightDiffuse + lightAmbient; //カラーに拡散反射光と環境光を適用

    float3 V = normalize(camPos.xyz - pin.worldPos);
    float lv = saturate(-dot(L, V));

    float edge = 1.3f - saturate(dot(N, V));
    float rim = lv * pow(edge, 5.0f);

    color += rim;
    

    return color;
}
