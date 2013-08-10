#include <stdio.h>

#include <windows.h>

#include "pythonutil.h"
#include "pyautocore.h"

using namespace pyauto;

LRESULT Hook_Clipboard_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DRAWCLIPBOARD:
		//PythonUtil_DebugPrintf("WM_DRAWCLIPBOARD\n");
		{
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

				if(g.clipboard_chain_next_window)
				{
					SendMessage(g.clipboard_chain_next_window, message, wParam, lParam );
				}

				enter = false;
			}
			else
			{
				PythonUtil_DebugPrintf("WM_DRAWCLIPBOARD nested!\n");
			}
		}
		break;

	case WM_CHANGECBCHAIN:
		//PythonUtil_DebugPrintf("WM_CHANGECBCHAIN\n");
		{
			if( (HWND)wParam==g.clipboard_chain_next_window )
			{
				g.clipboard_chain_next_window = (HWND)lParam;
			}
			else if( g.clipboard_chain_next_window!=NULL )
			{
				SendMessage( g.clipboard_chain_next_window, message, wParam, lParam );
			}
		}
		break;
	}

	return 0;
}

void HookStart_Clipboard()
{
	//PythonUtil_DebugPrintf("HookStart_Clipboard\n");

	if(g.clipboard_chain_next_window)
	{
		return;
	}

	g.clipboard_chain_next_window = SetClipboardViewer(g.pyauto_window);

	if( !g.clipboard_chain_next_window && GetLastError()!=0 )
	{
		PythonUtil_DebugPrintf("SetClipboardViewer() failed : %x\n", GetLastError() );
	}
}

void HookEnd_Clipboard()
{
	//PythonUtil_DebugPrintf("HookEnd_Clipboard\n");

	if(g.clipboard_chain_next_window)
	{
		ChangeClipboardChain( g.pyauto_window, g.clipboard_chain_next_window );
		g.clipboard_chain_next_window = NULL;
	}
}
