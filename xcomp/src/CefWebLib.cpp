///
/// CefWebLib Impl
/// 
/// 25.07.14/fb 
///
///

#include "CefWebLib.h"
#include "CefInstance.h"
#include "Win32Error.h"

struct EventId {
	Enum id_;
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
		EventId id = {evDoCloseModule, 7500};
		AddMethodEvent(events, id);
	}
	{
		EventId id = {evDoShowMessage, 7502};
		ECOparam params[] = {
			// resid	datatype		flags   flags2+
			8220,	fftCharacter,		0,		0,			// pType
			8221,	fftCharacter,		0,		0,			// pParam1
			8222,	fftCharacter,		0,		0,			// pParam2
			8223,	fftCharacter,		0,		0,			// pParam3
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnConsoleMessageAdded, 7503};
		ECOparam params[] = {
			8150, 	fftCharacter,		0, 		0,			// pConsoleMessage
			8151,	fftInteger,			0,		0,			// pLineNumber
			8152,	fftCharacter,		0,		0,			// pSource
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnDocumentReady, 7504};
		ECOparam params[] = {
			8101,	fftCharacter,	0,		0,				// pUrl
			8102,	fftCharacter,	0,		0,				// pScheme
			8103,	fftCharacter,	0,		0,				// pHost
			8104,	fftCharacter,	0,		0,				// pPort
			8105,	fftCharacter,	0,		0,				// pPath
			8106,	fftCharacter,	0,		0,				// pQuery
			8107,	fftCharacter,	0,		0,				// pAnchor
			8108,	fftCharacter,	0,		0,				// pFilename
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnFrameLoadingFailed, 7505};
		ECOparam params[] = {
			8120, 	fftInteger,		0, 		0,				// pErrorCode
			8121,	fftCharacter,	0,		0,				// pErrorMsg
			8122, 	fftCharacter,	0, 		0,				// pUrl
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnTitleChange, 7506};
		ECOparam params[] = {
			8130, 	fftCharacter,	0, 		0,				// pNewTitle
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnAddressBarChanged, 7507};
		ECOparam params[] = {
			8140, 	fftCharacter,	0, 		0,				// pNewUrl
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnOpenNewWindow, 7508};
		ECOparam params[] = {
			8160, 	fftCharacter,		0, 		0,			// pUrl
			8161,	fftCharacter,		0,		0,			// pTarget
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnDownloadRequest, 7509};
		ECOparam params[] = {
			8170,	fftInteger,			0,		0,			// pDownloadId
			8171, 	fftCharacter,		0, 		0,			// pUrl
			8172,	fftCharacter,		0,		0,			// pFileName
			8173,	fftCharacter,		0,		0,			// pMimeType
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnDownloadUpdate, 7510};
		ECOparam params[] = {
			8200,	fftInteger,			0,		0,			// pDownloadId
			8201,	fftInteger,			0,		0,			// pTotalBytes
			8202,	fftInteger,			0,		0,			// pReceivedBytes
			8203,	fftInteger,			0,		0,			// pCurrentSpeed
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnDownloadFinish, 7511};
		ECOparam params[] = {
			8210,	fftInteger,			0,		0,			// pDownloadId
			8211,	fftCharacter,		0,		0,			// pUrl
			8212,	fftCharacter,		0,		0,			// pSavedPath
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnJsInitFailed, 7513};
		AddMethodEvent(events, id);
	}
	{
		EventId id = {evOnCustomCompAction, 7514};
		ECOparam params[] = {
			8370, fftCharacter, 0, 0,			// pCompId
			8371, fftCharacter, 0, 0,			// pType
			8372, fftCharacter, 0, 0,			// pParam2
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
		EventId id = {evOnCompInit, 7515};
		ECOparam params[] = {
			8390,	fftCharacter,		0,		0,			// pCompId
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = {evOnGotFocus, 7516};
		AddMethodEvent(events, id);
	}
	return ECOreturnEvents(gInstLib, eci, &events[0], events.size());
}

qbool ReturnMethods(EXTCompInfo *eci) {
	std::vector<ECOmethodEvent> events;
	{
		EventId id = { ofnavigateToUrl, 7000 };
		ECOparam params[] = {
			8000, fftCharacter, 0, 0,				// pURL
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofHistoryGoBack, 7001 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofHistoryGoForward, 7002 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofInitWebView, 7003 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofFocus, 7004 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofUnFocus, 7005 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofShutDownWebView, 7006 };
		AddMethodEvent(events, id);
	}
	{
		EventId id = { ofCancelDownload, 7007 };
		ECOparam params[] = {
			8180, fftInteger, 0, 0,			// pDownloadId
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofStartDownload, 7008 };
		ECOparam params[] = {
			8190, fftInteger, 0, 0,			// pDownloadId
			8191, fftCharacter, 0, 0,			// pFullPath
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofGetCompData, 7009 };
		ECOparam params[] = {
			8300, fftCharacter, 0, 0,			// pCompId
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofSetCompData, 7010 };
		ECOparam params[] = {
			8310, fftCharacter, 0, 0,			// pCompId
			8311, fftCharacter, 0, 0,			// pData
		};
		AddMethodEvent(events, id, params);
	}
	{
		EventId id = { ofSendActionToComp, 7014 };
		ECOparam params[] = {
			8350, fftCharacter, 0, 0,			// pCompId
			8351, fftCharacter, 0, 0,			// pType
			8352, fftCharacter, 0, 0,			// pParam2
			8353, fftCharacter, 0, 0,			// pParam2
			8354, fftCharacter, 0, 0,			// pParam3
			8355, fftCharacter, 0, 0,			// pParam4
			8356, fftCharacter, 0, 0,			// pParam5
			8357, fftCharacter, 0, 0,			// pParam6
			8358, fftCharacter, 0, 0,			// pParam7
			8359, fftCharacter, 0, 0,			// pParam8
			8310, fftCharacter, 0, 0,			// pParam9
		};
		AddMethodEvent(events, id, params);
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
//  propid				resourceid,	datatype,		propflags	propFlags2, enumStart, 	enumEnd
	pBasePath,			4000, 		fftCharacter, 	0,			0, 			0, 			0,
	pUserPath,			4001, 		fftCharacter, 	0,			0, 			0, 			0
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
			if ( eci->mCompId==COMP_BROWSER) {
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
		case ECM_SETPROPERTY: 				
		case ECM_GETPROPERTY: {
			// access proprerties

			if(eci->mCompId == COMP_BROWSER) {
				/* ######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  
				{
					return object->attributeSupport(Msg,wParam,lParam, eci);
				}*/
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
		
		//////////////////////// Window Messages  //////////////////////////////
		
		/*// Anforderung zu Zeichnen 
		case WM_PAINT:
		{
			if(eci->mCompId == COMP_BROWSER) {
				 ######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					if ( NULL!=object && object->paint()){
						return qtrue;
					}
					
				}
			}
			return qfalse;
		}*/

		case WM_WINDOWPOSCHANGED: {
			// the comp changed size, resize the browser component to match.
			if(eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
				if(instance && instance->IsHwnd(hwnd))
					instance->Resize();
			}
			return qtrue;
		}

		/* case WM_SIZE:
		{
			if(eci->mCompId == COMP_BROWSER) {
				RECT rect; GetClientRect(hwnd, &rect);
				rect.left = 0;
				######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					WNDinvalidateRect( hwnd, NULL );
					return qtrue;
				}
			}
			return qtrue;
		}*/

		/*case WM_DESTROY:
		{
			 ######## if ( eci->mCompId==COMP_BROWSER) {
				WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					return qtrue;
				}
			}
			return qtrue;
		}*/

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

	
