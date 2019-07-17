matrix World;
matrix View;
matrix Projection;

float4 LightColor;
float4 LightDirection;
float4 LightPosition;

float Phi;
float Theta;

struct VS_INPUT
{
	float4 Pos: POSITION;
	float4 Color: COLOR;
};

struct GSPS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float4 Color: COLOR;
	float3 InterPos: TEXTCOORD0;
	float3 Normal: TEXTCOORD1;
};

GSPS_INPUT VS(VS_INPUT Data)
{
	GSPS_INPUT Out;
	Out = (GSPS_INPUT)0;
	
	Out.Pos = mul(Data.Pos, World);
	Out.Color = Data.Color;
	
	return Out;
}

[MaxVertexCount(6)]
void GS(triangle GSPS_INPUT input[3],
		inout TriangleStream<GSPS_INPUT> TriStream)
{
	GSPS_INPUT output;
	
	for (int i = 0; i < 3; i++)
	{
		output.Pos = input[i].Pos;
		output.Pos = mul( output.Pos, View );
		output.Pos = mul( output.Pos, Projection );
		output.Color = input[i].Color;
		
		TriStream.Append( output );
	}
	TriStream.RestartStrip();
	
	float3 faceEdgeA = input[1].Pos - input[0].Pos;
	float3 faceEdgeB = input[2].Pos - input[0].Pos;
	float3 faceNormal = normalize( cross(faceEdgeA, faceEdgeB) );
	
	for (int i = 0; i < 3; i++)
	{
		output.Pos = input[i].Pos + float4(1.2*faceNormal, 0);
		output.Pos = mul( output.Pos, View );
		output.Pos = mul( output.Pos, Projection );
		output.Color = float4( 1,1,1,1 );
		
		TriStream.Append( output );
	}
	TriStream.RestartStrip();
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
	float4 LightIntensity = LightColor/pow(length(PixelToLight), 1.0);
	float4 FinalColor = saturate(Vertex.Color * LightIntensity * dot (NewNormal, NewDirection) );
	FinalColor.a = 1;
	return FinalColor;
}

float GetAngleIntensity(float3 LightDir, float3 PixelToLight)
{
	float F = 1;
	float AngleIntensity = 0;
	float CosAlpha = saturate(dot(LightDir, -PixelToLight));
	float ct = cos(Theta/2);
	if (CosAlpha > ct) AngleIntensity = 1.0;
	else 
	{
		float cp = cos (Phi/2);
		if (CosAlpha < cp) AngleIntensity = 0;
		else AngleIntensity = saturate(
			pow((CosAlpha - cp)/(ct-cp),F) );
	}
	return AngleIntensity;
}

float4 PS_SpotLight( PS_INPUT Vertex ) : SV_Target
{
	float A = 0.02f, B = 0, C = 0;
	
	float3 PixelToLight = (float3)LightPosition - Vertex.InterPos;
	float l = length(PixelToLight);
	float3 NewNormal = normalize(Vertex.Normal);
	float3 NewDirection = normalize(PixelToLight);
	
	float LightAtten = 1/(A*l*l + B*l + C);
	float LightAngle = GetAngleIntensity(normalize((float3)LightDirection), NewDirection);
	
	float4 LightIntensity = (LightColor*LightAngle*LightAtten + Vertex.Color)/2;
	float4 FinalColor = saturate(LightIntensity * dot(NewNormal, NewDirection));
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

technique10 RenderGeometric
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( CompileShader( gs_4_0, GS() );
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

technique10 RenderSpotLight
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_SpotLight() ) );
		//SetRasterizerState( rsNoCulling );
	}
}