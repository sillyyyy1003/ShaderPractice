struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 worldPos : POSITION0;
};

cbuffer Light : register(b0)
{
    float4 lightColor;
    float3 lightPos;
    float lightRange;
    float3 lightDir; // 光の方向
    float lightAngle; // スポットライトの制限角度
};


Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	//各ピクセルから点光源に向かうベクトルを計算
    float3 toLightVec = lightPos.xyz - pin.worldPos.xyz;
    float3 V = normalize(toLightVec); //ベクトル正規化
    float3 toLightLen = length(toLightVec); //光源までの距離

	//点光源に向かうベクトルと法線から明るさを計算
    float3 N = normalize(pin.normal);
    float dotNV = saturate(dot(N, V));

	//距離に応じて光の強さを変える
	//距離が近つければ明るく離れていればくらくなるよう計算
    float3 attenuation = saturate(1.0f - toLightLen / lightRange);

	//自然な減衰の計算
	//距離に応じて光のあたる面が乗算で増えていく
    attenuation = pow(attenuation, 2.0f);

    float3 L = normalize(-lightDir);

    float dotVL = dot(V, L);
    float angle = acos(dotVL);

    //角度に応じて明るさを計算（1から０にグラデーションにする処理）
    float diffAngle = (lightAngle * 0.5f) * 0.1f;
    float spotAngle = ((lightAngle * 0.5f) + diffAngle);
    float spotRate = (spotAngle - angle) / diffAngle;
    spotRate = pow(saturate(spotRate), 2.0f);

    color.rgb = dotNV * attenuation * spotRate;
    return color;
}

