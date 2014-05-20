static const char* deferred_buffer_ps_gl = R"(
#version 330

uniform sampler2D		g_gbuffer0Texture;

uniform sampler2D		g_gbuffer1Texture;

uniform sampler2D		g_gbuffer2Texture;

uniform sampler2D		g_gbuffer3Texture;

uniform sampler2D		g_shadowmapTexture;

uniform sampler2D		g_ssaoTexture;

in vec4 voutPosition;
in vec2 voutUV;

out vec4 outOutput0;

vec3 GetNormal(vec2 uv)
{
	return texture2D(g_gbuffer2Texture, uv).xyz;
}

void main()
{
	vec2 uv = voutUV;
	uv.y = 1.0 - uv.y;

	vec4 color = vec4(0.0,0.0,0.0,1.0);

	color.xyz = GetNormal(uv);

	outOutput0 = color;
}

)";