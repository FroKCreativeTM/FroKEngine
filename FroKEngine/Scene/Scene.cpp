#include "Scene.h"
#include "Layer.h"
#include "../Object/Object.h"

unordered_map<string, Object*> Scene::m_mapPrototype[SC_END];

Scene::Scene()
{
    // ����� �����ϸ� �ƹ� ���̾ ������
    // �ϳ��� ��������.
    Layer* pLayer = CreateLayer("Default", 1);
    pLayer = CreateLayer("HUD", INT_MAX - 1);
    pLayer = CreateLayer("UI", INT_MAX); // �� �ڿ� �ִ� ��
    pLayer = CreateLayer("Stage", 0); // �� �ڿ� �ִ� ��
    m_eSceneType = SC_CURRENT;
}

Scene::~Scene()
{
    ErasePrototype(m_eSceneType);
    Safe_Delete_VecList(m_LayerList);
}

// ���ο� ���̾� ����
Layer* Scene::CreateLayer(const string& strTag, int nZOrder)
{
    Layer* pLayer = new Layer();

    // �� ��鿡 ���� ���ο� ���̾� ������ ä���.
    pLayer->SetTag(strTag);
    pLayer->SetScene(this);

    // ����Ʈ�� ���̾ �ִ´�.
    m_LayerList.push_back(pLayer);

    // ����Ʈ�� 2���� ������ Z������ �°� �����Ѵ�.
    // (��������)
    if (m_LayerList.size() >= 2)
    {
        // �Լ� ��ü�� bool type�� template Ÿ���� �Լ� �����Ͱ� ����.
        // �� �� ȣ����(caller)�� �����Լ� �����͸� �޴´�.
        m_LayerList.sort(Scene::LayerSort);
    }

    return nullptr;
}

// ���̾� ã��
// @parameter
// strTag : ã������ ���̾��� �±�
Layer* Scene::FindLayer(const string& strTag)
{
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd; ++iter)
    {
        if ((*iter)->GetTag() == strTag)
        {
            return *iter;
        }
    }
    return nullptr;
}

bool Scene::LayerSort(Layer* pL1, Layer* pL2)
{
    // zOrder�� ���� �������� ������ �� ���̴�.
    // = ���� crash ������ �����ϱ� ����
    return pL1->GetZOrder() < pL2->GetZOrder();
}

bool Scene::Init()
{
    return true;
}

void Scene::Input(float fDeltaTime)
{
    // ���̾� ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd;)
    {// ��Ȱ��ȭ��
        if (!(*iter)->GetEnable())
        {
            ++iter;
            continue;
        }

        (*iter)->Input(fDeltaTime);

        // ���� �� ���̾ �׾��ٸ�
        if (!(*iter)->GetLife())
        {
            SAFE_DELETE((*iter));
            iter = m_LayerList.erase(iter);
            iterEnd = m_LayerList.end();
        }
        else
        {
            ++iter;
        }
    }
}

int Scene::Update(float fDeltaTime)
{
    // ���̾� ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd;)
    {
        // ��Ȱ��ȭ��
        if (!(*iter)->GetEnable())
        {
            ++iter;
            continue;
        }

        (*iter)->Update(fDeltaTime);

        // ���� �� ���̾ �׾��ٸ�
        if (!(*iter)->GetLife())
        {
            SAFE_DELETE((*iter));
            iter = m_LayerList.erase(iter);
            iterEnd = m_LayerList.end();
        }
        else
        {
            ++iter;
        }
    }

    return 0;
}

int Scene::LateUpdate(float fDeltaTime)
{
    // ���̾� ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd;)
    {// ��Ȱ��ȭ��
        if (!(*iter)->GetEnable())
        {
            ++iter;
            continue;
        }

        (*iter)->LateUpdate(fDeltaTime);

        // ���� �� ���̾ �׾��ٸ�
        if (!(*iter)->GetLife())
        {
            SAFE_DELETE((*iter));
            iter = m_LayerList.erase(iter);
            iterEnd = m_LayerList.end();
        }
        else
        {
            ++iter;
        }
    }

    return 0;
}

void Scene::Collision(float fDeltaTime)
{
    // ���̾� ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd;)
    {// ��Ȱ��ȭ��
        if (!(*iter)->GetEnable())
        {
            ++iter;
            continue;
        }

        (*iter)->Collision(fDeltaTime);

        // ���� �� ���̾ �׾��ٸ�
        if (!(*iter)->GetLife())
        {
            SAFE_DELETE((*iter));
            iter = m_LayerList.erase(iter);
            iterEnd = m_LayerList.end();
        }
        else
        {
            ++iter;
        }
    }
}

void Scene::Render(float fDeltaTime)
{
    // ���̾� ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
    list<Layer*>::iterator iter;
    list<Layer*>::iterator iterEnd = m_LayerList.end();

    for (iter = m_LayerList.begin(); iter != iterEnd;)
    {// ��Ȱ��ȭ��
        if (!(*iter)->GetEnable())
        {
            ++iter;
            continue;
        }

        (*iter)->Render(fDeltaTime);

        // ���� �� ���̾ �׾��ٸ�
        if (!(*iter)->GetLife())
        {
            SAFE_DELETE((*iter));
            iter = m_LayerList.erase(iter);
            iterEnd = m_LayerList.end();
        }
        else
        {
            ++iter;
        }
    }
}

void Scene::ErasePrototype(SCENE_CREATE sc)
{
    // ��ü ������Ÿ���� ������ ������.
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