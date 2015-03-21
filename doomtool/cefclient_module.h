#ifndef CEF_TESTS_CEFCLIENT_MODULE_H_
#define CEF_TESTS_CEFCLIENT_MODULE_H_

#include "include/cef_base.h"

class CefV8Module  : public virtual CefBase {
public:
	///
	// Called to retrieve the module name. 
	// Return true if the directory was provided.
	///
	virtual bool GetModuleName(CefString& name) { return false; }

	///
	// Called to retrieve the module version. 
	// Return true if the directory was provided.
	///
	virtual bool GetModuleVersion(CefString& version) { return false; }

	///
	// Called to retrieve the module Dependents. 
	// Return true if the directory was provided.
	///
	//virtual bool GetModuleDependents(std::vector<CefString>& deps) { return false; }

	///
	// Called when the module manager wants to load the module. 
	// Return true if the directory was provided.
	///
	virtual bool OnExecute(/*const CefV8ValueList& deps, */CefRefPtr<CefV8Value>& obj) { return false; }
};

#endif  // CEF_TESTS_CEFCLIENT_MODULE_H_