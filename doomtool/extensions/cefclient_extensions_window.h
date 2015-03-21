#ifndef CEFCLIENT_EXTENSIONS_WINDOW_H_
#define CEFCLIENT_EXTENSIONS_WINDOW_H_

#include "client_handler.h"
#include "client_app.h"

namespace extensions_window {

// Delegate creation. Called from ClientHandler.
void CreateProcessMessageDelegates(
    ClientHandler::ProcessMessageDelegateSet& delegates);
// Create the render delegate.
void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates);

// Platform implementations.
void StartDrag(HWND hwnd);
void SetTopMost(HWND hwnd, bool on);
void MoveTo(HWND hwnd, int x, int y);
void MoveBy(HWND hwnd, int x, int y);
void ResizeTo(HWND hwnd, int width, int height);
void ResizeBy(HWND hwnd, int width, int height);
void Focus(HWND hwnd);
void Blur(HWND hwnd);
void Show(HWND hwnd);
void Hide(HWND hwnd);
void Maximize(HWND hwnd);
void Minimize(HWND hwnd);
void Restore(HWND hwnd);
}  // namespace extensions_window

#endif  // CEFCLIENT_EXTENSIONS_WINDOW_H_
