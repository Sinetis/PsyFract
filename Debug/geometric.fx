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
	int i;
	for (i = 0; i < 3; i++)
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
	
	for (i = 0; i < 3; i++)
	{
		output.Pos = input[i].Pos + float4(0.6*faceNormal, 0);
		output.Pos = mul( output.Pos, View );
		output.Pos = mul( output.Pos, Projection );
		output.Color = float4( 1,1,1,1 );
		
		TriStream.Append( output );
	}
	TriStream.RestartStrip();
}

float4 PS_Color(GSPS_INPUT input) : SV_Target
{
	return input.Color;
}

RasterizerState rsNoCulling{ CullMode = None; };
technique10 RenderGeometric
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( CompileShader( gs_4_0, GS() ) );
		SetPixelShader( CompileShader( ps_4_0, PS_Color() ) );
		SetRasterizerState( rsNoCulling );
	}
}
