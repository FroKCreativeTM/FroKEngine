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

	shared_ptr<class Camera> GetMainCamera();

	// ���۰� ������Ʈ�� ���� ������ �� �� Render�� ����ȴ�.
	void Render();

	void ClearRTV();
	
	void RenderShadow();	// Light �������� ���ָ� �ȴ�.
	void RenderDeffered();
	void RenderLights();
	void RenderFinal();

	void RenderForward();

private : 
	// ������ ���ۿ� �ִ� ������ �ϴ� �޼����̴�.
	void PushLightData();

public: 
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);

	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }

	wstring GetSceneName() const { return _sceneName; }
	void SetSceneName(const wstring& str) { _sceneName = str; }

private:
	vector<shared_ptr<GameObject>> _gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;

	wstring _sceneName;
};

