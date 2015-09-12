///
/// CefWebLib Impl
/// 
/// 25.07.14/fb 
///
///

#include "CefWebLib.h"
#include "CefInstance.h"
#include "Win32Error.h"
#include "OmnisTools.h"

using namespace OmnisTools;

struct EventId {
	Identifier id_;
	qlong resource_id_;
};

// Add a method/event that takes no parameters.
void AddMethodEvent(std::vector<ECOmethodEvent> &evs, const EventId &id) {
	ECOmethodEvent event = {
		// 	event id	resourceid,  		return datatype,	paramcnt,	parameters,	flags,	flags2
		id.id_,			id.resource_id_, 	0,					0,			0,			0,		0
	};
	evs.push_back(event);
}

// Add a method/event that takes a number of parameters.
// The template allows us to use literals without manually 
// counting the number of parameters.
template<size_t count> void AddMethodEvent(std::vector<ECOmethodEvent> &evs, const EventId &id, ECOparam(&params)[count]) {
	ECOmethodEvent event = {
		// 	event id	resourceid,  		return datatype,	paramcnt,	parameters,	flags,	flags2
		id.id_,			id.resource_id_, 	0,					count,		params,		0,		0
	};
	evs.push_back(event);
}

// return the events that we send to Omnis.
// all literal numbers are string resource ids.
qbool ReturnEvents(EXTCompInfo* eci) {
	std::vector<ECOmethodEvent> events;
	{
		EventId id = { evLoadingStateChange, 7500 };
		ECOparam params[] = {
			8100, fftBoolean, 0, 0,				// pIsLoading
			8101, fftBoolean, 0, 0,				// pCanGoBack
			8102, fftBoolean, 0, 0,				// pCanGoForward
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { evLoadStart, 7503 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { evLoadEnd, 7504 };
		ECOparam params[] = {
			8120, fftInteger, 0, 0,				// pStatusCode
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { evLoadError, 7505 };
		ECOparam params[] = {
			8120, fftInteger,	0, 0,			// pStatusCode
			8121, fftCharacter, 0, 0,			// pErrorMsg
			8122, fftCharacter, 0, 0,			// pUrl
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evTitleChange, 7506};
		ECOparam params[] = {
			8130, 	fftCharacter,	0, 		0,	// pTitle
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evAddressChange, 7507};
		ECOparam params[] = {
			8122, 	fftCharacter,	0, 		0,	// pUrl
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evDownloadUpdate, 7510};
		ECOparam params[] = {
			8200,	fftInteger,			0,		0,			// pId
			8201,	fftBoolean,			0,		0,			// pIsComplete
			8202,	fftBoolean,			0,		0,			// pIsCanceled
			8203,	fftInteger,			0,		0,			// pReceivedBytes
			8204,	fftInteger,			0,		0,			// pTotalBytes
			8205,	fftInteger,			0,		0,			// pCurrentSpeed
			8206,	fftCharacter,		0,		0,			// pFullPath
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evCustomEvent, 7514 };
		ECOparam params[] = {
			8371, fftCharacter, 0, 0,			// pName
			8372, fftCharacter, 0, 0,			// pParam1
			8373, fftCharacter, 0, 0,			// pParam2
			8374, fftCharacter, 0, 0,			// pParam3
			8375, fftCharacter, 0, 0,			// pParam4
			8376, fftCharacter, 0, 0,			// pParam5
			8377, fftCharacter, 0, 0,			// pParam6
			8378, fftCharacter, 0, 0,			// pParam7
			8379, fftCharacter, 0, 0,			// pParam8
			8380, fftCharacter, 0, 0,			// pParam9
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evGotFocus, 7516};
		AddMethodEvent(events, id);
	}
	return ECOreturnEvents(gInstLib, eci, &events[0], events.size());
}

qbool ReturnMethods(EXTCompInfo *eci) {
	std::vector<ECOmethodEvent> events;
	{
		EventId id = { ofNavigateToUrl, 7000 };
		ECOparam params[] = {
			8000, fftCharacter, 0, 0,				// pURL
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofHistoryBack, 7001 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofHistoryForward, 7002 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofSendCustomEvent, 7015 };
		ECOparam params[] = {
			8400, fftCharacter, 0, 0,			// pName
			8401, fftCharacter, 0, 0,			// pValue
		};
		AddMethodEvent(events, id, params);
	}
	return ECOreturnMethods(gInstLib, eci, &events[0], events.size());
}

qbool ReturnStaticMethods(EXTCompInfo *eci) {
	std::vector<ECOmethodEvent> events;
	return ECOreturnMethods(gInstLib, eci, &events[0], events.size());
}

// This is how we define functions
ECOmethodEvent browserStaticFunctions[1] = 
{};

ECOproperty browserProperties[6] =
{ 
//  propid						resourceid,	datatype,		propflags	propFlags2, enumStart, 	enumEnd
	pContextMenus,		4000, 		fftBoolean, 	0,			0, 			0, 			0,
	pTraceLogConsole,	4001,		fftBoolean,		0,			0,			0,			0,
};

#define cSBrowserMethod_Count (0)
#define cIBrowserParams_Count (sizeof(browserStaticFunctions)/sizeof(browserStaticFunctions[0]))
#define cIBrowserProps_Count (sizeof(browserProperties)/sizeof(browserProperties[0]))

// Component library entry point (name as declared in resource 31000 )
extern "C" qlong OMNISWNDPROC GenericWndProc(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo *eci)
{
	 // Initialize callback tables - THIS MUST BE DONE 
	 ECOsetupCallbacks(hwnd, eci);		

	 switch (Msg)
	 {	
		
		//////////////////////// Omnis Messages  //////////////////////////////
		
		case ECM_OBJCONSTRUCT: {
			// create a new CEF instance 
			if (eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = new CefInstance(hwnd);
				instance->AddRef();
				ECOinsertObject(eci, hwnd, static_cast<void*>(instance));
				return qtrue;
			}
			return qfalse;
		}
		
		case ECM_OBJDESTRUCT: {
			// destroy the CEF instance 
			if(eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOremoveObject(eci, hwnd));
				if(instance)
					instance->SubRef();
				return qtrue;
			}
			return qfalse;
		}

		case ECM_GETMETHODNAME: {
			// query method names
			return ReturnMethods(eci);
		}
		
		case ECM_GETSTATICOBJECT: {
			// query static method names 
			return ReturnStaticMethods(eci);
		}

		case ECM_METHODCALL: {
			// call an object method
			if(eci->mCompId == COMP_BROWSER) {
				qlong funcId =	ECOgetId(eci);
				CefInstance::RefPtr instance(eci, hwnd);
				if(instance->IsHwnd(hwnd))
					instance->CallMethod(eci);
			}
			return qfalse;
		}

		case ECM_GETPROPNAME: {
			// query properties
			if (eci->mCompId == COMP_BROWSER)
				return ECOreturnProperties(gInstLib, eci, &browserProperties[0], cIBrowserProps_Count);
			return qfalse;
		}
		
		case ECM_PROPERTYCANASSIGN:
			// can assign all our properties.
			return qtrue;

		case ECM_SETPROPERTY: {
			if (eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
				if (instance && instance->IsHwnd(hwnd))
					return instance->SetProperty(eci);
			}
			return qfalse;
		}
		case ECM_GETPROPERTY: {
			if (eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
				if (instance && instance->IsHwnd(hwnd))
					return instance->GetProperty(eci);
			}
			return qfalse;
		}

		case ECM_GETEVENTNAME: {
			// query events
			if(eci->mCompId == COMP_BROWSER)
				return ReturnEvents(eci);
			return qfalse;	
		}
		
		case ECM_GETCOMPICON: {
			// query icon
			if(eci->mCompId == COMP_BROWSER) 
				return ECOreturnIcon(gInstLib, eci, COMP_BROWSER_ICN);
			return qfalse;
		}

		case ECM_GETCOMPID: {
			// query comp id
			if(wParam == 1)
				return ECOreturnCompID(gInstLib, eci, COMP_BROWSER, cObjType_Basic);
			return qfalse;
		}	

 		case ECM_CONNECT: {
			// return external flags
			return EXT_FLAG_LOADED | EXT_FLAG_ALWAYS_USABLE | EXT_FLAG_REMAINLOADED;
		} 
		case ECM_DISCONNECT: { 
			return qtrue;
		}
		case ECM_GETCOMPLIBINFO: {
  			return ECOreturnCompInfo(gInstLib, eci, LIB_RES_NAME, OBJECT_COUNT);
		}
		case ECM_GETVERSION: {
			return ECOreturnVersion(VERSION_MAJOR, VERSION_MINOR);
		} 
		case ECM_CANFOCUS:
		case ECM_CANCLICK:{
			// the component can click and receive the focus if it is enabled
			return wParam;
		}
		
		//////////////////////// Window Messages  //////////////////////////////

		case WM_FOCUSCHANGED: {
			if (wParam) {
				// the xcomp gained focus, we need to notify CEF so focus is passed down to the browser window.
				if (eci->mCompId == COMP_BROWSER) {
					CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
					if (instance && instance->IsHwnd(hwnd))
						instance->Focus();
					return qtrue;
				}
			} // else the xcomp lost focus.
			break;
		}

		case WM_WINDOWPOSCHANGED: {
			// the comp changed size, resize the browser component to match.
			if(eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
				if(instance && instance->IsHwnd(hwnd))
					instance->Resize();
			}
			return qtrue;
		}

		default:
			if(Msg == CefInstance::PIPE_MESSAGES_AVAILABLE) {
				// the pipe listener thread has signalled that there are messages in the queue.
				CefInstance::RefPtr(eci, hwnd)->PopMessages();
				return qtrue;
			}
	 }

	 // defer all unhandled messages to the default handler.
	 return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}

	
