///
/// CefWebLib Impl
/// 
/// 25.07.14/fb 
///
///

#include "CefWebLib.h"
#include "CefInstance.h"
#include "Win32Error.h"

// Function parameters
ECOparam browserParams[111] =
{
//	resid	datatype		flags   flags2+
	8001, 	fftCharacter,	0, 		0,				//pBasePath
	8002, 	fftInteger,		0, 		0,				//pWhat
	8000, 	fftCharacter,	0, 		0,				//pURL
	8003, 	fftCharacter,	0, 		0,				//Free
	8100,	fftCharacter,	0,		0,				//NOT USED

	// Events 
	
	// Dom Ready (pos 5)
	8101,	fftCharacter,	0,		0,				//pUrl
	8102,	fftCharacter,	0,		0,				//pScheme
	8103,	fftCharacter,	0,		0,				//pHost
	8104,	fftCharacter,	0,		0,				//pPort
	8105,	fftCharacter,	0,		0,				//pPath
	8106,	fftCharacter,	0,		0,				//pQuery
	8107,	fftCharacter,	0,		0,				//pAnchor
	8108,	fftCharacter,	0,		0,				//pFilename
	

	// FrameFail (pos 13)
	8120, 	fftInteger,		0, 		0,				//pErrorCode
	8121,	fftCharacter,	0,		0,				//pErrorMsg
	8122, 	fftCharacter,	0, 		0,				//pUrl

	// TitleChanged (pos 16)
	8130, 	fftCharacter,	0, 		0,				//pNewTitle

	// Adressbar-Changed (pos 17)
	8140, 	fftCharacter,	0, 		0,				//pNewUrl

	// ConsoleMessage (pos 18)
	8150, 	fftCharacter,		0, 		0,			//pConsoleMessage
	8151,	fftInteger,			0,		0,			//pLineNumber
	8152,	fftCharacter,		0,		0,			//pSource


	// onOpenWindow (pos 21)
	8160, 	fftCharacter,		0, 		0,			//pUrl
	8161,	fftCharacter,		0,		0,			//pTarget


	// onDownloadRequest (pos 23)
	8170,	fftInteger,			0,		0,			//pDownloadId
	8171, 	fftCharacter,		0, 		0,			//pUrl
	8172,	fftCharacter,		0,		0,			//pFileName
	8173,	fftCharacter,		0,		0,			//pMimeType


	// cancelDownload (pos 27)
	8180,	fftInteger,			0,		0,			//pDownloadId

	// startDownload (pos 28)
	8190,	fftInteger,			0,		0,			//pDownloadId
	8191,	fftCharacter,		0,		0,			//pFullPath

	// onDownloadUpdate (pos 30)
	8200,	fftInteger,			0,		0,			//pDownloadId
	8201,	fftInteger,			0,		0,			//pTotalBytes
	8202,	fftInteger,			0,		0,			//pReceivedBytes
	8203,	fftInteger,			0,		0,			//pCurrentSpeed


	// onDownloadFinished(pos 34)
	8210,	fftInteger,			0,		0,			//pDownloadId
	8211,	fftCharacter,		0,		0,			//pUrl
	8212,	fftCharacter,		0,		0,			//pSavedPath


	// onShowMessage(pos 37)
	8220,	fftCharacter,		0,		0,			//pType
	8221,	fftCharacter,		0,		0,			//pParam1
	8222,	fftCharacter,		0,		0,			//pParam2
	8223,	fftCharacter,		0,		0,			//pParam3

	// onOpenModule(pos 41)
	8240,	fftCharacter,		0,		0,			//pType
	8241,	fftCharacter,		0,		0,			//pParam1
	8242,	fftCharacter,		0,		0,			//pParam2
	8243,	fftCharacter,		0,		0,			//pParam3
	8244,	fftCharacter,		0,		0,			//pParam4
	8245,	fftCharacter,		0,		0,			//pParam5
	8246,	fftCharacter,		0,		0,			//pParam6
	8247,	fftCharacter,		0,		0,			//pParam7
	8248,	fftCharacter,		0,		0,			//pParam8
	8249,	fftCharacter,		0,		0,			//pParam9
	8250,	fftCharacter,		0,		0,			//pParam10	
	8251,	fftCharacter,		0,		0,			//pParam11
	8252,	fftCharacter,		0,		0,			//pParam12
	8253,	fftCharacter,		0,		0,			//pParam13
	8254,	fftCharacter,		0,		0,			//pParam14
	8255,	fftCharacter,		0,		0,			//pParam15
	8256,	fftCharacter,		0,		0,			//pParam16
	8257,	fftCharacter,		0,		0,			//pParam17
	8258,	fftCharacter,		0,		0,			//pParam18
	8259,	fftCharacter,		0,		0,			//Not Used
	8260,	fftCharacter,		0,		0,			//Not Used


	// onDorgAction (pos 62)
	8280,	fftCharacter,		0,		0,			//pType
	8281,	fftCharacter,		0,		0,			//pParam1
	8282,	fftCharacter,		0,		0,			//pParam2
	8283,	fftCharacter,		0,		0,			//pParam3
	8284,	fftCharacter,		0,		0,			//pParam4
	8285,	fftCharacter,		0,		0,			//pParam5
	8286,	fftCharacter,		0,		0,			//pParam6
	8287,	fftCharacter,		0,		0,			//pParam7
	8288,	fftCharacter,		0,		0,			//pParam8
	8289,	fftCharacter,		0,		0,			//pParam9
	8290,	fftCharacter,		0,		0,			//pParam10	
	8291,	fftCharacter,		0,		0,			//pParam11
	8292,	fftCharacter,		0,		0,			//pParam12
	8293,	fftCharacter,		0,		0,			//pParam13
	8294,	fftCharacter,		0,		0,			//pParam14
	8295,	fftCharacter,		0,		0,			//pParam15
	8296,	fftCharacter,		0,		0,			//pParam16
	8297,	fftCharacter,		0,		0,			//pParam17
	8298,	fftCharacter,		0,		0,			//pParam18

	
	// ofGetCompData (pos 81)
	8300,	fftCharacter,		0,		0,			//pCompId

	// ofSetCompData (pos 82)
	8310,	fftCharacter,		0,		0,			//pCompId
	8311,	fftCharacter,		0,		0,			//pData

	// ofIsCompValid (pos 84)
	8320,	fftCharacter,		0,		0,			//pCompId

	// ofGetVersionOfComp (pos 85)
	8330,	fftCharacter,		0,		0,			//pCompId
	
	// ofGetApiVersionOfComp (pos 86)
	8340,	fftCharacter,		0,		0,			//pCompId
	
	// ofSendActionToComp (pos 87)
	8350,	fftCharacter,		0,		0,			//pCompId
	8351,	fftCharacter,		0,		0,			//pType
	8352,	fftCharacter,		0,		0,			//pParam2
	8353,	fftCharacter,		0,		0,			//pParam2
	8354,	fftCharacter,		0,		0,			//pParam3
	8355,	fftCharacter,		0,		0,			//pParam4
	8356,	fftCharacter,		0,		0,			//pParam5
	8357,	fftCharacter,		0,		0,			//pParam6
	8358,	fftCharacter,		0,		0,			//pParam7
	8359,	fftCharacter,		0,		0,			//pParam8
	8310,	fftCharacter,		0,		0,			//pParam9
	
	// evOnCustomCompAction (pos 98)
	8370,	fftCharacter,		0,		0,			//pCompId
	8371,	fftCharacter,		0,		0,			//pType
	8372,	fftCharacter,		0,		0,			//pParam2
	8373,	fftCharacter,		0,		0,			//pParam2
	8374,	fftCharacter,		0,		0,			//pParam3
	8375,	fftCharacter,		0,		0,			//pParam4
	8376,	fftCharacter,		0,		0,			//pParam5
	8377,	fftCharacter,		0,		0,			//pParam6
	8378,	fftCharacter,		0,		0,			//pParam7
	8379,	fftCharacter,		0,		0,			//pParam8
	8380,	fftCharacter,		0,		0,			//pParam9
	
	// evOnCompInit (pos 109)
	8390,	fftCharacter,		0,		0,			//pCompId

	// Nächstes ab (pos 110)
};

ECOmethodEvent browserEvents[16] = 
{
	// 	event id			resourceid,     return datatype,	paramcnt		parameters			flags,		flags2
	evDoCloseModule,  		7500, 			0,					0, 				0, 					0, 			0,
	
	evDoShowMessage,  		7502, 			0,					4,				&browserParams[37],	0, 			0,
	
	evOnConsoleMessageAdded,7503, 			0,					3, 				&browserParams[18],	0, 			0,
	evOnDocumentReady,		7504, 			0,					8, 				&browserParams[5],	0, 			0,
	evOnFrameLoadingFailed,	7505, 			0,					3, 				&browserParams[13],	0, 			0,
	evOnTitleChange,		7506, 			0,					1, 				&browserParams[16],	0, 			0,
	evOnAdressBarChanged,	7507, 			0,					1, 				&browserParams[17],	0, 			0,
	evOnOpenNewWindow,		7508, 			0,					2, 				&browserParams[21],	0, 			0,
	evOnDownloadRequest,	7509, 			0,					4, 				&browserParams[23],	0, 			0,
	evOnDownloadUpdate,		7510, 			0,					4, 				&browserParams[30],	0, 			0,
	evOnDownloadFinish,		7511, 			0,					3, 				&browserParams[34],	0, 			0,
	evOnJsInitFailed,		7513, 			0,					0, 				0,					0, 			0,
	evOnCustomCompAction,	7514, 			0,					11,				&browserParams[98],	0, 			0,
	evOnCompInit,			7515,			0, 					1,				&browserParams[109],0, 			0
};

// This is how we define functions
ECOmethodEvent browserStaticFunctions[1] = 
{};

ECOproperty browserProperties[6] =
{ 
//  propid				resourceid,	datatype,		propflags	propFlags2, enumStart, 	enumEnd
	pBasePath,			4000, 		fftCharacter, 	0,			0, 			0, 			0,
	pUserPath,			4001, 		fftCharacter, 	0,			0, 			0, 			0
};

// the methods defined in CefWebLib.rc:
ECOmethodEvent browserObjfunctions[15] = {
//	methodid 				resourceid,		return datatype,	paramcnt		parameters		flags,	flags2
	ofnavigateToUrl,		7000,			fftInteger, 		1, 				&browserParams[2], 	0, 		0,
	ofHistoryGoBack,		7001,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofHistoryGoForward,		7002,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofInitWebView,			7003,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofFocus,				7004,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofUnFocus,				7005,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofShutDownWebView,		7006,			fftInteger, 		0, 				&browserParams[0], 	0, 		0,
	ofCancelDownload,		7007,			fftInteger, 		1, 				&browserParams[27],	0, 		0,
	ofStartDownload,		7008,			fftInteger, 		2, 				&browserParams[28],	0, 		0,
	ofGetCompData,			7009,			fftCharacter, 		1, 				&browserParams[81],	0, 		0,
	ofSetCompData,			7010,			fftInteger, 		2, 				&browserParams[82],	0, 		0,
	ofSendActionToComp,		7014,			fftInteger, 		11,				&browserParams[88],	0, 		0	
};

int webBrowserCounter = 0;

#define cSBrowserMethod_Count (0)
#define cIBrowserMethod_Count (sizeof(browserObjfunctions)/sizeof(browserObjfunctions[0]))
#define cIBrowserParams_Count (sizeof(browserStaticFunctions)/sizeof(browserStaticFunctions[0]))
#define cIBrowserProps_Count (sizeof(browserProperties)/sizeof(browserProperties[0]))
#define cIBrowserEvents_Count (sizeof(browserEvents)/sizeof(browserEvents[0]))


// Component library entry point (name as declared in resource 31000 )
extern "C" qlong OMNISWNDPROC GenericWndProc(HWND hwnd, LPARAM Msg,WPARAM wParam,LPARAM lParam,EXTCompInfo* eci)
{
	 // Initialize callback tables - THIS MUST BE DONE 
	 ECOsetupCallbacks(hwnd, eci);		

	 switch (Msg)
	 {	
		
		//////////////////////// Omnis Messages  //////////////////////////////
		
		// Create a new Object 
		case ECM_OBJCONSTRUCT:				
		{
			if ( eci->mCompId==COMP_BROWSER) {
				webBrowserCounter++;
				// ######## WebLib::WebBrowser* object = new WebLib::WebBrowser( hwnd, mWebSession );
				// ######## ECOinsertObject( eci, hwnd, (void*)object );
				CefInstance *instance = new CefInstance(hwnd);
				instance->AddRef();
				ECOinsertObject(eci, hwnd, static_cast<void*>(instance));
				return qtrue;
			}
			return qfalse;
		}
		
		// Destruct the object 
		case ECM_OBJDESTRUCT:					
		{
			if ( eci->mCompId==COMP_BROWSER) {
				// First find the object in the libraries chain of objects, 
				// this call if ok also removes the object from the chain.
				webBrowserCounter--;
				CefInstance *instance = static_cast<CefInstance*>(ECOremoveObject(eci, hwnd));
				if(instance)
					instance->SubRef();

				return qtrue;
			}
			return qfalse;
		}

		case ECM_GETMETHODNAME:
			// query method names
			return ECOreturnMethods(gInstLib, eci, &browserObjfunctions[0], cIBrowserMethod_Count);
		
		case ECM_GETSTATICOBJECT:
			// query static method names 
			return ECOreturnMethods(gInstLib, eci, &browserStaticFunctions[0], cSBrowserMethod_Count);

		case ECM_METHODCALL:
		{
			// call an object method
			if(eci->mCompId == COMP_BROWSER) {
				qlong funcId =	ECOgetId(eci);
				CefInstance::RefPtr instance(eci, hwnd);
				if(instance->IsHwnd(hwnd))
					instance->CallMethod(eci);
			}
			return qfalse;
		}
		
		// query properties
		case ECM_PROPERTYCANASSIGN:  		
		case ECM_SETPROPERTY: 				
		case ECM_GETPROPERTY:				
		{

			if ( eci->mCompId==COMP_BROWSER) {
				/* ######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  
				{
					return object->attributeSupport(Msg,wParam,lParam, eci);
				}*/
			}
			return 0L;
		}
		

		// query property names
		case ECM_GETPROPNAME:
		{
			if(eci->mCompId == COMP_BROWSER)
				return ECOreturnProperties(gInstLib, eci, &browserProperties[0], cIBrowserProps_Count);
			return qfalse;	
		}

		// ECM_GETEVENTNAME - to support events we have to register event information in the 
		// same way we register properties
		case ECM_GETEVENTNAME:
		{
			if(eci->mCompId == COMP_BROWSER)
				return ECOreturnEvents(gInstLib, eci, &browserEvents[0], cIBrowserEvents_Count);
			return qfalse;	
		}
		
		// query icon
		case ECM_GETCOMPICON:
		{
			if(eci->mCompId == COMP_BROWSER) 
				return ECOreturnIcon(gInstLib, eci, COMP_BROWSER_ICN);
			return qfalse;
		}

		// query id
		case ECM_GETCOMPID:
		{
			if(wParam == 1)
				return ECOreturnCompID(gInstLib, eci, COMP_BROWSER, cObjType_Basic);
			return 0L;
		}	

		// return external flags
 		case ECM_CONNECT:
		{
			return EXT_FLAG_LOADED | EXT_FLAG_ALWAYS_USABLE | EXT_FLAG_REMAINLOADED;
		} 
		case ECM_DISCONNECT:
		{ 
			return qtrue;
		}
		case ECM_GETCOMPLIBINFO:
		{
  			return ECOreturnCompInfo(gInstLib, eci, LIB_RES_NAME, OBJECT_COUNT);
		}

		case ECM_GETVERSION:
		{
			return ECOreturnVersion(VERSION_MAJOR, VERSION_MINOR);
		} 
		
		//////////////////////// Window Messages  //////////////////////////////
		
		// Anforderung zu Zeichnen 
		case WM_PAINT:
		{
			if(eci->mCompId == COMP_BROWSER) {
				/* ######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					if ( NULL!=object && object->paint()){
						return qtrue;
					}
					
				}*/
			}
			return qfalse;
		}

		case WM_WINDOWPOSCHANGED:
		{
			if(eci->mCompId == COMP_BROWSER) {
				CefInstance *instance = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
				if(instance && instance->IsHwnd(hwnd))
					instance->Resize();
			}
			return qtrue;
		}

		case WM_SIZE:
		{
			if(eci->mCompId == COMP_BROWSER) {
				RECT rect; GetClientRect(hwnd, &rect);
				rect.left = 0;
				/* ######## WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					WNDinvalidateRect( hwnd, NULL );
					return qtrue;
				}*/
			}
			return qtrue;
		}

		case WM_DESTROY:
		{
			/* ######## if ( eci->mCompId==COMP_BROWSER) {
				WebLib::WebBrowser* object = (WebLib::WebBrowser*)ECOfindObject( eci, hwnd );
				if (object && object->hwnd() == hwnd)  // rmm4999
				{
					return qtrue;
				}
			}*/
			return qtrue;
		}

		default:
			// the pipe listener thread has signalled that there are messages in the queue.
			if(Msg == CefInstance::PIPE_MESSAGES_AVAILABLE) {
				CefInstance::RefPtr(eci, hwnd)->PopMessages();
				return qtrue;
			}
	 }

	 // As a final result this must ALWAYS be called. It handles all other messages that this component
	 // decides to ignore.
	 return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}

	
