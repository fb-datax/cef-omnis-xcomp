#ifndef CEF_TESTS_CEF_OTHER_APP_H_
#define CEF_TESTS_CEF_OTHER_APP_H_

#include "include/cef_app.h"

class OtherApp : public CefApp {
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(OtherApp);
};

#endif  // CEF_TESTS_CEF_OTHER_APP_H_
