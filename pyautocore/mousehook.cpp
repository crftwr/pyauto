#include <stdio.h>

#include <windows.h>

#include "pythonutil.h"
#include "pyautocore.h"

static HHOOK mouse_hook = NULL;

using namespace pyauto;

static int OnMouseButton( WPARAM wParam, MSLLHOOKSTRUCT * pmousellhook, PyObject * pyfunc )
{
	int result;

	g.last_key_time = pmousellhook->time;

	DWORD vk;
	switch(wParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		vk = VK_LBUTTON;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		vk = VK_RBUTTON;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		vk = VK_MBUTTON;
		break;
	}

	PyObject * pyarglist = Py_BuildValue("(iii)", pmousellhook->pt.x, pmousellhook->pt.y, vk );
	PyObject * pyresult = PyEval_CallObject( pyfunc, pyarglist );
	Py_DECREF(pyarglist);
	if(pyresult)
	{
		if( pyresult==Py_None )
		{
			result = 0;
		}
		else
		{
			PyArg_Parse(pyresult,"i", &result );
		}
		Py_DECREF(pyresult);
	}
	else
	{
		PyErr_Print();
		result = 0;
	}

	return result;
}

static int OnMouseWheel(WPARAM wParam, MSLLHOOKSTRUCT * pmousellhook, PyObject * pyfunc)
{
	int result;

	g.last_key_time = pmousellhook->time;

	PyObject * pyarglist = Py_BuildValue("(iif)", pmousellhook->pt.x, pmousellhook->pt.y, ((float)(short)HIWORD(pmousellhook->mouseData))/WHEEL_DELTA);
	PyObject * pyresult = PyEval_CallObject(pyfunc, pyarglist);
	Py_DECREF(pyarglist);
	if (pyresult)
	{
		if (pyresult == Py_None)
		{
			result = 0;
		}
		else
		{
			PyArg_Parse(pyresult, "i", &result);
		}
		Py_DECREF(pyresult);
	}
	else
	{
		PyErr_Print();
		result = 0;
	}

	return result;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PythonUtil::GIL_Ensure gil_ensure;

	if(nCode<0)
	{
		LRESULT result = CallNextHookEx(mouse_hook, nCode, wParam, lParam);
		return result;
	}

	MSLLHOOKSTRUCT * pmousellhook = (MSLLHOOKSTRUCT*)lParam;

	// タイムスタンプが逆転してしまった場合は、予期しないことが起きている
	if( g.last_key_time > pmousellhook->time )
	{
		PythonUtil_Printf("Time stamp inversion happened.\n");
	}

	// プログラムによって挿入されたキーイベントはスクリプトで処理しない
	if( pmousellhook->flags & LLMHF_INJECTED
		&& pmousellhook->dwExtraInfo == (ULONG_PTR)g.module_handle )
	{
		LRESULT result = CallNextHookEx(mouse_hook, nCode, wParam, lParam);
		return result;
	}

	switch(wParam)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if( g.pyhook->mousedown )
		{
			int result = OnMouseButton( wParam, pmousellhook, g.pyhook->mousedown );
			if(result)
			{
				return result;
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if( g.pyhook->mouseup )
		{
			int result = OnMouseButton( wParam, pmousellhook, g.pyhook->mouseup );
			if(result)
			{
				return result;
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		if( g.pyhook->mousedblclk )
		{
			int result = OnMouseButton( wParam, pmousellhook, g.pyhook->mousedblclk );
			if(result)
			{
				return result;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (g.pyhook->mousewheel)
		{
			int result = OnMouseWheel(wParam, pmousellhook, g.pyhook->mousewheel);
			if (result)
			{
				return result;
			}
		}
		break;

	case WM_MOUSEHWHEEL:
		if (g.pyhook->mousehorizontalwheel)
		{
			int result = OnMouseWheel(wParam, pmousellhook, g.pyhook->mousehorizontalwheel);
			if (result)
			{
				return result;
			}
		}
		break;

	}

	LRESULT result = CallNextHookEx(mouse_hook, nCode, wParam, lParam);
	return result;
}

void HookStart_Mouse()
{
	if(!mouse_hook)
	{
		PythonUtil_DebugPrintf("SetWindowsHookEx\n" );
		mouse_hook = SetWindowsHookEx( WH_MOUSE_LL, MouseHookProc, g.module_handle, 0 );
	}

	if( mouse_hook==NULL )
	{
		PythonUtil_Printf("SetWindowsHookEx failed : %x\n", GetLastError() );
	}
}

void HookEnd_Mouse()
{
	if(mouse_hook)
	{
		if( ! UnhookWindowsHookEx(mouse_hook) )
		{
			PythonUtil_Printf("UnhookWindowsHookEx(mouse) failed\n");
		}

		mouse_hook=NULL;
	}
}
