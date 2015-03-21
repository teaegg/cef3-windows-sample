#include "cefclient_extensions_window.h"
#include "cefclient_osr_widget_win.h"

namespace extensions_window {

namespace {

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
    std::string name = message->GetName();
	HWND hwnd = browser->GetHost()->GetWindowHandle();
	if(name == "startDrag") {
		StartDrag(hwnd);
		return true;
	} else if (name == "setZoomLevel") {
		browser->GetHost()->SetZoomLevel(message->GetArgumentList()->GetInt(0));
		return true;
	} else if (name == "setTopMost") {
		SetTopMost(hwnd, message->GetArgumentList()->GetBool(0));
		return true;
	} else if (name == "moveTo") {
		MoveTo(hwnd, message->GetArgumentList()->GetInt(0), message->GetArgumentList()->GetInt(1));
		return true;
	} else if (name == "moveBy") {
		MoveBy(hwnd, message->GetArgumentList()->GetInt(0), message->GetArgumentList()->GetInt(1));
		return true;
	} else if (name == "resizeTo") {
		ResizeTo(hwnd, message->GetArgumentList()->GetInt(0), message->GetArgumentList()->GetInt(1));
		return true;
	} else if (name == "resizeBy") {
		ResizeBy(hwnd, message->GetArgumentList()->GetInt(0), message->GetArgumentList()->GetInt(1));
		return true;
	} else if (name == "focus") {
		Focus(hwnd);
		return true;
	} else if (name == "blur") {
		Blur(hwnd);
		return true;
	} else if (name == "show") {
		Show(hwnd);
		return true;
	} else if (name == "hide") {
		Hide(hwnd);
		return true;
	} else if (name == "reload") {
		browser->Reload();
		return true;
	} else if (name == "reloadIgnoringCache") {
		browser->ReloadIgnoreCache();
		return true;
	} else if (name == "maximize") {
		Maximize(hwnd);
		return true;
	} else if (name == "minimize") {
		Minimize(hwnd);
		return true;
	} else if (name == "restore") {
		Restore(hwnd);
		return true;
	} else if (name == "popUp") {
		RECT rect;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION);
		rect.left = 0;
		rect.top = 0;
		CefRefPtr<OSRWindow> osr_window = OSRWindow::From(handler->GetOSRHandler());
		HWND hWnd = osr_window->CreateWidget(NULL, rect, NULL,L"DoomToolPopUpWnd");

		if(!hWnd)
			::MessageBoxA(NULL, "CreateWidget failed!", NULL, 0);
		CefWindowInfo info;
		CefBrowserSettings settings;
		info.SetAsOffScreen(hWnd);
		info.SetTransparentPainting(TRUE);

		if(!CefBrowserHost::CreateBrowser(info, browser->GetHost()->GetClient(), "http://localhost", settings, NULL))
			::MessageBoxA(NULL, "CreateBrowser failed!", NULL, 0);
	} else {
		ASSERT(false);  // Not reached.
	}
    return false;
  }

  IMPLEMENT_REFCOUNTING(ProcessMessageDelegate);
};

// Handles the native implementation for the client_app extension.
class V8Handler : public CefV8Handler {
 public:
  V8Handler(){
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) {
    bool handled = false;
	CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();

	ASSERT(browser.get());
	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(name);
    if (name == "setZoomLevel") {
      // Send a message to the browser process.
      if (arguments.size() == 1 &&
          arguments[0]->IsInt()) {
		  message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
          browser->SendProcessMessage(PID_BROWSER, message);
          handled = true;
      }
	} else if (name == "setTopMost") {
		if (arguments.size() == 1 && arguments[0]->IsBool()) {
			message->GetArgumentList()->SetBool(0, arguments[0]->GetBoolValue());
			browser->SendProcessMessage(PID_BROWSER, message);
			handled = true;
		}
    } else if ( name == "moveTo" || 
				name == "moveBy" || 
				name == "resizeTo" || 
				name == "resizeBy") {
		if(arguments.size() == 2 && arguments[0]->IsInt() && arguments[1]->IsInt()) {
			message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
			message->GetArgumentList()->SetInt(1, arguments[1]->GetIntValue());
			browser->SendProcessMessage(PID_BROWSER, message);
			handled = true;
		}
	} else if ( name == "startDrag" ||
				name == "focus" || 
				name == "blur" || 
				name == "show" || 
				name == "hide" || 
				name == "reload" || 
				name == "reloadIgnoringCache" || 
				name == "maximize" || 
				name == "minimize" || 
				name == "restore" ||
				name == "popUp"){
		if(arguments.size() == 0) {
			browser->SendProcessMessage(PID_BROWSER, message);
			handled = true;
		}
	} else {
		ASSERT(false);	// function name missed.
	}
    if (!handled)
      exception = "Invalid method arguments";

    return true;
  }

  IMPLEMENT_REFCOUNTING(V8Handler);
};

class RenderDelegate : public ClientApp::RenderDelegate {
 public:
  RenderDelegate() {
  }

  virtual void OnContextCreated(CefRefPtr<ClientApp> app,
                                  CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) OVERRIDE {
	if(!frame->IsMain())
		return;
    CefRefPtr<CefV8Value> object = context->GetGlobal();

    CefRefPtr<CefV8Handler> handler = new V8Handler();

    // Bind window functions.
	object->SetValue("startDrag",
		CefV8Value::CreateFunction("startDrag", handler),
		V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("setZoomLevel",
        CefV8Value::CreateFunction("setZoomLevel", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("moveTo",
        CefV8Value::CreateFunction("moveTo", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("moveBy",
        CefV8Value::CreateFunction("moveBy", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
    object->SetValue("resizeTo",
        CefV8Value::CreateFunction("resizeTo", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("resizeBy",
        CefV8Value::CreateFunction("resizeBy", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("focus",
        CefV8Value::CreateFunction("focus", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("blur",
        CefV8Value::CreateFunction("blur", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("show",
        CefV8Value::CreateFunction("show", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
    object->SetValue("hide",
        CefV8Value::CreateFunction("hide", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("reload",
        CefV8Value::CreateFunction("reload", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
    object->SetValue("reloadIgnoringCache",
        CefV8Value::CreateFunction("reloadIgnoringCache", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("maximize",
        CefV8Value::CreateFunction("maximize", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
    object->SetValue("minimize",
        CefV8Value::CreateFunction("minimize", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("restore",
        CefV8Value::CreateFunction("restore", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("setTopMost",
        CefV8Value::CreateFunction("setTopMost", handler),
            V8_PROPERTY_ATTRIBUTE_READONLY);
	object->SetValue("popUp",
		CefV8Value::CreateFunction("popUp", handler),
		V8_PROPERTY_ATTRIBUTE_READONLY);

	std::string window_code =
		"(function() {"
		"	var zoomLevel_ = 0;"
		"	window.__defineGetter__('zoomLevel', function() {"
		"		return zoomLevel_;"
		"	});"
		"	window.__defineSetter__('zoomLevel', function(b) {"
		"		window.setZoomLevel(b);"
		"		zoomLevel_ = b;"
		"	});"
		"	window.addEventListener('load', function(){"
		"		var regions = document.querySelectorAll('*[dragable-region]');"
		"		for(i in regions) {"
		"			regions[i].addEventListener('mousedown', function(event) {"
		"				if (event.target != event.currentTarget)"
		"					return;"
		"				window.startDrag();"
		"			}, false);"
		"		}"
		"	}, false);"
		"})();";
	frame->ExecuteJavaScript(window_code, frame->GetURL(), 0);
  }

  IMPLEMENT_REFCOUNTING(RenderDelegate);
};

}  // namespace

void CreateProcessMessageDelegates(
    ClientHandler::ProcessMessageDelegateSet& delegates) {
  delegates.insert(new ProcessMessageDelegate);
}

void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates) {
  delegates.insert(new RenderDelegate);
}

}  // namespace extensions_window
