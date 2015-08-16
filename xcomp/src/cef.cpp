#include "cef.h"
#include "cef_client.h"

/* Component library entry point (name as declared in resource) */
extern "C" LRESULT OMNISWNDPROC CEFWndProc(HWND hwnd, UINT Msg,WPARAM wParam,LPARAM lParam,EXTCompInfo* eci)
{
   PAINTSTRUCT ps;
   HDC hdc;

     ECOsetupCallbacks(hwnd,eci);		/* Initialize callback tables */
	 switch (Msg)
	 {      
	       case ECM_CONNECT: 
			{ 
				return EXT_FLAG_LOADED | EXT_FLAG_BCOMPONENTS; 
			} 
			case ECM_GETVERSION:
			{
				return ECOreturnVersion( 1, 1 ); 
			}
			case ECM_OBJCONSTRUCT:				/* Construct an object */
			{
				cef_client* object = new cef_client( hwnd, eci->mOmnisInstance );
				object->cef_create();
				//if (object != NULL)
				//{
				//	object->cef_create();
				//}
				ECOinsertObject( eci, hwnd, (void*)object );
				return qtrue;
			}
			case ECM_OBJDESTRUCT:					/* Destruct the object */
			{
				cef_client* object = (cef_client*)ECOremoveObject( eci, hwnd );
				if ( NULL!=object )
					delete object;
				return qtrue;
			}
		    case ECM_GETCOMPLIBINFO: /* Return component library name & component count */
		    {
			   return ECOreturnCompInfo( gInstLib, eci, LIB_RES_NAME, OBJECT_COUNT );
		    }
			case ECM_GETCOMPICON: 	/* Return component icon (as a hbitmap) */
			{
				if ( eci->mCompId==CEF_ID ) return ECOreturnIcon( gInstLib, eci, CEF_ICON );
				return qfalse;
			}
			case ECM_GETCOMPID:			/* Get the external object name & id (from a sequential wParam) */
			{	
				if ( wParam==1 )
					return ECOreturnCompID( gInstLib, eci, CEF_ID, cObjType_Basic );
				return 0L;
			}
			case ECM_GETPROPNAME:
			{
				return ECOreturnProperties( gInstLib, eci, NULL, 0 );
			}                                                                                 
			case ECM_PROPERTYCANASSIGN:  	/* Object: Is the property assignable (ie readonly?) */
			case ECM_SETPROPERTY: 				/* Object: Assignment to a property */
			case ECM_GETPROPERTY:					/* Object: Retrieve value from property */
			{
				return 0L;
			}
			case WM_CREATE:
		    {
				//return 0;
				cef_client* object = (cef_client*)ECOfindObject( eci, hwnd );
				/*if ( object==NULL)
				{
                    object = new cef_client( hwnd, eci->mOmnisInstance );
				    //object->cef_create();
				    ECOinsertObject( eci, hwnd, (void*)object );
				}*/
				if (object != NULL)
				{
					object->cef_create();
				}
				return 0;

			}
			case WM_ERASEBKGND:
			{
				return 0;
			}
			case WM_NULL: //not getting this event
            {
			    hdc = BeginPaint(hwnd, &ps);
                
				/*cef_client* object = (cef_client*)ECOfindObject( eci, hwnd );
				if ( object!=NULL)
				{
					object->cef_paint();
				}*/
				EndPaint(hwnd, &ps);
				return 1;
			}
			case WM_PAINT:
			{
			    hdc = BeginPaint(hwnd, &ps);
                
				cef_client* object = (cef_client*)ECOfindObject( eci, hwnd );
				if ( object!=NULL)
				{
					object->cef_paint();
				}
				EndPaint(hwnd, &ps);
				return 1;
			}
	 }
	 return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}
