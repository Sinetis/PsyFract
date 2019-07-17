matrix World;
matrix View;
matrix Projection;

struct VS_INPUT
{
	float4 Pos: POSITION;
	float4 Color: COLOR;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float4 Color: COLOR;
};

PS_INPUT VS(VS_INPUT Data)
{
	PS_INPUT Out;
	Out = (PS_INPUT)0;
	
	Out.Pos = mul(Data.Pos, World);
	Out.Pos = mul(Out.Pos, View);
	Out.Pos = mul(Out.Pos, Projection);
	
	Out.Color = Data.Color;
	
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