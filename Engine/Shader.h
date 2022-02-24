#pragma once
#include "Object.h"

enum class SHADER_TYPE : uint8
{
	DEFERRED,
	FORWARD,
	LIGHTING,
	COMPUTE,
};

enum class RASTERIZER_TYPE : uint8
{
	CULL_NONE,		// 무시하지 않고 전부 연산
	CULL_FRONT,		// 시계 방향 무시
	CULL_BACK,		// 반시계 방향 무시
	WIREFRAME,
};

enum class DEPTH_STENCIL_TYPE : uint8
{
	LESS,					// 절두체 상 1까지 범위면 1보다 작아야 카메라에 찍히는 식
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	NO_DEPTH_TEST,				// 깊이 테스트(X) + 깊이 기록(O)
	NO_DEPTH_TEST_NO_WRITE,		// 깊이 테스트(X) + 깊이 기록(X)
	LESS_NO_WRITE				// 깊이 테스트(O) + 깊이 기록(X)
};

enum class BLEND_TYPE : uint8
{
	DEFAULT,
	ALPHA_BLEND,
	ONE_TO_ONE_BLEND,
	END,
};

struct ShaderInfo
{
	SHADER_TYPE shaderType = SHADER_TYPE::FORWARD;
	RASTERIZER_TYPE rasterizeType = RASTERIZER_TYPE::CULL_BACK;
	DEPTH_STENCIL_TYPE depthStencilType = DEPTH_STENCIL_TYPE::LESS;
	BLEND_TYPE blendType = BLEND_TYPE::DEFAULT;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

// [일감 기술서] 외주 인력들이 뭘 해야할지 기술
class Shader : public Object
{
public:
	Shader();
	virtual ~Shader();

	void CreateGraphicsShader(const wstring& path, ShaderInfo info = ShaderInfo(), const string& vs = "VS_Main", const string& ps = "PS_Main");
	void CreateComputeShader(const wstring& path, const string name , const string version);
	void Update();

	SHADER_TYPE GetShaderType() { return _info.shaderType; }

private:
	void CreateShader(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob, D3D12_SHADER_BYTECODE& shaderByteCode);
	void CreateVertexShader(const wstring& path, const string& name, const string& version);
	void CreatePixelShader(const wstring& path, const string& name, const string& version);

private:
	ShaderInfo _info;
	ComPtr<ID3D12PipelineState>			_pipelineState;

private : 
	// Graphics Shader
	ComPtr<ID3DBlob>					_vsBlob;
	ComPtr<ID3DBlob>					_psBlob;
	ComPtr<ID3DBlob>					_errBlob;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC  _graphicsPipelineDesc = {};

	ComPtr<ID3DBlob>					_csBlob;
	D3D12_COMPUTE_PIPELINE_STATE_DESC  _computePipelineDesc = {};
};

