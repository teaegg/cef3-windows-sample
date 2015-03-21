#ifndef CEFCLIENT_EXTENSIONS_REQUIRE_H_
#define CEFCLIENT_EXTENSIONS_REQUIRE_H_

#include "client_handler.h"
#include "client_app.h"

namespace extensions_require {

// Delegate creation. Called from ClientHandler.
void CreateProcessMessageDelegates(
    ClientHandler::ProcessMessageDelegateSet& delegates);
// Create the render delegate.
void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates);

// Platform implementations.

}  // namespace extensions_require

#endif  // CEFCLIENT_EXTENSIONS_REQUIRE_H_
