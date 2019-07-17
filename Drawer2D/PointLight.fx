matrix World;
matrix View;
matrix Projection;

float4 LightColor;
float4 LightDirection;
float4 LightPosition;

struct VS_INPUT
{
	float4 Pos: POSITION;
	float3 Normal: NORMAL;
	float4 Color: COLOR;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float4 Color: COLOR;
	float3 InterPos: TEXTCOORD0;
	float3 Normal: TEXTCOORD1;
};

PS_INPUT VS(VS_INPUT Data)
{
	PS_INPUT Out;
	Out = (PS_INPUT)0;
	
	Out.Pos = mul(Data.Pos, World);
	
	Out.InterPos = Out.Pos.xyz;
	
	Out.Pos = mul(Out.Pos, View);
	Out.Pos = mul(Out.Pos, Projection);
	
	Out.Color = Data.Color;
	Out.Normal = mul( Data.Normal, World);
	
	return Out;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return float4(1,1,1,1);
}

float4 PS_Color(PS_INPUT input) : SV_Target
{
	return input.Color;
}

float4 PS_DirectLight( PS_INPUT Vertex ) : SV_Target
{
	float4 FinalColor =
		saturate( ((Vertex.Color * LightColor)/1) * dot( (float3)LightDirection, Vertex.Normal ) );
	FinalColor.a = 1;
	
	return FinalColor;
}

float4 PS_PointLight( PS_INPUT Vertex ) : SV_Target
{
	float3 PixelToLight = (float3)LightPosition - Vertex.InterPos;
	float3 NewNormal = normalize(Vertex.Normal);
	float3 NewDirection = normalize(PixelToLight);
	float4 LightIntensity = LightColor/pow(length(PixelToLight), 2.0);
	float4 FinalColor = saturate((LightIntensity) * dot (NewNormal, NewDirection) );
	FinalColor.a = 1;
	return FinalColor;
}

RasterizerState rsNoCulling{ CullMode = None; };
technique10 RenderWhite
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
		SetRasterizerState( rsNoCulling );
	}
}

technique10 RenderColor
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_Color() ) );
		SetRasterizerState( rsNoCulling );
	}
}

technique10 RenderDirectLight
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_DirectLight() ) );
		//SetRasterizerState( rsNoCulling );
	}
}

technique10 RenderPointLight
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_PointLight() ) );
		//SetRasterizerState( rsNoCulling );
	}
}