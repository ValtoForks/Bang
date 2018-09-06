#include "Bang/MetaFilesManager.h"

#include "Bang/Set.h"
#include "Bang/File.h"
#include "Bang/List.h"
#include "Bang/USet.h"
#include "Bang/Debug.h"
#include "Bang/Paths.h"
#include "Bang/MetaNode.h"
#include "Bang/Resources.h"
#include "Bang/Application.h"
#include "Bang/ResourceHandle.h"

#include "yaml-cpp/yaml.h"

USING_NAMESPACE_BANG

MetaFilesManager::MetaFilesManager()
{
}

MetaFilesManager::~MetaFilesManager()
{
}

void MetaFilesManager::CreateMissingMetaFiles(const Path &directory)
{
    // First load existing meta files, to avoid creating new meta files
    // with duplicated GUIDs.
    MetaFilesManager::LoadMetaFilepathGUIDs(directory);

    Array<Path> assetFiles = directory.GetFiles(Path::FindFlag::RECURSIVE);

    USet<Path> files;
    files.Add(assetFiles.Begin(), assetFiles.End());

    for (const Path &filepath : files)
    {
        if (!IsMetaFile(filepath) && !HasMetaFile(filepath))
        {
            MetaFilesManager::CreateMetaFileIfMissing(filepath);
        }
    }
}

void MetaFilesManager::LoadMetaFilepathGUIDs(const Path &directory)
{
    Array<String> extensions = {GetMetaExtension()};
    Array<Path> metaFilepaths = directory.GetFiles(Path::FindFlag::RECURSIVE_HIDDEN,
                                                   extensions);

    // Remove alone .meta files
    for (const Path &metaFilepath : metaFilepaths)
    {
        if ( IsMetaFile(metaFilepath) &&
            !GetFilepath(metaFilepath).IsFile() )
        {
            File::Remove(metaFilepath);
        }
    }

    // Load GUID's of meta files!
    for (const Path &metaFilepath : metaFilepaths)
    {
        RegisterMetaFilepath(metaFilepath);
    }
}

std::pair<Path, GUID> MetaFilesManager::CreateMetaFileIfMissing(const Path &filepath)
{
    Path metaFilepath = GetMetaFilepath(filepath);
    GUID newGUID = GUID::Empty();
    if ( !IsMetaFile(filepath) && !HasMetaFile(filepath) )
    {
        MetaNode metaNode;
        newGUID = GUIDManager::GetNewGUID();
        metaNode.Set("GUID", newGUID);
        File::Write(metaFilepath, metaNode.ToString());
        MetaFilesManager::RegisterMetaFilepath(metaFilepath);
    }
    else
    {
        newGUID = GetGUID(filepath);
    }
    return std::make_pair(metaFilepath, newGUID);
}

bool MetaFilesManager::HasMetaFile(const Path &filepath)
{
    return GetMetaFilepath(filepath).Exists();
}

bool MetaFilesManager::IsMetaFile(const Path &filepath)
{
    return filepath.IsHiddenFile() &&
           filepath.HasExtension( GetMetaExtension() );
}

void MetaFilesManager::DuplicateMetaFile(const Path &filepath,
                                         const Path &dupFilepath)
{
    const Path dupMetaFilepath = GetMetaFilepath(dupFilepath);
    File::Remove(dupMetaFilepath);

    const GUID& newGUID = CreateMetaFileIfMissing(dupFilepath).second;
    const Path originalMetaFilepath = GetMetaFilepath(filepath);
    MetaNode originalMetaNode;
    originalMetaNode.Import(originalMetaFilepath);
    originalMetaNode.Set("GUID", newGUID);

    File::Write(dupMetaFilepath, originalMetaNode.ToString());
    RegisterMetaFilepath(dupMetaFilepath);
}

GUIDManager* MetaFilesManager::GetGUIDManager()
{
    return &(MetaFilesManager::GetInstance()->m_GUIDManager);
}

void MetaFilesManager::RegisterMetaFilepath(const Path &metaFilepath)
{
    if (IsMetaFile(metaFilepath))
    {
        MetaNode metaNode;
        metaNode.Import(metaFilepath);

        Path filepath = GetFilepath(metaFilepath);
        GUID guid = metaNode.Get<GUID>("GUID");
        if (!guid.IsEmpty())
        {
            MetaFilesManager *ifm = MetaFilesManager::GetInstance();
            if (ifm->m_GUIDToFilepath.ContainsKey(guid) &&
                ifm->m_GUIDToFilepath.Get(guid) != filepath)
            {
                Debug_Error("Found conflicting GUID: " << guid <<
                            "(Files '" << filepath << "' and '" <<
                            ifm->m_GUIDToFilepath.Get(guid) << "'");
            }

            ifm->m_GUIDToFilepath.Add(guid, filepath);
            ifm->m_filepathToGUID.Add(filepath, guid);
        }
    }
}

void MetaFilesManager::UnRegisterMetaFilepath(const Path &metaFilepath)
{
    MetaFilesManager *ifm = MetaFilesManager::GetInstance();
    Path filepath = GetFilepath(metaFilepath);
    ifm->m_GUIDToFilepath.RemoveValues(filepath);
    ifm->m_filepathToGUID.Remove(filepath);
}

GUID MetaFilesManager::GetGUID(const Path& filepath)
{
    MetaFilesManager *ifm = MetaFilesManager::GetInstance();
    if (ifm->m_filepathToGUID.ContainsKey(filepath))
    {
        return ifm->m_filepathToGUID.Get(filepath);
    }
    else
    {
        if ( !Resources::IsEmbeddedResource(filepath) )
        {
            Path metaFilepath = GetMetaFilepath(filepath);
            if (metaFilepath.IsFile())
            {
                MetaNode metaNode;
                metaNode.Import(metaFilepath);
                GUID guid = metaNode.Get<GUID>("GUID");
                ifm->m_filepathToGUID.Add(filepath, guid);
                return guid;
            }
        }
        else
        {
            Path parentResPath = filepath.GetDirectory();
            Resource *parentRes = Resources::GetCached(parentResPath);
            if (parentRes)
            {
                if (Resource *embeddedRes = parentRes->GetEmbeddedResource(
                                                        filepath.GetNameExt()))
                {
                    return embeddedRes->GetGUID();
                }
            }
        }
    }
    return GUID::Empty();
}

Path MetaFilesManager::GetFilepath(const GUID &guid)
{
    MetaFilesManager *ifm = MetaFilesManager::GetInstance();
    if (ifm->m_GUIDToFilepath.ContainsKey(guid))
    {
        Path path = ifm->m_GUIDToFilepath.Get(guid);
        return path;
    }
    else
    {
        if (Resources::IsEmbeddedResource(guid))
        {
            Path parentPath = MetaFilesManager::GetFilepath(
                                    guid.WithoutEmbeddedResourceGUID());
            if (parentPath.IsFile())
            {
                RH<Resource> resRH = Resources::LoadFromExtension(parentPath);
                if (resRH)
                {
                    String name = resRH.Get()->GetEmbeddedResourceName(
                                                guid.GetEmbeddedResourceGUID() );
                    return parentPath.Append(name);
                }
            }
        }
    }
    return Path::Empty;
}

Path MetaFilesManager::GetFilepath(const Path &metaFilepath)
{
    Path filepath = metaFilepath.WithHidden(false);

    String strPath = filepath.GetAbsolute();
    if (strPath.BeginsWith("."))
    {
        strPath.Remove(0, 1);
    }

    String ending = "." + GetMetaExtension();
    if (strPath.EndsWith(ending))
    {
        strPath.Remove(strPath.Size() - ending.Size(), strPath.Size());
    }

    return Path(strPath);
}

Path MetaFilesManager::GetMetaFilepath(const Path &filepath)
{
    return filepath.AppendExtension(
                MetaFilesManager::GetMetaExtension() ).WithHidden(true);
}

Path MetaFilesManager::GetMetaFilepath(const GUID &guid)
{
    return MetaFilesManager::GetMetaFilepath(
                                    MetaFilesManager::GetFilepath(guid) );
}

void MetaFilesManager::OnFilepathRenamed(const Path &oldPath,
                                         const Path &newPath)
{
    Path oldMetaFilepath = MetaFilesManager::GetMetaFilepath(oldPath);
    MetaFilesManager::UnRegisterMetaFilepath(oldMetaFilepath);

    Path newMetaFilepath = MetaFilesManager::GetMetaFilepath(newPath);

    File::Rename(oldMetaFilepath, newMetaFilepath);
    MetaFilesManager::RegisterMetaFilepath(newMetaFilepath);
}

MetaFilesManager *MetaFilesManager::GetInstance()
{
    return Application::GetInstance()->GetMetaFilesManager();
}

String MetaFilesManager::GetMetaExtension()
{
    return "meta";
}
