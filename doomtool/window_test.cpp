// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "window_test.h"
#include "resource_util.h"

namespace window_test {

namespace {

const char* kMessagePositionName = "WindowTest.Position";
const char* kMessageMinimizeName = "WindowTest.Minimize";
const char* kMessageMaximizeName = "WindowMaximize";
const char* kMessageRestoreName = "WindowTest.Restore";

// Handle messages in the browser process.
class ProcessMessageDelegate : public ClientHandler::ProcessMessageDelegate {
 public:
  ProcessMessageDelegate() {
  }

  // From ClientHandler::ProcessMessageDelegate.
  virtual bool OnProcessMessageReceived(
      CefRefPtr<ClientHandler> handler,
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE {
    std::string message_name = message->GetName();
    if (message_name == kMessagePositionName) {
      CefRefPtr<CefListValue> args = message->GetArgumentList();
      if (args->GetSize() >= 4) {
        int x = args->GetInt(0);
        int y = args->GetInt(1);
        int width = args->GetInt(2);
        int height = args->GetInt(3);
        SetPos(browser->GetHost()->GetWindowHandle(), x, y, width, height);
      }
      return true;
    } else if (message_name == kMessageMinimizeName) {
      Minimize(browser->GetHost()->GetWindowHandle());
      return true;
    } else if (message_name == kMessageMaximizeName) {
      Maximize(browser->GetHost()->GetWindowHandle());
      return true;
    } else if (message_name == kMessageRestoreName) {
      Restore(browser->GetHost()->GetWindowHandle());
      return true;
    }

    return false;
  }

  IMPLEMENT_REFCOUNTING(ProcessMessageDelegate);
};

// Handles the native implementation for the client_app extension.
class WindowExtensionHandler : public CefV8Handler {
 public:
  WindowExtensionHandler(){
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) {
    bool handled = false;

    if (name == "sendMessage") {
      // Send a message to the browser process.
      if ((arguments.size() == 1 || arguments.size() == 2) &&
          arguments[0]->IsString()) {
        CefRefPtr<CefBrowser> browser =
            CefV8Context::GetCurrentContext()->GetBrowser();
        ASSERT(browser.get());

        CefString name = arguments[0]->GetStringValue();
        if (!name.empty()) {
          CefRefPtr<CefProcessMessage> message =
              CefProcessMessage::Create(name);
          // Translate the arguments, if any.
          if (arguments.size() == 2 && arguments[1]->IsArray())
            SetList(arguments[1], message->GetArgumentList());

          browser->SendProcessMessage(PID_BROWSER, message);
          handled = true;
        }
      }
	} else if (name == "WindowMaximize") {
		if (arguments.size() == 0) {
			CefRefPtr<CefBrowser> browser =
            CefV8Context::GetCurrentContext()->GetBrowser();
			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(name);
			browser->SendProcessMessage(PID_BROWSER, message);
			handled = true;
		}
	}
    if (!handled)
      exception = "Invalid method arguments";

    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(ClientAppExtensionHandler);
};

class WindowRenderDelegate : public ClientApp::RenderDelegate {
 public:
  WindowRenderDelegate() {
  }

  virtual void OnWebKitInitialized(CefRefPtr<ClientApp> app) {
	  // Register the client_app extension.
	  std::string window_code;
	  if(!LoadBinaryResource("window.js", window_code))
		  ::MessageBox(NULL,L"LOAD ARROR!", 0,0);
	  CefRegisterExtension("v8/app", window_code,
		  new WindowExtensionHandler());
  }

  virtual bool OnProcessMessageReceived(
      CefRefPtr<ClientApp> app,
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE {
    return false;
  }

 private:
  IMPLEMENT_REFCOUNTING(DOMRenderDelegate);
};

}  // namespace

void CreateProcessMessageDelegates(
    ClientHandler::ProcessMessageDelegateSet& delegates) {
  delegates.insert(new ProcessMessageDelegate);
}
void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates) {
  delegates.insert(new WindowRenderDelegate);
}
void ModifyBounds(const CefRect& display, CefRect& window) {
  window.x += display.x;
  window.y += display.y;

  if (window.x < display.x)
    window.x = display.x;
  if (window.y < display.y)
    window.y = display.y;
  if (window.width < 100)
    window.width = 100;
  else if (window.width >= display.width)
    window.width = display.width;
  if (window.height < 100)
    window.height = 100;
  else if (window.height >= display.height)
    window.height = display.height;
  if (window.x + window.width >= display.x + display.width)
    window.x = display.x + display.width - window.width;
  if (window.y + window.height >= display.y + display.height)
    window.y = display.y + display.height - window.height;
}

// Transfer a V8 value to a List index.
void SetListValue(CefRefPtr<CefListValue> list, int index,
                  CefRefPtr<CefV8Value> value) {
  if (value->IsArray()) {
    CefRefPtr<CefListValue> new_list = CefListValue::Create();
    SetList(value, new_list);
    list->SetList(index, new_list);
  } else if (value->IsString()) {
    list->SetString(index, value->GetStringValue());
  } else if (value->IsBool()) {
    list->SetBool(index, value->GetBoolValue());
  } else if (value->IsInt()) {
    list->SetInt(index, value->GetIntValue());
  } else if (value->IsDouble()) {
    list->SetDouble(index, value->GetDoubleValue());
  }
}

// Transfer a V8 array to a List.
void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target) {
  ASSERT(source->IsArray());

  int arg_length = source->GetArrayLength();
  if (arg_length == 0)
    return;

  // Start with null types in all spaces.
  target->SetSize(arg_length);

  for (int i = 0; i < arg_length; ++i)
    SetListValue(target, i, source->GetValue(i));
}

// Transfer a List value to a V8 array index.
void SetListValue(CefRefPtr<CefV8Value> list, int index,
                  CefRefPtr<CefListValue> value) {
  CefRefPtr<CefV8Value> new_value;

  CefValueType type = value->GetType(index);
  switch (type) {
    case VTYPE_LIST: {
      CefRefPtr<CefListValue> list = value->GetList(index);
      new_value = CefV8Value::CreateArray(static_cast<int>(list->GetSize()));
      SetList(list, new_value);
      } break;
    case VTYPE_BOOL:
      new_value = CefV8Value::CreateBool(value->GetBool(index));
      break;
    case VTYPE_DOUBLE:
      new_value = CefV8Value::CreateDouble(value->GetDouble(index));
      break;
    case VTYPE_INT:
      new_value = CefV8Value::CreateInt(value->GetInt(index));
      break;
    case VTYPE_STRING:
      new_value = CefV8Value::CreateString(value->GetString(index));
      break;
    default:
      break;
  }

  if (new_value.get()) {
    list->SetValue(index, new_value);
  } else {
    list->SetValue(index, CefV8Value::CreateNull());
  }
}

// Transfer a List to a V8 array.
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target) {
  ASSERT(target->IsArray());

  int arg_length = static_cast<int>(source->GetSize());
  if (arg_length == 0)
    return;

  for (int i = 0; i < arg_length; ++i)
    SetListValue(target, i, source);
}

}  // namespace window_test
