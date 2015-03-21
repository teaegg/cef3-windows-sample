// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include "client_app.h"
#include "cefclient_osr_widget_win.h"
#include "resource.h"

CefRefPtr<ClientHandler> g_handler;

class MainBrowserProvider : public OSRBrowserProvider {
	virtual CefRefPtr<CefBrowser> GetBrowser(HWND hWnd) {
		if (g_handler.get())
			return g_handler->GetBrowser(hWnd);

		return NULL;
	}
} g_main_browser_provider;

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow) {
		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);
		//MessageBoxA(NULL, "main", NULL, 0);
		// Provide CEF with command-line arguments.
		CefMainArgs main_args(hInstance);

		// SimpleApp implements application-level callbacks. It will create the first
		// browser instance in OnContextInitialized() after CEF has initialized.
		CefRefPtr<ClientApp> app(new ClientApp);

		// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
		// that share the same executable. This function checks the command-line and,
		// if this is a sub-process, executes the appropriate logic.
		int exit_code = CefExecuteProcess(main_args, app.get());
		if (exit_code >= 0) {
			// The sub-process has completed so return here.
			return exit_code;
		}

		// Try to open the mutex.
		HANDLE hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, 0, L"DoomToolInstance");

		// If hMutex is 0 then the mutex doesn't exist.
		if (!hMutex) {
			hMutex = ::CreateMutex(0, 0, L"DoomToolInstance");
		} else {
			// This is a second instance. Bring the 
			// original instance to the top.
			HWND hWnd;
			hWnd = ::FindWindow(L"DoomToolWnd", NULL);
			if(hWnd) {
				::SetForegroundWindow(hWnd);
				::ShowWindow(hWnd, SW_RESTORE);
			} else {
				hWnd = ::FindWindow(L"DoomToolPopUpWnd", NULL);
				if(hWnd)
					::PostMessage(hWnd ,ID_OPENMAINWINDOW, 0, 0);
			}
			return 0;
		}

		// Specify CEF global settings here.
		CefSettings settings;
		CefString(&settings.browser_subprocess_path).FromASCII("G:/project/doomtool/build/Release/doomtool.exe");
		// Initialize CEF.
		CefInitialize(main_args, settings, app.get());

		// Create the single static handler class instance
		g_handler = new ClientHandler();

		CefWindowInfo info;
		CefBrowserSettings browser_settings;
		

		CefRefPtr<OSRWindow> osr_window = OSRWindow::Create(hInstance, &g_main_browser_provider, TRUE);
		
		g_handler->SetOSRHandler(osr_window.get());

		RECT rect;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION);
		rect.left = 0;
		rect.top = 0;

		HWND hWnd = osr_window->CreateWidget(NULL, rect, L"DoomTool", L"DoomToolWnd");
		CefWindowInfo info2;
		info2.SetAsOffScreen(hWnd);
		info2.SetTransparentPainting(TRUE);
		CefBrowserHost::CreateBrowser(info2, g_handler.get(),
			g_handler->GetStartupURL(), browser_settings, NULL);
		
		// Run the CEF message loop. This will block until CefQuitMessageLoop() is
		// called.
		CefRunMessageLoop();

		// Shut down CEF.
		CefShutdown();

		ReleaseMutex(hMutex);

		return 0;
}
