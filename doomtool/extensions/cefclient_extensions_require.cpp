#include "cefclient_extensions_require.h"
#include "include/cef_urlrequest.h"
#include "cefclient_module.h"

namespace extensions_require {

namespace {

typedef std::set<CefRefPtr<CefV8Module>> V8ModuleSet;
V8ModuleSet v8_modules;

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
        return true;
    }

    IMPLEMENT_REFCOUNTING(ProcessMessageDelegate);
};

class URLRequestClient : public CefURLRequestClient {
public:
	URLRequestClient(CefRefPtr<CefV8Context> context, 
		CefRefPtr<CefV8Value> func, 
		CefRefPtr<CefV8Value> status_func,
		CefRefPtr<CefV8Value> object)
        : upload_total_(0),
          download_total_(0) {
		context_ = context;
		func_ = func;
		status_func_ = status_func;
		object_ = object;
	}

    virtual void OnRequestComplete(CefRefPtr<CefURLRequest> request) OVERRIDE {
        CefURLRequest::Status status = request->GetRequestStatus();
        CefURLRequest::ErrorCode error_code = request->GetRequestError();
        CefRefPtr<CefResponse> response = request->GetResponse();
//		::MessageBoxA(NULL, download_data_.c_str(), NULL, 0);

		context_->Enter();
		CefV8ValueList args;
		func_->ExecuteFunction(object_, args);
		context_->Exit();
    }

    virtual void OnUploadProgress(CefRefPtr<CefURLRequest> request,
                                  uint64 current,
                                  uint64 total) OVERRIDE {
        upload_total_ = total;
    }

    virtual void OnDownloadProgress(CefRefPtr<CefURLRequest> request,
                                    uint64 current,
									uint64 total) OVERRIDE {
        download_total_ = total;
		if(status_func_->IsFunction()){
			context_->Enter();
			CefV8ValueList args;
			args.push_back(CefV8Value::CreateInt(current*100/total));
			status_func_->ExecuteFunction(object_, args);
			context_->Exit();
		}
	}

    virtual void OnDownloadData(CefRefPtr<CefURLRequest> request,
                                const void* data,
                                size_t data_length) OVERRIDE {
        download_data_ += std::string(static_cast<const char*>(data), data_length);
    }

    virtual bool GetAuthCredentials(bool isProxy,
                                    const CefString& host,
                                    int port,
                                    const CefString& realm,
                                    const CefString& scheme,
                                    CefRefPtr<CefAuthCallback> callback) OVERRIDE {
        return true;
    }

	CefRefPtr<CefV8Context> context_;
	CefRefPtr<CefV8Value> func_;
	CefRefPtr<CefV8Value> status_func_;
	CefRefPtr<CefV8Value> object_;
private:
    uint64 upload_total_;
    uint64 download_total_;
    std::string download_data_;

private:
    IMPLEMENT_REFCOUNTING(URLRequestClient);
};

// Handles the native implementation for the client_app extension.
class V8Handler : public CefV8Handler {
public:
    V8Handler() {
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) {
        bool handled = false;

        if(name == "require") {
            if(arguments[0]->IsFunction() && arguments[1]->IsUndefined()) {
                // require(function(){})
				CefV8ValueList args;
				//args.push_back(CefV8Value::CreateInt(1));
                arguments[0]->ExecuteFunction(object, args);
                handled = true;
            } else if(arguments[0]->IsArray() && arguments[1]->IsFunction()) {
				bool go_error = false;
				for(int i = 0; i < arguments[0]->GetArrayLength(); ++i){
					if(!arguments[0]->GetValue(i)->IsString()){
						go_error = true;
						break;
					} 
				}
				if(!go_error){
					// Create the client instance.
					/*CefRefPtr<URLRequestClient> client = 
					new URLRequestClient(CefV8Context::GetCurrentContext(),
					arguments[1], 
					arguments[2],
					object);
					// Set up the CefRequest object.
					CefRefPtr<CefRequest> request = CefRequest::Create();
					std::string url("http://localhost/");
					url += arguments[0]->GetValue(0)->GetStringValue();

					request->SetURL(url);
					request->SetMethod("POST");
					CefRefPtr<CefURLRequest> url_request = CefURLRequest::Create(request, client.get());*/

					CefString str;
					CefV8ValueList args(arguments[0]->GetArrayLength(), CefV8Value::CreateUndefined());
					V8ModuleSet::iterator it = v8_modules.begin();
					for (; it != v8_modules.end(); ++it){
						if((*it)->GetModuleName(str)){
							for(int i =0; i < arguments[0]->GetArrayLength(); ++i) {
								if(str == arguments[0]->GetValue(i)->GetStringValue()){
									args[i] = CefV8Value::CreateObject(NULL);
									(*it)->OnExecute(args[i]);
									//break; // when only one module with same name
								}
							}
						}
					}
					//假设全部已载入
					arguments[1]->ExecuteFunction(object, args);
					handled = true;
					
				}
            }
        }

        if (!handled)
            exception = "Invalid method arguments";

        return true;
    }

    IMPLEMENT_REFCOUNTING(V8Handler);
};
class V8Module : public CefV8Module {
public:
	V8Module() {
	}
	///
	virtual bool GetModuleName(CefString& name) { 
		name = "test1.dt";
		return true; 
	}
	///
	virtual bool GetModuleVersion(CefString& version) { return false; }
	///
	virtual bool OnExecute(CefRefPtr<CefV8Value>& obj) { 
		obj->SetValue("name",
			CefV8Value::CreateString("test.name"),
			V8_PROPERTY_ATTRIBUTE_NONE);
		return true; 
	}

	IMPLEMENT_REFCOUNTING(V8Module);
};
class RenderDelegate : public ClientApp::RenderDelegate {
public:
    RenderDelegate() {
    }

    virtual void OnWebKitInitialized(CefRefPtr<ClientApp> app) {
        std::string require_code =
            "var require;"
            "if(!require)"
            "	require = {};"
            "(function(){"
            "	require = function(modules, callback, progress) {"
            "		native function require();"
            "		return require(modules, callback, progress);"
            "	};"
            "})()";
        CefRegisterExtension("v8/require", require_code, new V8Handler());
		v8_modules.insert(new V8Module());
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

}  // namespace extensions_require
