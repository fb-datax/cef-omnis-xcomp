Chromium Embedded Framework (CEF3) running inside O$ 4.3.X
=======================

### Requirements

+ Win 7+
+ O$ 4.3.X Non-Unicode
+ Visual Studio 2013 (Express)
+ Omnis Externals SDK for O$ 4.3.X
+ [CEF3 for Windows 32-bit](https://cefbuilds.com) branch 2454 based on Chromium 45
+ [CMake](http://cmake.org/)

### Installation

+ Unpack the CEF downloads.
+ Set environment variables:
    * `CEF_ROOT` =  `c:\path\to\cef_binary_3.2454.1320.g807de3c_windows32` or similar. This must be the directory containing `README.txt`.
    * `OMNIS_XCOMP_ROOT` = `C:\Program Files (x86)\TigerLogic\OS4321\xcomp` or similar
    * `OMNIS_SDK_ROOT` = `C:\path\to\EXTCOMP_Source_Win32_4.3.2` or similar
+ Build the CEF wrapper:
    * Install [CMake](http://cmake.org/) if necessary.
    * Open a command prompt and enter `cd %CEF_ROOT% && cmake -G "Visual Studio 12" .`
    * Open the generated `%CEF_ROOT%\cef.sln` solution in Visual Studio 2013.
    * Build for both Release and Debug configurations. Either build the entire solution, or just right-click build the `libcef_dll_wrapper` project.

Note: Visual Studio must be run as administrator before opening `cef-omnis-xcomp.sln`. This is so that the post-build events can install into `OMNIS_XCOMP_ROOT`.

### XCOMP API

The following interface is available from within Omnis on the XCOMP.

#### Properties

**`$contextMenus`**

> A flag to control whether right-click context menus are enabled (defaults to `true`).

**`$traceLogConsole`**

> A flag to control whether javascript `console.log` messages are written to the Omnis trace log (defaults to `true`).

**`$cachePath`**

> The location where cache data will be stored on disk. If empty then browsers will be created in *incognito mode* where in-memory caches are used for storage and no data is persisted to disk. HTML5 databases such as localStorage will only persist across sessions if a cache path is specified. **Must be set only before the first call to navigateToUrl**.

#### Methods

**`$navigateToUrl(pUrl)`**

> Navigate to the given URL. `file://` URLs are supported.

**`$historyBack()`**, **`$historyForward()`**

> Equivalent to hitting the browser back or forward button respectively.

**`$sendCustomEvent(pName, pValue)`**

> Send a custom event to a javascript callback function which must have been registered under the same name with `omnis.setEventCallback` (see below).
For more details see the "Event Demo" in `demo.lbs`.

#### Events

**`CustomEvent(pName[, pParam1, pParam2, ..., pParam9])`**

> A custom event from javascript (see `omnis.customEvent()` below). Example:
```
On evCustomEvent
  Switch pName
    Case 'myCustomEvent'
      Send to trace log {[con('Received myCustomEvent. param = ', pParam1)]}
      Do $cfield.$sendCustomEvent('customResponse', 'Received myCustomEvent in Omnis.')
      Break to end of switch
    Case 'close'
      Do $cinst.$close()
      Break to end of switch
  End Switch
```

**`LoadingStateChange(pIsLoading, pCanGoBack, pCanGoForward)`**

> One of the indicated flags has changed:
> 
* `pIsLoading` - whether the page is currently loading.
* `pCanGoBack` - whether the browser back history button should be enabled.
* `pCanGoForward` - whether the browser forward history button should be enabled.

**`LoadStart()`**

> The page has started to load.

**`LoadEnd(pStatusCode)`**

> The page has finished loading with the given [HTTP status code](http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html).

**`LoadError(pStatusCode, pErrorMsg, pUrl)`**

> The given URL failed to load with the given error.

**`DownloadUpdate(pId, pIsComplete, pIsCanceled, pReceivedBytes, pTotalBytes, pCurrentSpeed, pFullPath)`**

> The given file is downloading, or has finished downloading.
>
* `pId` - the integer download identifier.
* `pIsComplete` - flag to indicate successful completion.
* `pIsCanceled` - flag to indicate user cancellation of the download.
* `pReceivedBytes` - the number of bytes downloaded.
* `pTotalBytes` - the total number of bytes in the download (if known).
* `pCurrentSpeed` - a simple speed estimate in bytes/s.
* `pFullPath` - the full path to the download file.

> Example:
```javascript
On evDownloadUpdate
  If pIsComplete
    Send to trace log {[con('Download complete: ', pFullPath)]}
    OK message Download (Icon,Sound bell) {Download complete: [pFullPath]}
  Else
    Send to trace log {[con('Downloading ', pFullPath, ' [',rnd(100*pReceivedBytes/pTotalBytes,1), '%] ', pCurrentSpeed, 'B/s')]}
  End If
```

**`TitleChange(pTitle)`**

> The browser title has changed.

**`AddressChange(pUrl)`**

> The given URL was navigated to.

**`GotFocus()`**

> The browser took the keyboard focus. The event should be handled with:
```
Queue set current field {[$cinst]}
```

### Javascript API

The following interface is available in javascript on the global `omnis` object.

**`omnis.showMsg(message[, bell, type])`**

> Show a message box dialog in Omnis studio.
> 
* `message` - the message string to display
* `bell` - determines whether the system bell will sound (defaults to `true`)
* `type` - must be one of:
    * `omnis.MSGBOX_OK`
    * `omnis.MSGBOX_YESNO`
    * `omnis.MSGBOX_NOYES`
    * `omnis.MSGBOXICON_OK` (the default)
    * `omnis.MSGBOXICON_YESNO`
    * `omnis.MSGBOXICON_NOYES`
    * `omnis.MSGBOXCANCEL_YESNO`
    * `omnis.MSGBOXCANCEL_NOYES`

**`omnis.traceLog(message)`**

> Write the given message to the tracelog. If multiple arguments are provided they are joined with spaces. Example:
```javascript
omnis.traceLog('pi =', Math.PI);
```

**`omnis.customEvent(name[, ...])`**

> Generates a `CustomEvent` event with the given name and parameters. Up to 9 parameters may be provided.
Example:
```javascript
omnis.customEvent('myCustomEvent', Math.PI);
```

**`omnis.setEventCallback(name, callback)`**

> Register the given callback function for custom events from Omnis with the given name. When Omnis calls `$sendCustomEvent` with this name, the callback will be invoked with a message string argument. Example:
```javascript
// set a callback for the custom event name 'customResponse'.
omnis.setEventCallback('customResponse', function(message) {
    console.log('Received "%s" from Omnis!', message);
});
```

**`omnis.clearEventCallback(name)`**

> Unregister the event callback function for the given custom event name.










