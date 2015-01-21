﻿static const char* light_ps_dx = R"(

Texture2D		g_gbuffer0Texture		: register( t0 );
SamplerState	g_gbuffer0Sampler		: register( s0 );

Texture2D		g_gbuffer1Texture		: register( t1 );
SamplerState	g_gbuffer1Sampler		: register( s1 );

Texture2D		g_gbuffer2Texture		: register( t2 );
SamplerState	g_gbuffer2Sampler		: register( s2 );

Texture2D		g_gbuffer3Texture		: register( t3 );
SamplerState	g_gbuffer3Sampler		: register( s3 );

Texture2D		g_shadowmapTexture		: register( t4 );
SamplerState	g_shadowmapSampler		: register( s4 );

Texture2D		g_ssaoTexture		: register( t5 );
SamplerState	g_ssaoSampler		: register( s5 );

Texture2D		g_environmentDiffuseTexture;
SamplerState	g_environmentDiffuseSampler;

float4x4		g_cameraPositionToShadowCameraPosition;
float4x4		g_shadowProjection;

float3 reconstructInfo1;
float4 reconstructInfo2;

float3		directionalLightDirection;
float3		directionalLightColor;
float3		skyLightColor;
float3		groundLightColor;
float3		upDir;

struct PS_Input
{
	float4 SV_Position		: SV_POSITION;

	float4 Position			: POSITION0;
	float2 UV				: UV0;
};

//<|| ALSL
float calcD_GGX(float roughness, float dotNH)
{
	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;
	float pi = 3.14159;
	float denom = dotNH * dotNH * (alphaSqr - 1.00000) + 1.00000;
	return (alpha / denom) * (alpha / denom) / pi;
}

float calcF(float F0, float dotLH)
{
	float dotLH5 = pow(1.00000f - dotLH, 5);
	return F0 + (1.00000 - F0) * (dotLH5);
}

float calcG_Schlick(float roughness, float dotNV, float dotNL)
{
	float k = (roughness + 1.00000) * (roughness + 1.00000) / 8.00000;
	float gV = dotNV * (1.00000 - k) + k;
	float gL = dotNL * (1.00000 - k) + k;
	return 1.00000 / (gV * gL);
}

float calcLightingGGX(float3 N, float3 V, float3 L, float roughness, float F0)
{
	float alpha = roughness * roughness;
	float3 H = normalize(V + L);
#ifdef OPENGL
	float dotNL = clamp(dot(N, L), 0.000000, 1.00000);
	float dotLH = clamp(dot(L, H), 0.000000, 1.00000);
	float dotNH = clamp(dot(N, H), 0.000000, 1.00000);
	float dotNV = clamp(dot(N, V), 0.000000, 1.00000);
#endif
#ifdef DIRECTX
	float dotNL = saturate(dot(N, L));
	float dotLH = saturate(dot(L, H));
	float dotNH = saturate(dot(N, H));
	float dotNV = saturate(dot(N, V));
#endif
	float D = calcD_GGX(roughness, dotNH);
	float F = calcF(F0, dotLH);
	float G = calcG_Schlick(roughness, dotNV, dotNL);

	D = min(32.0,D);

	return dotNL * D * F * G / 4.00000;
}

float3 calcAmbientColor(float3 upDir, float3 normal)
{
	float3 color = float3(0.000000, 0.000000, 0.000000);
	float NoU = dot(normal, upDir);
	color.xyz = color.xyz + skyLightColor * max(NoU + 1, 0.000000) / 2.00000;
	color.xyz = color.xyz + groundLightColor * max(-NoU + 1, 0.000000) / 2.00000;
	return color;
}

float3 calcDirectionalLightDiffuseColor(float3 diffuseColor, float3 normal, float3 lightDir, float shadow, float ao)
{
	float3 color = float3(0.000000, 0.000000, 0.000000);
	float NoL = dot(normal, lightDir);
	color.xyz = directionalLightColor * max(NoL, 0.000000) * shadow * ao / 3.14000;
	color.xyz = color.xyz * diffuseColor.xyz;
	return color;
}

float3 calcDirectionalLightSpecularColor(float3 specularColor, float3 normal, float3 lightDir, float smoothness, float fresnel, float shadow, float ao)
{
	float roughness = 1.00000 - smoothness;
	roughness = max(roughness, 0.08);
	roughness = min(roughness, 0.92);

	float3 viewDir = float3(0.000000, 0.000000, 1.00000);
	float3 specular;
	specular.x = calcLightingGGX(normal, viewDir, lightDir, roughness, specularColor.x);
	specular.y = calcLightingGGX(normal, viewDir, lightDir, roughness, specularColor.y);
	specular.z = calcLightingGGX(normal, viewDir, lightDir, roughness, specularColor.z);
	specular = specular * shadow * ao;
	specular.xyz = directionalLightColor * specular.xyz;
	float NoL = dot(normal, lightDir);
	specular.xyz = specular.xyz * max(NoL, 0.000000);
	return specular;
}

float VSM(float2 moments, float t)
{
	float ex = moments.x;
	float ex2 = moments.y;
	float p = 0.000000;
	if(t <= ex) p = 1.00000;
	float variance = ex2 - ex * ex;
	variance = max(variance, 2.00000e-005);
	float d = t - ex;
	float p_max = variance / (variance + d * d);
	return max(p, p_max);
}

float3 CalcDiffuseColor(float3 baseColor, float metalness)
{
	return baseColor * (1.00000 - metalness);
}

float3 CalcSpecularColor(float3 baseColor, float metalness)
{
	float3 minColor = float3(0.0400000, 0.0400000, 0.0400000);
	return minColor.xyz * (1.00000 - metalness) + baseColor.xyz * metalness;
}






//||>

float3 GetBaseColor(float2 uv)
{
	return g_gbuffer0Texture.Sample(g_gbuffer0Sampler, uv).xyz;
}

float4 GetSmoothnessMetalnessAO(float2 uv)
{
	return g_gbuffer1Texture.Sample(g_gbuffer1Sampler, uv).xyzw;
}

float3 GetNormal(float2 uv)
{
	return g_gbuffer2Texture.Sample(g_gbuffer2Sampler, uv).xyz;
}

float3 GetAO(float2 uv)
{
	float ao = g_ssaoTexture.Sample(g_ssaoSampler, uv).x;
	return float3(ao,ao,ao);
}

float GetNormalizedDepth(float2 uv)
{
	return g_gbuffer2Texture.Sample(g_gbuffer2Sampler, uv).w;
}

float ReconstructDepth(float z)
{
	return reconstructInfo1.x / (reconstructInfo1.y * z + reconstructInfo1.z);
	//return -((z * reconstructInfo1.x) + reconstructInfo1.y);
}

float3 ReconstructPosition(float2 screenXY, float depth)
{
	return float3( reconstructInfo2.xy * screenXY * (-depth), depth );
}

float4 ReconstructShadowmapPosition(float3 cameraPos)
{
	float4 shadowmapPos = mul( g_cameraPositionToShadowCameraPosition, float4(cameraPos,1.0) );
	shadowmapPos = shadowmapPos / shadowmapPos.w;
	return shadowmapPos;
}

float4 ReconstructProjectedShadowmapPosition(float4 shadowmapPosition)
{
	float4 shadowmapPos = mul( g_shadowProjection, shadowmapPosition );
	shadowmapPos = shadowmapPos / shadowmapPos.w;
	return shadowmapPos;
}

float2 GetShadowmapUV(float4 shadowmapPos)
{
	return float2( (shadowmapPos.x + 1.0) / 2.0f, 1.0 - (shadowmapPos.y + 1.0) / 2.0f );
}

float4 main( const PS_Input Input ) : SV_Target
{
	float2 uv = Input.UV;
	// uv.y = 1.0 - uv.y;

	float3 cameraPos = ReconstructPosition(Input.Position.xy, ReconstructDepth(GetNormalizedDepth(uv)));

	float4 lightColor = float4(0.0,0.0,0.0,1.0);

	float3 baseColor = GetBaseColor(uv);
	float3 normal = GetNormal(uv);
	float4 smoothnessMetalnessAO = GetSmoothnessMetalnessAO(uv);
	float smoothness = smoothnessMetalnessAO .x;
	float metalness = smoothnessMetalnessAO .y;
	float bakedAO = smoothnessMetalnessAO .z;

	float3 diffuseColor = CalcDiffuseColor(baseColor,metalness);
	float3 specularColor = CalcSpecularColor(baseColor,metalness);

	float ao = GetAO(uv).x * bakedAO;

#ifdef DIRECTIONAL_LIGHT
	float4 shadowmapPos = ReconstructShadowmapPosition(cameraPos);
	float4 projShadowmapPos = ReconstructProjectedShadowmapPosition(shadowmapPos);
	float2 shadowmapUV = GetShadowmapUV(projShadowmapPos);

	float depth = projShadowmapPos.z;
	float2 shadowParam = g_shadowmapTexture.Sample(g_shadowmapSampler, shadowmapUV).xy;

	float shadow = VSM(shadowParam, depth );

	lightColor.xyz += calcDirectionalLightDiffuseColor(diffuseColor, normal, directionalLightDirection, shadow, ao);

	lightColor.xyz += calcDirectionalLightSpecularColor(specularColor, normal, directionalLightDirection, smoothness, 0.06, shadow, ao);
#endif

#ifdef AMBIENT_LIGHT
	lightColor.xyz += calcAmbientColor(upDir, normal) * diffuseColor * ao;
	lightColor.xyz += g_environmentDiffuseTexture.Sample(g_environmentDiffuseSampler, uv).xyz * ao;
#endif

	return lightColor;
}

)";
