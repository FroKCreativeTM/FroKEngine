#include "Scene.h"
#include "../Object/Object.h"

unordered_map<string, Object*> Scene::m_mapPrototype[SC_END];

Scene::Scene()
{
    m_eSceneType = SC_CURRENT;
}

Scene::~Scene()
{
    ErasePrototype(m_eSceneType);
}

bool Scene::Init()
{
    return true;
}

void Scene::Input(float fDeltaTime)
{
}

int Scene::Update(float fDeltaTime)
{

    return 0;
}

int Scene::LateUpdate(float fDeltaTime)
{
    return 0;
}

void Scene::Collision(float fDeltaTime)
{
}

void Scene::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
}

void Scene::ErasePrototype(SCENE_CREATE sc)
{
    // 전체 프로토타입을 통으로 날린다.
    Safe_Release_Map(m_mapPrototype[sc]);
}

void Scene::ErasePrototype(const string& strTag, SCENE_CREATE sc)
{
    unordered_map<string, Object*>::iterator iter
        = m_mapPrototype[sc].find(strTag);

    if (!iter->second)
    {
        return;
    }

    SAFE_RELEASE(iter->second);
    m_mapPrototype[sc].erase(iter);
}

Object* Scene::FindPrototype(const string& strTag, SCENE_CREATE sc)
{
    unordered_map<string, Object*>::iterator iter = m_mapPrototype[sc].find(strTag);

    if (iter == m_mapPrototype[sc].end())
    {
        return nullptr;
    }

    return iter->second;
}

void Scene::ChangePrototype()
{
    ErasePrototype(SC_CURRENT);
    m_mapPrototype[SC_CURRENT] = m_mapPrototype[SC_NEXT];
    m_mapPrototype[SC_NEXT].clear();
}

void Scene::OnResize()
{
}
