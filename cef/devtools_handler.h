#ifndef CEF_DEVTOOLS_HANDLER_H_
#define CEF_DEVTOOLS_HANDLER_H_

#include "include/cef_client.h"

class DevToolsHandler : public CefClient {
 private:

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(DevToolsHandler);
};

#endif  // CEF_DEVTOOLS_HANDLER_H_
