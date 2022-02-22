#pragma once
class GameObject;

class Scene
{
public:
	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	// ���۰� ������Ʈ�� ���� ������ �� �� Render�� ����ȴ�.
	void Render();
	void RenderLights();
	void RenderFinal();

private : 
	// ������ ���ۿ� �ִ� ������ �ϴ� �޼����̴�.
	void PushLightData();

public: 
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);

	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }

private:
	vector<shared_ptr<GameObject>> _gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;
};

