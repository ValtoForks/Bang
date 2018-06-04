#include "Bang/Path.h"

#include <ctime>
#include <cstdio>

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
constexpr char Separator[] = "/";
constexpr char SeparatorC = '/';
#elif _WIN32
#include <windows.h>
#include "Shlwapi.h"
#include "Bang/WinUndef.h"
constexpr char Separator[] = "\\";
constexpr char SeparatorC = '\\';
#endif

#include "Bang/List.h"
#include "Bang/Paths.h"
#include "Bang/Array.h"
#include "Bang/Debug.h"

USING_NAMESPACE_BANG

const Path Path::Empty;

Path::Path()
{
}

Path::Path(const Path &path) : Path(path.GetAbsolute())
{
}

Path::Path(const String &absolutePath)
{
    SetPath(absolutePath);
}

void Path::SetPath(const String &path)
{
    m_absolutePath = path;
    if (!m_absolutePath.IsEmpty() &&
         m_absolutePath.At( SCAST<int>(m_absolutePath.Size()) - 1 ) == 
         SeparatorC)
    {
        m_absolutePath.Remove(SCAST<int>(m_absolutePath.Size()) - 1,
                              SCAST<int>(m_absolutePath.Size()));
    }

    if (m_absolutePath.BeginsWith("./"))
    {
        m_absolutePath.Remove(0, 1);
    }
}

bool Path::IsDir() const
{
    if (!Exists()) { return false; }
    return !IsFile();
}

bool Path::IsFile() const
{
    if (!Exists()) { return false; }

    #ifdef __linux__
    struct stat path_stat;
    stat(GetAbsolute().ToCString(), &path_stat);
    return S_ISREG(path_stat.st_mode);
    #elif _WIN32
    return !PathIsDirectory( GetAbsolute().ToCString() );
    #endif
}

bool Path::Exists() const
{
    #ifdef __linux__
    return access(GetAbsolute().ToCString(), F_OK) != -1;
    #elif _WIN32
    return PathFileExists( GetAbsolute().ToCString() );
    #endif
}

List<Path> Path::GetFiles(Path::FindFlags findFlags,
                           const Array<String> &extensions) const
{
    if (!IsDir()) { return {}; }

    if (!findFlags.IsOn(Path::FindFlag::Recursive))
    {
        List<Path> subFilesList;
        List<Path> subPathsList = GetSubPaths(findFlags);
        for (const Path &subPath : subPathsList)
        {
            bool extMatches = extensions.IsEmpty() ||
                              subPath.HasExtension(extensions);
            if ( subPath.IsFile() && extMatches )
            {
                subFilesList.PushBack(subPath);
            }
        }
        return subFilesList;
    }
    else
    {
        List<Path> filesList;
        Path::FindFlags noRecursive = findFlags;
        noRecursive.SetOff(Path::FindFlag::Recursive);
        filesList.PushBack( GetFiles(noRecursive, extensions) );

        List<Path> subDirs = GetSubDirectories(findFlags);
        for (const Path &subdir : subDirs)
        {
            List<Path> subFilesList = subdir.GetFiles(findFlags, extensions);
            filesList.PushBack(subFilesList);
        }
        return filesList;
    }
}

List<Path> Path::GetSubDirectories(Path::FindFlags findFlags) const
{
    if (!IsDir()) { return {}; }

    List<Path> subDirsList;
    if (findFlags.IsOff(Path::FindFlag::Recursive))
    {
        List<Path> subPathsList = GetSubPaths(findFlags);
        for (const Path &subPath : subPathsList)
        {
            if (subPath.IsDir()) { subDirsList.PushBack(subPath); }
        }
    }
    else
    {
        Path::FindFlags noRecursive = findFlags;
        noRecursive.SetOff(Path::FindFlag::Recursive);
        subDirsList = GetSubDirectories(noRecursive);
        for (Path subdirPath : subDirsList)
        {
            List<Path> subdirsListRecursive =
                    subdirPath.GetSubDirectories(findFlags);
            subDirsList.Splice(subDirsList.End(), subdirsListRecursive);
        }
    }
    return subDirsList;
}

List<Path> Path::GetSubPaths(Path::FindFlags findFlags) const
{
    List<Path> subPathsList;

    #ifdef __linux__

    struct dirent *dir;
    DIR *d = opendir(GetAbsolute().ToCString());
    if (!d) { return {}; }

    while ((dir = readdir(d)) != nullptr)
    {
        String subName(dir->d_name);
        if (findFlags.IsOff(Path::FindFlag::Hidden) &&
            (subName.BeginsWith("."))) { continue; }

        if (subName != "." && subName != "..")
        {
            Path subPath = this->Append(subName);
            subPathsList.PushBack(subPath);
        }
    }
    closedir(d);

    #elif _WIN32

    char fullpath[MAX_PATH];
    GetFullPathName(GetAbsolute().ToCString(), MAX_PATH, fullpath, 0);
    std::string fp(fullpath);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((LPCSTR)(GetAbsolute() +
                                  String(Separator + String("*"))).ToCString(),
                   &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            String fileName(findFileData.cFileName);
            if (fileName != "." && fileName != "..")
            {
                subPathsList.PushBack( Append(fileName) );
            }
        }
        while (FindNextFile(hFind, &findFileData) != 0);
    }

    #endif

    return subPathsList;
}

uint64_t Path::GetModificationTimeSeconds() const
{
    if (!Exists()) { return 0; }

    #ifdef __linux__
    struct stat attr;
    stat(GetAbsolute().ToCString(), &attr);
    return attr.st_mtim.tv_sec;
    #elif _WIN32
    struct stat fileStat;
    stat(GetAbsolute().ToCString(), &fileStat);
    return SCAST<uint64_t>(fileStat.st_mtime);
    /*
    HANDLE hFile = CreateFile(GetAbsolute().ToCString(), GENERIC_READ, 
                              FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, 0, NULL);
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    return GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);
    return ftWrite.dwHighDateTime
    */
    #endif
}

String Path::GetName() const
{
    String name = "";
    bool iteratingFirstDots = true; // Treat hidden files "....foo.txt.bang"
    const String nameExt = GetNameExt();
    for (std::size_t i = 0; i < nameExt.Size(); ++i)
    {
        if (nameExt[i] != '.') { iteratingFirstDots = false; }
        if (iteratingFirstDots) { name += nameExt[i]; }
        else if (nameExt[i] != '.') { name += nameExt[i]; }
        else { break; }
    }
    return name;
}

bool Path::IsAbsolute() const
{
	#ifdef __linux__
    
	return GetAbsolute().BeginsWith(Separator);
	
	#elif _WIN32
	
	return 
		GetAbsolute().Size() >= 3 &&
		( (GetAbsolute()[0] >= 'A' && GetAbsolute()[0] <= 'Z') ||
	      (GetAbsolute()[0] >= 'a' && GetAbsolute()[0] <= 'z') ) &&
		GetAbsolute()[1] <= ':' &&
		GetAbsolute()[2] == SeparatorC;

	#endif
}

String Path::GetNameExt() const
{
    if (IsEmpty()) { return ""; }

    String filename = GetAbsolute();
    const size_t lastSlash = GetAbsolute().RFind(SeparatorC);
    if (lastSlash != String::npos)
    {
        filename = GetAbsolute().SubString(lastSlash + 1);
    }
    return filename;
}

String Path::GetExtension() const
{
    if (IsEmpty()) { return ""; }
    List<String> parts = GetNameExt().Split<List>('.');
    if (parts.IsEmpty()) { return ""; }
    parts.PopFront();
    return String::Join(parts, ".");
}

Array<String> Path::GetExtensions() const
{
    if (IsEmpty()) { return {}; }
    List<String> parts = GetNameExt().Split<List>('.');
    if (!parts.IsEmpty()) { parts.PopFront(); }
    return parts.To<Array>();
}

Path Path::GetDirectory() const
{
    if (IsEmpty()) { return Path::Empty; }

    const size_t lastSlash = GetAbsolute().RFind(SeparatorC);
    if (lastSlash != String::npos)
    {
        return Path(GetAbsolute().SubString(0, lastSlash-1));
    }
    return Path(".");
}

const String &Path::GetAbsolute() const
{
    return m_absolutePath;
}

Path Path::GetDuplicatePath() const
{
    if (IsEmpty()) { return Path::Empty; }

    Path resultPath(*this);
    while (resultPath.Exists())
    {
        resultPath = Path::GetNextDuplicatePath(resultPath);
    }
    return resultPath;
}

String Path::ToString() const
{
    return GetAbsolute();
}

bool Path::IsEmpty() const
{
    return GetAbsolute().IsEmpty();
}

bool Path::BeginsWith(const Path &path) const
{
    return BeginsWith(path.GetAbsolute());
}

bool Path::BeginsWith(const String &path) const
{
    return GetAbsolute().BeginsWith(path);
}

Path Path::Append(const Path &pathRHS) const
{
    String str = pathRHS.GetAbsolute();
    if (str.BeginsWith( "." + String(Separator) )) { str.Remove(0, 1); }
    while (str.BeginsWith(Separator)) { str.Remove(0, 1); }
    return this->AppendRaw(Separator + str);
}

Path Path::Append(const String &str) const
{
    return Path(GetAbsolute()).Append( Path(str) );
}

Path Path::AppendRaw(const String &str) const
{
    return Path(GetAbsolute() + str);
}

Path Path::AppendExtension(const String &extension) const
{
    if (HasExtension(extension) || extension.IsEmpty()) { return Path(*this); }
    return Path(GetAbsolute() + "." + extension);
}

bool Path::IsHiddenFile() const
{
    return IsFile() && GetName().BeginsWith(".");
}

Path Path::WithHidden(bool hidden) const
{
    String nameExt = GetNameExt();
    if ( hidden && !nameExt.BeginsWith(".")) { nameExt.Insert(0, "."); }
    if (!hidden &&  nameExt.BeginsWith(".")) { nameExt.Remove(0, 1); }
    return GetDirectory().Append(nameExt);
}

Path Path::WithNameExt(const String &name, const String &extension) const
{
    return GetDirectory().Append(name).AppendExtension(extension);
}

Path Path::WithExtension(const String &extension) const
{
    return Path( GetDirectory().Append(GetName())
                               .AppendExtension(extension) );
}

bool Path::HasExtension(const String &extensions) const
{
    Array<String> extensionsList = extensions.Split<Array>(' ', true);
    return HasExtension(extensionsList);
}

bool Path::HasExtension(const Array<String> &extensions) const
{
    Array<String> thisExtensions = GetExtensions();
    for (const String &extension : extensions)
    {
        for (const String &thisExtension : thisExtensions)
        {
            if (extension.EqualsNoCase(thisExtension)) { return true; }
        }
    }
    return false;
}

Path::operator String() const
{
    return GetAbsolute();
}

bool Path::operator!=(const Path &rhs) const
{
    return !(*this == rhs);
}

bool Path::operator==(const Path &rhs) const
{
    return GetAbsolute() == rhs.GetAbsolute();
}

bool Path::operator<(const Path &rhs) const
{
    return GetAbsolute() < rhs.GetAbsolute();
}

Path Path::GetNextDuplicatePath(const Path &filepath)
{
    if (filepath.IsEmpty()) { return Path::Empty; }

    Path fileDir         = filepath.GetDirectory();
    String fileName      = filepath.GetName();
    String fileExtension = filepath.GetExtension();

    Array<String> splitted = fileName.Split<Array>('_');
    int number = 1;
    if (splitted.Size() > 1)
    {
        String numberString = splitted[splitted.Size() - 1];
        bool ok = false;
        int readNumber = String::ToInt(numberString, &ok);
        if (ok)
        {
            number = readNumber + 1;
            splitted.PopBack();

            int lastUnderscorePos = SCAST<int>(fileName.RFind('_'));
            if (lastUnderscorePos != -1) // Strip _[number] from fileName
            {
                fileName = fileName.SubString(0, lastUnderscorePos-1);
            }
        }
    }

    Path result = fileDir.Append(fileName + "_" + String::ToString(number))
                         .AppendExtension(fileExtension);
    return result;
}

Path Path::EmptyPath() { return Path(); }
