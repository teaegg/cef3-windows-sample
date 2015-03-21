// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_WINDOW_TEST_H_
#define CEF_TESTS_CEFCLIENT_WINDOW_TEST_H_
#pragma once

#include "client_handler.h"
#include "client_app.h"

namespace window_test {

// Delegate creation. Called from ClientHandler.
void CreateProcessMessageDelegates(
    ClientHandler::ProcessMessageDelegateSet& delegates);
// Create the render delegate.
void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates);

// Fit |window| inside |display|. Coordinates are relative to the upper-left
// corner of the display.
void ModifyBounds(const CefRect& display, CefRect& window);

// Forward declarations.
void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target);
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target);

// Platform implementations.
void SetPos(CefWindowHandle handle, int x, int y, int width, int height);
void Minimize(CefWindowHandle handle);
void Maximize(CefWindowHandle handle);
void Restore(CefWindowHandle handle);

}  // namespace window_test

#endif  // CEF_TESTS_CEFCLIENT_WINDOW_TEST_H_
