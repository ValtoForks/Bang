#include "Bang/UILayoutManager.h"

#include "Bang/Map.h"
#include "Bang/Set.h"
#include "Bang/UMap.h"
#include "Bang/Scene.h"
#include "Bang/AARect.h"
#include "Bang/UICanvas.h"
#include "Bang/GameObject.h"
#include "Bang/Application.h"
#include "Bang/SceneManager.h"
#include "Bang/RectTransform.h"
#include "Bang/UILayoutIgnorer.h"
#include "Bang/IEventsTransform.h"
#include "Bang/ILayoutSelfController.h"

USING_NAMESPACE_BANG

UILayoutManager::UILayoutManager()
{
}

void UILayoutManager::PropagateInvalidation(ILayoutElement *element)
{
    Component *comp = DCAST<Component*>(element);
    GameObject *go = DCAST<GameObject*>(element);
    if (!go && comp)
    {
        go = comp->GetGameObject();
    }

    if (go)
    {
        const List<ILayoutController*> &layoutControllers =
                                       GetLayoutControllersIn(go->GetParent());
        for (ILayoutController *layoutController : layoutControllers)
        {
            if (!layoutController->IsSelfController())
            {
                layoutController->Invalidate();
            }
        }

        for (ILayoutController *layoutController : layoutControllers)
        {
            if (layoutController->IsSelfController())
            {
                layoutController->Invalidate();
            }
        }
    }
}

void UILayoutManager::PropagateInvalidation(ILayoutController *controller)
{
    Component *comp = DCAST<Component*>(controller);
    GameObject *go = DCAST<GameObject*>(controller);
    if (!go && comp)
    {
        go = comp->GetGameObject();
    }

    if (go)
    {
        ILayoutElement *lElm = comp ? DCAST<ILayoutElement*>(comp) : nullptr;
        if (!lElm)
        {
            lElm = go ? DCAST<ILayoutElement*>(go) : nullptr;
        }

        if (lElm)
        {
            lElm->Invalidate();
        }
    }
}

Vector2i UILayoutManager::GetMinSize(GameObject *go)
{
    return Vector2i( UILayoutManager::GetSize(go, LayoutSizeType::MIN) );
}

Vector2i UILayoutManager::GetPreferredSize(GameObject *go)
{
    return Vector2i( UILayoutManager::GetSize(go, LayoutSizeType::PREFERRED) );
}

Vector2 UILayoutManager::GetFlexibleSize(GameObject *go)
{
    return UILayoutManager::GetSize(go, LayoutSizeType::FLEXIBLE);
}

Vector2 UILayoutManager::GetSize(GameObject *go, LayoutSizeType sizeType)
{
    // Retrieve layout elements and their respective priority
    Map<int, List<ILayoutElement*> > priorLayoutElms;
    List<ILayoutElement*> les = go->GetComponents<ILayoutElement>();
    if (les.IsEmpty())
    {
        return Vector2::Zero;
    }

    for (ILayoutElement *le : les)
    {
        int prior = le->GetLayoutPriority();
        if (!priorLayoutElms.ContainsKey(prior))
        {
            priorLayoutElms.Add(prior, List<ILayoutElement*>());
        }
        priorLayoutElms.Get(prior).PushBack(le);
    }
    if (priorLayoutElms.IsEmpty())
    {
        return Vector2::Zero;
    }

    // Get the max size between the elements ordered by priority.
    // Sizes less than zero will be ignored.
    Vector2 size = Vector2(-1);
    bool sizeXFound = false, sizeYFound = false;
    for (auto it = priorLayoutElms.RBegin(); it != priorLayoutElms.REnd(); ++it)
    {
        const List<ILayoutElement*> &les = (*it).second;
        for (ILayoutElement *le : les)
        {
            Vector2 leSize =  le->GetSize(sizeType);
            if (!sizeXFound)
            {
                size.x = Math::Max(size.x, leSize.x);
            }

            if (!sizeYFound)
            {
                size.y = Math::Max(size.y, leSize.y);
            }
        }

        if (size.x >= 0)
        {
            sizeXFound = true;
        }

        if (size.y >= 0)
        {
            sizeYFound = true;
        }

        if (sizeXFound && sizeYFound)
        {
            break;
        }
    }

    return Vector2::Max(size, Vector2::Zero);
}

List<GameObject *> UILayoutManager::GetLayoutableChildrenList(GameObject *go)
{
    List<GameObject*> childrenList;
    for (GameObject *child : go->GetChildren())
    {
        UILayoutIgnorer *ltIgnorer = child->GetComponent<UILayoutIgnorer>();
        bool ignoreLayout = ltIgnorer ? ltIgnorer->IsIgnoreLayout() : false;
        if (child->IsEnabled() &&
            child->HasComponent<ILayoutElement>() &&
            !ignoreLayout)
        {
            childrenList.PushBack(child);
        }
    }
    return childrenList;
}

void UILayoutManager::RebuildLayout(GameObject *rootGo)
{
    if (rootGo)
    {
        CalculateLayout(rootGo, Axis::HORIZONTAL);
        ApplyLayout(rootGo, Axis::HORIZONTAL);
        CalculateLayout(rootGo, Axis::VERTICAL);
        ApplyLayout(rootGo, Axis::VERTICAL);
    }
}

void UILayoutManager::CalculateLayout(GameObject *gameObject, Axis axis)
{
    const List<GameObject*> &children = gameObject->GetChildren();
    for (GameObject *child : children)
    {
        CalculateLayout(child, axis);
    }

    const List<ILayoutElement*> &goLEs = GetLayoutElementsIn(gameObject);
    for (ILayoutElement *goLE : goLEs)
    {
        goLE->_CalculateLayout(axis);
    }
}

void UILayoutManager::ApplyLayout(GameObject *gameObject, Axis axis)
{
    std::queue<GameObject*> goQueue;
    goQueue.push(gameObject);
    while (!goQueue.empty())
    {
        GameObject *go = goQueue.front();
        goQueue.pop();

        const List<ILayoutController*> &layoutControllers = GetLayoutControllersIn(go);

        // SelfLayoutControllers
        for (ILayoutController *layoutController : layoutControllers)
        {
            if (layoutController->IsSelfController())
            {
                layoutController->_ApplyLayout(axis);
            }
        }

        // Normal LayoutControllers
        for (ILayoutController *layoutController : layoutControllers)
        {
            if (!layoutController->IsSelfController())
            {
                layoutController->_ApplyLayout(axis);
            }
        }

        const List<GameObject*> &children = go->GetChildren();
        for (GameObject *child : children)
        {
            goQueue.push(child);
        }
    }
}

void UILayoutManager::OnDestroyed(EventEmitter<IEventsDestroy> *object)
{
    m_iLayoutElementsPerGameObject.Remove( DCAST<GameObject*>(object) );
    m_iLayoutControllersPerGameObject.Remove( DCAST<GameObject*>(object) );
}

template <class T>
const List<T*> &GetGatheredListOf(
                        UILayoutManager *layoutMgr,
                        GameObject *gameObject,
                        UMap<GameObject*, ObjectGatherer<T, false>*> &gatherMap)
{
    if (gameObject)
    {
        auto it = gatherMap.Find(gameObject);
        if (it == gatherMap.End())
        {
            ObjectGatherer<T, false> *objGatherer = new ObjectGatherer<T, false>();
            objGatherer->SetRoot(gameObject);
            gatherMap.Add(gameObject, objGatherer);
            gameObject->EventEmitter<IEventsDestroy>::RegisterListener(layoutMgr);
            return gatherMap.Get(gameObject)->GetList();
        }
        else
        {
            return it->second->GetList();
        }
    }
    return List<T*>::Empty();
}

const List<ILayoutElement*> &UILayoutManager::
GetLayoutElementsIn(GameObject *gameObject)
{
    return GetGatheredListOf<ILayoutElement>(this,
                                             gameObject,
                                             m_iLayoutElementsPerGameObject);
}

const List<ILayoutController*> &UILayoutManager::
GetLayoutControllersIn(GameObject *gameObject)
{
    return GetGatheredListOf<ILayoutController>(
                                  this,
                                  gameObject,
                                  m_iLayoutControllersPerGameObject);
}

UILayoutManager *UILayoutManager::GetActive(GameObject *go)
{
    if (go)
    {
        UICanvas *canvas = UICanvas::GetActive(go);
        return canvas ? canvas->GetLayoutManager() : nullptr;
    }
    return nullptr;
}

UILayoutManager *UILayoutManager::GetActive(Component *comp)
{
    return comp ? UILayoutManager::GetActive(comp->GetGameObject()) : nullptr;
}

UILayoutManager *UILayoutManager::GetActive(ILayoutElement *layoutElement)
{
    if (Component *comp = DCAST<Component*>(layoutElement))
    {
        return UILayoutManager::GetActive(comp);
    }

    if (GameObject *go = DCAST<GameObject*>(layoutElement))
    {
        return UILayoutManager::GetActive(go);
    }

    return nullptr;
}

UILayoutManager *UILayoutManager::GetActive(ILayoutController *layoutController)
{
    if (Component *comp = DCAST<Component*>(layoutController))
    {
        return UILayoutManager::GetActive(comp);
    }

    if (GameObject *go = DCAST<GameObject*>(layoutController))
    {
        return UILayoutManager::GetActive(go);
    }

    return nullptr;
}

