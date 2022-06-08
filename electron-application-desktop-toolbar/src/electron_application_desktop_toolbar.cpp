
#include <napi.h>
#include <iostream>
#include <string>

#ifdef _WIN32

#include "windows.h"
#define APPBAR_CALLBACK WM_USER + 0x01;

#endif

namespace addon
{

#ifdef _WIN32
  extern int g_uSide = ABE_TOP;
  extern bool g_fAppRegistered = false;
  
  int g_offset = 0;

  Napi::Function emit;
  Napi::FunctionReference f;

  // AppBarQuerySetPos - sets the size and position of an appbar.

  // uEdge - screen edge to which the appbar is to be anchored
  // lprc - current bounding rectangle of the appbar
  // pabd - address of the APPBARDATA structure with the hWnd and cbSize members
  // filled
  void PASCAL AppBarQuerySetPos(HWND hwndAccessBar, UINT uEdge, LPRECT lprc, PAPPBARDATA pabd)
  {

    int iHeight = 0;
    int iWidth = 0;

    Napi::Object obj = Napi::Object::New(f.Env());

    switch (uEdge)
    {
    case ABE_LEFT:
      obj.Set("side", "ABE_LEFT");
      break;

    case ABE_RIGHT:
      obj.Set("side", "ABE_RIGHT");
      break;

    case ABE_TOP:
      obj.Set("side", "ABE_TOP");
      break;

    case ABE_BOTTOM:
      obj.Set("side", "ABE_BOTTOM");
      break;
    }

    pabd->hWnd = hwndAccessBar;
    pabd->rc = *lprc;
    pabd->uEdge = uEdge;

    // Copy the screen coordinates of the appbar's bounding
    // rectangle into the APPBARDATA structure.
    if ((uEdge == ABE_LEFT) || (uEdge == ABE_RIGHT))
    {
      iWidth = pabd->rc.right - pabd->rc.left;
      pabd->rc.top = 0;
      pabd->rc.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
      iHeight = pabd->rc.bottom - pabd->rc.top;
      pabd->rc.left = g_offset;
      pabd->rc.right = 1;// GetSystemMetrics(SM_CXSCREEN);
    }

    // Query the system for an approved size and position.
    SHAppBarMessage(ABM_QUERYPOS, pabd);

    // Adjust the rectangle, depending on the edge to which the appbar is
    // anchored.
    switch (uEdge)
    {
    case ABE_LEFT:
      pabd->rc.right = pabd->rc.left + iWidth;
      break;

    case ABE_RIGHT:
      pabd->rc.left = pabd->rc.right - iWidth;
      break;

    case ABE_TOP:
      pabd->rc.bottom = pabd->rc.top + iHeight;
      break;

    case ABE_BOTTOM:
      pabd->rc.top = pabd->rc.bottom - iHeight;
      break;
    }

    // Pass the final bounding rectangle to the system.
    SHAppBarMessage(ABM_SETPOS, pabd);

    // Move and size the appbar so that it conforms to the
    // bounding rectangle passed to the system.
    Napi::Number left = Napi::Number::New(f.Env(), pabd->rc.left);
    Napi::Number top = Napi::Number::New(f.Env(), pabd->rc.top);
    Napi::Number width = Napi::Number::New(f.Env(), pabd->rc.right - pabd->rc.left);
    Napi::Number height = Napi::Number::New(f.Env(), pabd->rc.bottom - pabd->rc.top);
    Napi::Number side = Napi::Number::New(f.Env(), uEdge);

    // Assign values to propertieselectron-application-desktop-toolbar
    obj.Set("left", left);
    obj.Set("top", top);
    obj.Set("width", width);
    obj.Set("height", height);

    f.Call({Napi::String::New(f.Env(), "onMove"), obj});
  }

  // AppBarPosChanged - adjusts the appbar's size and position.

  // pabd - address of an APPBARDATA structure that contains information
  //        used to adjust the size and position.
  void PASCAL AppBarPosChanged(PAPPBARDATA pabd)
  {
    RECT rc;
    RECT rcWindow;
    int iHeight;
    int iWidth;

    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(pabd->hWnd, &rcWindow);

    iHeight = rcWindow.bottom - rcWindow.top;
    iWidth = rcWindow.right - rcWindow.left;

    switch (g_uSide)
    {
    case ABE_LEFT:
      rc.bottom = rc.top + iHeight;
      break;

    case ABE_BOTTOM:
      rc.top = rc.bottom - iHeight;
      break;

    case ABE_TOP:
      rc.right = rc.left + iWidth;
      break;

    case ABE_RIGHT:
      rc.left = rc.right - iWidth;
      break;
    }

    AppBarQuerySetPos(pabd->hWnd, g_uSide, &rc, pabd);
  }

  // AppBarCallback - processes notification messages sent by the system.

  // hwndAccessBar - handle to the appbar
  // uNotifyMsg - identifier of the notification message
  // lParam - message parameter
  void AppBarCallback(HWND hwndAccessBar, UINT uNotifyMsg, LPARAM lParam)
  {

    /*
    f.Call({Napi::String::New(f.Env(), "data"),
            Napi::String::New(f.Env(), "AppBarCallback")});
    */

    APPBARDATA abd;
    UINT uState;

    abd.cbSize = sizeof(abd);
    abd.hWnd = hwndAccessBar;

    switch (uNotifyMsg)
    {
    case ABN_STATECHANGE:

      // Check to see if the taskbar's always-on-top state has changed
      // and, if it has, change the appbar's state accordingly.
      uState = SHAppBarMessage(ABM_GETSTATE, &abd);

      SetWindowPos(hwndAccessBar,
                   (ABS_ALWAYSONTOP & uState) ? HWND_TOPMOST : HWND_BOTTOM, 0, 0,
                   0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

      break;

    case ABN_FULLSCREENAPP:

      // A full-screen application has started, or the last full-screen
      // application has closed. Set the appbar's z-order appropriately.
      if (lParam)
      {
        SetWindowPos(hwndAccessBar,
                     (ABS_ALWAYSONTOP & uState) ? HWND_TOPMOST : HWND_BOTTOM, 0,
                     0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }
      else
      {
        uState = SHAppBarMessage(ABM_GETSTATE, &abd);

        if (uState & ABS_ALWAYSONTOP)
          SetWindowPos(hwndAccessBar, HWND_TOPMOST, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }

    case ABN_POSCHANGED:

      // The taskbar or another appbar has changed its size or position.
      AppBarPosChanged(&abd);
      break;
    }
  }

  // RegisterAccessBar - registers or unregisters an appbar.
  // Returns TRUE if successful, or FALSE otherwise.

  // hwndAccessBar - handle to the appbar
  // fRegister - register and unregister flag

  // Global variables
  //     g_uSide - screen edge (defaults to ABE_LEFT)
  //     g_fAppRegistered - flag indicating whether the bar is registered
  BOOL RegisterAccessBar(HWND hwndAccessBar, BOOL fRegister)
  {
    APPBARDATA abd;

    // Specify the structure size and handle to the appbar.
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwndAccessBar;

    if (fRegister && g_fAppRegistered)
    {
      return true;
    }

    if (!fRegister && !g_fAppRegistered)
    {
      return true;
    }

    if (!fRegister && g_fAppRegistered)
    {
      // Unregister the appbar.
      SHAppBarMessage(ABM_REMOVE, &abd);
      g_fAppRegistered = FALSE;
    }

    if (fRegister && !g_fAppRegistered)
    {

      // Provide an identifier for notification messages.
      abd.uCallbackMessage = APPBAR_CALLBACK;

      // Register the appbar.
      if (!SHAppBarMessage(ABM_NEW, &abd))
        return FALSE;

      g_uSide = ABE_TOP; // default edge
      g_fAppRegistered = TRUE;
    }
  }

  void DockAccessBar(HWND hwndAccessBar, UINT edge, UINT windowSize)
  {
    APPBARDATA abd;
    RECT lprc;

    if (edge == ABE_LEFT)
    {
      lprc.top = 0;
      lprc.bottom = 0;
      lprc.left = 0;
      lprc.right = windowSize;
    }
    else if(edge == ABE_RIGHT)
    {
      lprc.top = 0;
      lprc.bottom = 0;
      lprc.left = GetSystemMetrics(SM_CXSCREEN) - windowSize;
      lprc.right = GetSystemMetrics(SM_CXSCREEN);
    }
    else if(edge == ABE_BOTTOM)
    {
      lprc.top = GetSystemMetrics(SM_CYSCREEN) - windowSize;
      lprc.bottom = GetSystemMetrics(SM_CYSCREEN);
      lprc.left = 0;
      lprc.right = 0;
    }
    else // ABE_TOP
    {
      lprc.top = 0;
      lprc.bottom = windowSize;
      lprc.left = 0;
      lprc.right = 0;
    }

    // Specify the structure size and handle to the appbar.
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwndAccessBar;
    abd.uCallbackMessage = APPBAR_CALLBACK;

    AppBarQuerySetPos(hwndAccessBar, edge, &lprc, &abd);

    return;
  }

  /* Public */

  Napi::Value Register(const Napi::CallbackInfo &info)
  {

    Napi::Env env = info.Env();

    if (info.Length() < 2)
    {
      Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[0].IsBuffer())
    {
      Napi::TypeError::New(env, "Wrong argument to handle").ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[1].IsFunction())
    {
      Napi::TypeError::New(env, "Wrong argument to events").ThrowAsJavaScriptException();
      return env.Null();
    }

    // save event emitter
    emit = info[1].As<Napi::Function>();
    f = Persistent(emit);

    Napi::Buffer<void *> handleBufffer = info[0].As<Napi::Buffer<void *> >();
    HWND handle =
        static_cast<HWND>(*reinterpret_cast<void **>(handleBufffer.Data()));

    BOOL r = RegisterAccessBar(handle, true);
    Napi::Boolean res = Napi::Boolean::New(env, r);

    return res;
  }

  Napi::Value Unregister(const Napi::CallbackInfo &info)
  {

    Napi::Env env = info.Env();

    if (info.Length() < 1)
    {
      Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[0].IsBuffer())
    {
      Napi::TypeError::New(env, "Wrong arguments 0").ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Buffer<void *> handleBufffer = info[0].As<Napi::Buffer<void *> >();
    HWND handle =
        static_cast<HWND>(*reinterpret_cast<void **>(handleBufffer.Data()));

    BOOL r = RegisterAccessBar(handle, false);
    Napi::Boolean res = Napi::Boolean::New(env, r);

    return res;
  }

  Napi::Value Dock(const Napi::CallbackInfo &info)
  {

    Napi::Env env = info.Env();

    if (info.Length() < 3)
    {
      Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[0].IsBuffer())
    {
      Napi::TypeError::New(env, "Wrong arguments 0").ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[1].IsBoolean())
    {
      Napi::TypeError::New(env, "Wrong arguments 1").ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[2].IsNumber())
    {
      Napi::TypeError::New(env, "Wrong arguments 2").ThrowAsJavaScriptException();
      return env.Null();
    }

    if (!info[3].IsNumber())
    {
      Napi::TypeError::New(env, "Wrong arguments 2").ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Buffer<void *> handleBufffer = info[0].As<Napi::Buffer<void *> >();
    Napi::Boolean side = info[1].As<Napi::Boolean>();
    Napi::Number windowSize = info[2].As<Napi::Number>();
	g_offset = info[3].As<Napi::Number>();

    UINT edge = ABE_BOTTOM;

    if (side)
    {
      edge = ABE_TOP;
    }

    HWND handle =
        static_cast<HWND>(*reinterpret_cast<void **>(handleBufffer.Data()));

    DockAccessBar(handle, edge, windowSize);

    /*
    f.Call({Napi::String::New(f.Env(), "data"),
            Napi::String::New(f.Env(), "Dock done")});
    */

    Napi::Number res = Napi::Number::New(env, edge);

    return res;
  }

#else
  Napi::Value Register(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    Napi::Boolean res = Napi::Boolean::New(env, true);
    return res;
  }
  Napi::Value Unregister(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    Napi::Boolean res = Napi::Boolean::New(env, true);
    return res;
  }
  Napi::Value Dock(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    Napi::Boolean res = Napi::Boolean::New(env, true);
    return res;
  }
#endif

  Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    exports.Set(Napi::String::New(env, "register"),
                Napi::Function::New(env, Register));
    exports.Set(Napi::String::New(env, "unregister"),
                Napi::Function::New(env, Unregister));
    exports.Set(Napi::String::New(env, "dock"), Napi::Function::New(env, Dock));
    return exports;
  }

  NODE_API_MODULE(electron_application_desktop_toolbar, Init)

} // namespace addon