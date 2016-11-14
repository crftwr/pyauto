#include <stdio.h>

#include <windows.h>

#include "pythonutil.h"
#include "pyautocore.h"

using namespace pyauto;

LRESULT Hook_Clipboard_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CLIPBOARDUPDATE:
		{
			//PythonUtil_DebugPrintf("WM_CLIPBOARDUPDATE\n");
		
			static bool enter = false;
			if(!enter)
			{
				enter = true;

				if(g.pyhook && g.pyhook->clipboard)
				{
					PyObject * pyarglist = Py_BuildValue("()");
					PyObject * pyresult = PyEval_CallObject( g.pyhook->clipboard, pyarglist );
					Py_DECREF(pyarglist);
					if(pyresult)
					{
						Py_DECREF(pyresult);
					}
					else
					{
						PyErr_Print();
					}
				}

				enter = false;
			}
			else
			{
				PythonUtil_DebugPrintf("WM_CLIPBOARDUPDATE nested!\n");
			}
		}
		break;
	}

	return 0;
}

void HookStart_Clipboard()
{
	//printf("HookStart_Clipboard\n");

	if(!AddClipboardFormatListener(g.pyauto_window))
	{
		PythonUtil_DebugPrintf("AddClipboardFormatListener() failed : %x\n", GetLastError() );
	}
}

void HookEnd_Clipboard()
{
	//printf("HookEnd_Clipboard\n");

	if (!RemoveClipboardFormatListener(g.pyauto_window))
	{
		PythonUtil_DebugPrintf("RemoveClipboardFormatListener() failed : %x\n", GetLastError());
	}
}
