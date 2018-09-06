#ifndef WINDOW_H
#define WINDOW_H

#include <stack>

#include "Bang/List.h"
#include "Bang/IEvents.h"
#include "Bang/Vector2.h"
#include "Bang/EventEmitter.h"
#include "Bang/EventListener.h"

FORWARD union  SDL_Event;
FORWARD struct SDL_Window;
using SDL_GLContext = void*;

NAMESPACE_BANG_BEGIN

FORWARD class Time;
FORWARD class Input;
FORWARD class Window;
FORWARD class GEngine;
FORWARD class Resources;
FORWARD class Texture2D;
FORWARD class SceneManager;

class IEventsWindow
{
    IEVENTS(IEventsWindow)
public:
    virtual void OnFocusGained(Window *w) { (void) w; }
    virtual void OnFocusLost(Window *w) { (void) w; }
};

class Window : public EventEmitter<IEventsWindow>
{
public:
    Window();
    virtual ~Window();

    virtual void Create(uint flags);
    void SwapBuffers() const;
    bool MainLoopIteration();
    void MakeCurrent();

    virtual void Update();
    virtual void Render();
    virtual void OnResize(int newWidth, int newHeight);

    virtual bool HandleEvent(const SDL_Event &sdlEvent);
    void OnHandleEventsFinished();
    virtual bool OnClosed();

    void Maximize();
    void Restore();
    void Minimize();
    void MoveToFront();
    void SetIcon(const Path &iconPath);
    void SetBordered(bool bordered);
    void SetMinSize(int minSizeX, int minSizeY);
    void SetMaxSize(int maxSizeX, int maxSizeY);
    void SetResizable(bool resizable);
    void SetPosition(int newPosX, int newPosY);
    void SetTitle(const String &title);
    void SetSize(int newWidth, int newHeight);
    static void SetActive(Window *window);

    int GetWidth() const;
    int GetHeight() const;
    bool HasFocus() const;
    bool IsBordered() const;
    String GetTitle() const;
    bool IsMouseOver() const;
    Vector2i GetSize() const;
    Vector2i GetMinSize() const;
    Vector2i GetMaxSize() const;
    bool IsResizable() const;
    Vector2i GetPosition() const;
    Vector2i GetInsidePosition() const;
    bool IsBlockedByChildren() const;
    bool HasFlags(uint flags) const;
    int GetGLMajorVersion() const;
    int GetGLMinorVersion() const;

    static float GetAspectRatioS();
    static int GetHeightS();
    static int GetWidthS();

    SDL_GLContext   GetGLContext() const;
    Input          *GetInput() const;
    SceneManager   *GetSceneManager() const;

    SDL_Window *GetSDLWindow() const;
    uint GetSDLWindowID() const;
    Window *GetParentWindow() const;
    const List<Window*>& GetChildren() const;

    static Window *GetActive();

protected:
    void SetParent(Window *parentWindow);

private:
    SDL_Window *m_sdlWindow = nullptr;

    List<Window*> p_children;
    Window* p_parent = nullptr;

    SDL_GLContext   m_sdlGLContext  = nullptr;
    Input          *m_input         = nullptr;
    SceneManager   *m_sceneManager  = nullptr;

    Vector2i m_minSize = Vector2i::Zero;
    Vector2i m_maxSize = Vector2i(4096);
    Vector2i m_prevSize = Vector2i::Zero;
    Vector2i m_newSize  = Vector2i::Zero;
    bool m_isResizable = true, m_resizableChanged = false;

    static Window* s_activeWindow;

    void RetrieveTitleBarHeight();

    void _SetMinSize(int minSizeX, int minSizeY);
    void _SetMaxSize(int maxSizeX, int maxSizeY);
    Vector2i _GetMinSize() const;
    Vector2i _GetMaxSize() const;
    bool IsParentWindow(int sdlWindowId) const;

    virtual SceneManager *CreateSceneManager() const;

    friend class Application;
    friend int EventFilter(void *userData, SDL_Event *event);
};

NAMESPACE_BANG_END

#endif // IWINDOW_H
