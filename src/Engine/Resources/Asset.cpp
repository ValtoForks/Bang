#include "Bang/Asset.h"

#include "Bang/Paths.h"
#include "Bang/Resources.h"

USING_NAMESPACE_BANG

String Asset::ToString() const
{
    String res = GetClassName();
    res +=  "Asset(" + String::ToString((void*)this) + ")";
    return res;
}

void Asset::ImportXML(const XMLNode &xmlInfo)
{
    Resource::ImportXML(xmlInfo);
}

void Asset::ExportXML(XMLNode *xmlInfo) const
{
    Resource::ExportXML(xmlInfo);
}
