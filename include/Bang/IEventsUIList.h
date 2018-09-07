#ifndef IEVENTSUILIST_H
#define IEVENTSUILIST_H

#include "Bang/IEvents.h"

NAMESPACE_BANG_BEGIN

FORWARD class GameObject;

class IEventsUIList
{
    IEVENTS(IEventsUIList);

public:
    virtual void OnItemAdded(GameObject *item, int index)
    {
        BANG_UNUSED_2(item, index);
    }

    virtual void OnItemMoved(GameObject *item, int oldIndex, int newIndex)
    {
        BANG_UNUSED_3(item, oldIndex, newIndex);
    }

    virtual void OnItemRemoved(GameObject *item)
    {
        BANG_UNUSED(item);
    }
};

NAMESPACE_BANG_END

#endif // IEVENTSUILIST_H

