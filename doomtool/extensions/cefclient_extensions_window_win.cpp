#include "cefclient_extensions_window.h"

namespace extensions_window {

namespace {

// Toggles the current display state.
void Toggle(HWND hwnd, UINT nCmdShow) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
  WINDOWPLACEMENT placement;
  ::GetWindowPlacement(root_wnd, &placement);

  if (placement.showCmd == nCmdShow)
    ::ShowWindow(root_wnd, SW_RESTORE);
  else
    ::ShowWindow(root_wnd, nCmdShow);
}

}  // namespace

void StartDrag(HWND hwnd) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	WINDOWPLACEMENT placement;
	::GetWindowPlacement(root_wnd, &placement);
	if(placement.showCmd == SW_MAXIMIZE)
		return;

	::ReleaseCapture();
	::SendMessage(root_wnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
	::SendMessage(root_wnd, WM_LBUTTONUP, NULL, NULL);
}

void SetTopMost(HWND hwnd, bool on) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	::SetWindowPos(root_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void MoveTo(HWND hwnd, int x, int y) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
  // Retrieve current window placement information.
  WINDOWPLACEMENT placement;
  ::GetWindowPlacement(root_wnd, &placement);
  if(placement.showCmd == SW_MAXIMIZE)
	  return;
  else if (placement.showCmd == SW_MINIMIZE)
    ::ShowWindow(root_wnd, SW_RESTORE);
  ::SetWindowPos(root_wnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

}
void MoveBy(HWND hwnd, int x, int y) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	RECT rect = {0};
	if (::GetWindowRect(root_wnd, &rect)) {
		MoveTo(hwnd, rect.left+x, rect.top+y);
	}
}

void ResizeTo(HWND hwnd, int width, int height) {
	 HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);

  // Retrieve current window placement information.
  WINDOWPLACEMENT placement;
  ::GetWindowPlacement(root_wnd, &placement);

  if(placement.showCmd == SW_MAXIMIZE)
	  return;

  HMONITOR monitor = MonitorFromRect(&placement.rcNormalPosition,
                                     MONITOR_DEFAULTTONEAREST);
  MONITORINFO info;
  info.cbSize = sizeof(info);
  GetMonitorInfo(monitor, &info);

  if (width < 100)
    width = 100;
  else if (width > info.rcWork.right - info.rcWork.left)
	  width = info.rcWork.right - info.rcWork.left;
  if (height < 100)
	  height = 100;
  else if (height > info.rcWork.bottom - info.rcWork.top)
    height = info.rcWork.bottom - info.rcWork.top;

  ::SetWindowPos(root_wnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
  if (placement.showCmd == SW_MINIMIZE)
    ::ShowWindow(root_wnd, SW_RESTORE);
}

void ResizeBy(HWND hwnd, int width, int height) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	RECT rect = {0};
	if (::GetWindowRect(root_wnd, &rect)) {
		ResizeTo(root_wnd, rect.right-rect.left+width, rect.bottom-rect.top+height);
	}
}

void Focus(HWND hwnd) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	::SetFocus(root_wnd);
}

void Blur(HWND hwnd) {
	::SetFocus(NULL);
}

void Show(HWND hwnd) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	::ShowWindow(root_wnd, SW_SHOW);
}

void Hide(HWND hwnd) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	::ShowWindow(root_wnd, SW_HIDE);
}

void Maximize(HWND hwnd) {
	Toggle(hwnd, SW_MAXIMIZE);
}

void Minimize(HWND hwnd) {
	Toggle(hwnd, SW_MINIMIZE);
}

void Restore(HWND hwnd) {
	HWND root_wnd = ::GetAncestor(hwnd, GA_ROOT);
	::ShowWindow(root_wnd, SW_RESTORE);
}

}  // namespace extensions_window
