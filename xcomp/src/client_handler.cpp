#include "client_handler.h"

ClientHandler::ClientHandler()
{
	int x = 1;
}

ClientHandler::~ClientHandler()
{
	
}



void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  //REQUIRE_UI_THREAD();

  //AutoLock lock_scope(this);
  if (!m_Browser.get())   {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserId = browser->GetIdentifier();
  }
}