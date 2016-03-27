#include <stdio.h>

#include <windows.h>

#include "pythonutil.h"
#include "pyautocore.h"

static HHOOK key_hook = NULL;

using namespace pyauto;

// WH_KEYBOARD_LL と SendInput の関係:
//
//   KeyHookProc() のなかで SendInput() を呼び出すと、
//   KeyHookProc() がネストして呼び出される。
//
LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PythonUtil::GIL_Ensure gil_ensure;

	if(nCode<0)
	{
		LRESULT result = CallNextHookEx(key_hook, nCode, wParam, lParam);
		return result;
	}

	KBDLLHOOKSTRUCT * pkbdllhook = (KBDLLHOOKSTRUCT*)lParam;

	// タイムスタンプが逆転してしまった場合は、予期しないことが起きている
	if( g.last_key_time > pkbdllhook->time )
	{
		PythonUtil_Printf("Time stamp inversion happened.\n");
	}

	// 自分の SendInput によって挿入されたキーイベントはスクリプトで処理しない。
	// vkCode==0 のイベントは特別扱いし、必ず Python で処理する。
	if( pkbdllhook->flags & LLKHF_INJECTED
		&& pkbdllhook->dwExtraInfo == (ULONG_PTR)g.module_handle
		&& pkbdllhook->vkCode )
	{
		LRESULT result = CallNextHookEx(key_hook, nCode, wParam, lParam);
		return result;
	}

	switch(wParam)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if(g.pyhook->keydown)
		{
			DWORD vk = pkbdllhook->vkCode;
			DWORD scan = pkbdllhook->scanCode;
			g.last_key_time = pkbdllhook->time;
			
			PyObject * pyarglist = Py_BuildValue("(ii)", vk, scan );
			PyObject * pyresult = PyEval_CallObject( g.pyhook->keydown, pyarglist );
			Py_DECREF(pyarglist);
			if(pyresult)
			{
				int result;
				if( pyresult==Py_None )
				{
					result = 0;
				}
				else
				{
					PyArg_Parse(pyresult,"i", &result );
				}
				Py_DECREF(pyresult);

				if(result)
				{
					return result;
				}
			}
			else
			{
				PyErr_Print();
			}
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if(g.pyhook->keyup)
		{
			DWORD vk = pkbdllhook->vkCode;
			DWORD scan = pkbdllhook->scanCode;
			g.last_key_time = pkbdllhook->time;
			
			PyObject * pyarglist = Py_BuildValue("(ii)", vk, scan );
			PyObject * pyresult = PyEval_CallObject( g.pyhook->keyup, pyarglist );
			Py_DECREF(pyarglist);
			if(pyresult)
			{
				int result;
				if( pyresult==Py_None )
				{
					result = 0;
				}
				else
				{
					PyArg_Parse(pyresult,"i", &result );
				}
				Py_DECREF(pyresult);

				if(result)
				{
					return result;
				}
			}
			else
			{
				PyErr_Print();
			}
		}
		break;
	}

	LRESULT result = CallNextHookEx(key_hook, nCode, wParam, lParam);
	return result;
}

void HookStart_Key()
{
	if(!key_hook)
	{
		PythonUtil_DebugPrintf("SetWindowsHookEx\n" );
		key_hook = SetWindowsHookEx( WH_KEYBOARD_LL, KeyHookProc, g.module_handle, 0 );
	}

	if( key_hook==NULL )
	{
		PythonUtil_DebugPrintf("SetWindowsHookEx failed : %x\n", GetLastError() );
	}
}

void HookEnd_Key()
{
	if(key_hook)
	{
		if( ! UnhookWindowsHookEx(key_hook) )
		{
			PythonUtil_DebugPrintf("UnhookWindowsHookEx(key) failed\n");
		}

		key_hook=NULL;
	}
}
