cbuffer WorldBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

struct VertexInput
{
	float3 Position : POSITION;
	float4 Colour : COLOUR;
};

struct PixelInput
{
	float3 Position : POSITION;
	float4 PositionH : SV_POSITION;
	float4 Colour : COLOUR;
};