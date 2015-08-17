Chromium Embedded Framework (CEF3) running inside O$ 4.3.X
=======================

#### Requirements

+ Win 7+
+ O$ 4.3.X Non-Unicode
+ Visual Studio C++ 2010 (Express)
+ Omnis Externals SDK for O$ 4.3.X
+ [CEF3 for Windows 32-bit](https://cefbuilds.com) branch 2357 based on Chromium 43 including the debug symbols
+ [CMake](http://www.cmake.org/)

#### Installation

+ Unpack the CEF downloads.
+ Set environment variables:
  * `CEF_ROOT` =  `c:\path\to\cef_binary_3.2454.1304.ge979611_windows32` or similar
  * `OMNIS_XCOMP_ROOT` = `C:\Program Files (x86)\TigerLogic\OS4321\xcomp` or similar
  * `OMNIS_SDK_ROOT` = `C:\path\to\EXTCOMP_Source_Win32_4.3.2` or similar
+ Create a `CEFWebLib` directory in `%OMNIS_XCOMP_ROOT%`