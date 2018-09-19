#ifndef COMPONENT_CLASS_IDS_H
#define COMPONENT_CLASS_IDS_H

#include "Bang/FastDynamicCast.h"

NAMESPACE_BANG_BEGIN

// Component class ids for FastDynamicCast
#define SET_COMPONENT_IDS(CLASS, ID_BEGIN, ID_END) \
    constexpr ClassIdType CLASS##CIDBegin = ID_BEGIN; \
    constexpr ClassIdType CLASS##CIDEnd   = ID_END
#define SET_COMPONENT_ID(CLASS, ID) \
    SET_COMPONENT_IDS(CLASS, ID, ID)

SET_COMPONENT_IDS(Component, 0, 10000000);
    SET_COMPONENT_IDS(Renderer, 1, 100);
        SET_COMPONENT_IDS(MeshRenderer, 10, 20);
            SET_COMPONENT_IDS(SkinnedMeshRenderer, 11, 15);
        SET_COMPONENT_IDS(LineRenderer, 21, 30);
        SET_COMPONENT_IDS(UIRenderer,   31, 40);
            SET_COMPONENT_IDS(UIImageRenderer, 32, 35);
            SET_COMPONENT_IDS(UITextRenderer,  36, 39);
        SET_COMPONENT_IDS(WaterRenderer, 41, 50);
    SET_COMPONENT_IDS(Animator,           101, 200);
    SET_COMPONENT_IDS(AudioListener,      201, 300);
    SET_COMPONENT_IDS(AudioSource,        301, 400);
    SET_COMPONENT_IDS(BehaviourContainer, 401, 500);
    SET_COMPONENT_IDS(Behaviour,          501, 600);
    SET_COMPONENT_IDS(Collider,           601, 700);
        SET_COMPONENT_IDS(BoxCollider,       602, 610);
        SET_COMPONENT_IDS(SphereCollider,    611, 620);
        SET_COMPONENT_IDS(CapsuleCollider,   621, 630);
    SET_COMPONENT_IDS(Light,              701, 800);
        SET_COMPONENT_IDS(DirectionalLight,  702, 710);
        SET_COMPONENT_IDS(PointLight,        711, 720);
    SET_COMPONENT_IDS(PostProcessEffect,   801, 900);
        SET_COMPONENT_IDS(PostProcessEffectSSAO, 802, 810);
    SET_COMPONENT_IDS(Transform, 901, 1000);
        SET_COMPONENT_IDS(RectTransform, 910, 920);
    SET_COMPONENT_IDS(RigidBody,                    1001, 1100);
    SET_COMPONENT_IDS(ReflectionProbe,              1101, 1200);
    SET_COMPONENT_IDS(UIAspectRatioFitter,          1201, 1300);
    SET_COMPONENT_IDS(UIAutoFocuser,                1301, 1400);
    SET_COMPONENT_IDS(UIButtonBase,                 1401, 1500);
        SET_COMPONENT_IDS(UIButton,                     1410, 1420);
        SET_COMPONENT_IDS(UIToolButton,                 1421, 1430);
    SET_COMPONENT_IDS(UICanvas,                     1601, 1700);
    SET_COMPONENT_IDS(UICheckBox,                   1701, 1800);
    SET_COMPONENT_IDS(UIComboBox,                   1801, 1900);
    SET_COMPONENT_IDS(UIContentSizeFitter,          1901, 2000);
    SET_COMPONENT_IDS(UIDirLayout,                  2001, 2100);
    SET_COMPONENT_IDS(UIDirLayoutMovableSeparator,  2101, 2200);
    SET_COMPONENT_IDS(UIDragDroppable,              2201, 2300);
    SET_COMPONENT_IDS(UIFileList,                   2301, 2400);
    SET_COMPONENT_IDS(UIFocusable,                  2401, 2500);
    SET_COMPONENT_IDS(UIGridLayout,                 2501, 2600);
    SET_COMPONENT_IDS(UIGroupLayout,                2601, 2700);
    SET_COMPONENT_IDS(UIHorizontalLayout,           2701, 2800);
    SET_COMPONENT_IDS(UIInputNumber,                2801, 2900);
    SET_COMPONENT_IDS(UIInputText,                  2901, 3000);
    SET_COMPONENT_IDS(UILabel,                      3001, 3100);
    SET_COMPONENT_IDS(UILayoutElement,              3101, 3200);
    SET_COMPONENT_IDS(UILayoutIgnorer,              3201, 3300);
    SET_COMPONENT_IDS(UIList,                       3301, 3400);
    SET_COMPONENT_IDS(UIMask,                       3401, 3500);
    SET_COMPONENT_IDS(UIRectMask,                   3501, 3600);
    SET_COMPONENT_IDS(UIRendererCacher,             3601, 3700);
    SET_COMPONENT_IDS(UIScrollArea,                 3701, 3800);
    SET_COMPONENT_IDS(UIScrollBar,                  3801, 3900);
    SET_COMPONENT_IDS(UIScrollPanel,                3901, 4000);
    SET_COMPONENT_IDS(UISlider,                     4001, 4100);
    SET_COMPONENT_IDS(UITextCursor,                 4101, 4200);
    SET_COMPONENT_IDS(UITree,                       4301, 4400);
    SET_COMPONENT_IDS(UIVerticalLayout,             4401, 4500);
    SET_COMPONENT_IDS(Camera,                       4501, 4600);

NAMESPACE_BANG_END

#endif // COMPONENT_CLASS_IDS_H
