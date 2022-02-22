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

	// 시작과 업데이트가 전부 끝나고 난 뒤 Render가 실행된다.
	void Render();
	void RenderLights();
	void RenderFinal();

private : 
	// 조명을 버퍼에 넣는 역할을 하는 메서드이다.
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

