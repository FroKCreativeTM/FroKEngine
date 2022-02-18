#pragma once

class Shader;
class Texture;

enum
{
	MATERIAL_INT_COUNT = 5,
	MATERIAL_FLOAT_COUNT = 5,
	MATERIAL_TEXTURE_COUNT = 5,
};

struct MaterialParams
{
	void SetInt(uint8 index, int32 value) { intParams[index] = value; }
	void SetFloat(uint8 index, float value) { floatParams[index] = value; }

	array<int32, MATERIAL_INT_COUNT> intParams;
	array<float, MATERIAL_FLOAT_COUNT> floatParams;
};

// 엄청난 큰 게임을 만든다 가정하자.
// 그럼 우리는 굉장히 많은 양의 메시와 셰이더
// 그리고 텍스처를 생성해야 한다.
// 이는 굉장히 비효율적이고 update 등의 연산이 굉장히 귀찮아진다는 단점이 있다.
// 그래서 이 셰이더, 그리고 텍스처를 가지고 있으며 이를 활용할 수 있는 클래스를 만들었는데
// 이것이 바로 머터리얼이다.
// 참고로 텍스처는 하나만 넣는 식이 아니라
// 다양한 맵 등을 넣어줄 수 있다.
class Material
{
public:
	shared_ptr<Shader> GetShader() { return _shader; }

	void SetShader(shared_ptr<Shader> shader) { _shader = shader; }
	void SetInt(uint8 index, int32 value) { _params.SetInt(index, value); }
	void SetFloat(uint8 index, float value) { _params.SetFloat(index, value); }
	void SetTexture(uint8 index, shared_ptr<Texture> texture) { _textures[index] = texture; }

	void PushData();

private:
	shared_ptr<Shader>	_shader;
	MaterialParams		_params;
	array<shared_ptr<Texture>, MATERIAL_TEXTURE_COUNT> _textures;
};

