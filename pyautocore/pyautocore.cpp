#include <string>
#include <vector>
#include <list>

#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>

#include <python.h>

#include "kbhook.h"
#include "mousehook.h"
#include "clipboard.h"
#include "pythonutil.h"
#include "pyautocore.h"

using namespace std;

// ------------ const -----------------------------------
#define MODULE_NAME "pyautocore"
#define PYAUTO_WINDOW_NAME "pyauto"

// ------------ globals -----------------------------------

namespace pyauto
{
	static HANDLE hook_instance_mutex;

	Globals g;
};

using namespace pyauto;

// ------------ stringutil -----------------------------------

namespace pyauto
{
    #ifdef _UNICODE
    typedef wstring _tString;
    #else
    typedef string _tString;
    #endif

	namespace stringutil
	{
		static wstring MultiByteToWideChar( const char * str, int len )
		{
			int buf_size = len+2;
			WCHAR * buf = new WCHAR[ buf_size ];
			int write_len = ::MultiByteToWideChar( 0, 0, str, len, buf, buf_size );
			buf[write_len] = '\0';

			wstring ret = buf;

			delete [] buf;

			return ret;
		}

		static string WideCharToMultiByte( const WCHAR * str, int len )
        {
        	int buf_size = len*2+2;
        	char * buf = new char[ buf_size ];
        	int write_len = ::WideCharToMultiByte( 0, 0, str, len, buf, buf_size, NULL, NULL );
        	buf[write_len] = '\0';

        	string ret = buf;

        	delete [] buf;

        	return ret;
        }

        static bool PyStringTo_tString( const PyObject * pystr, _tString * str )
        {
        	if( PyUnicode_Check(pystr) )
        	{
                #if _UNICODE
        		*str = (const wchar_t*)PyUnicode_AS_UNICODE(pystr);
                #else
        		*str = WideCharToMultiByte( (const wchar_t*)PyUnicode_AS_UNICODE(pystr), PyUnicode_GET_SIZE(pystr) );
                #endif
        		return true;
        	}
        	else
        	{
        		*str = _T("");
        		return false;
        	}
        }


	};
};

// ------------ PyObject_Window -----------------------------------------

namespace pyauto
{
	static list<PyObject*> window_list;

	static PyObject * WindowObject_FromHWND( HWND hwnd )
	{
		if(!hwnd)
		{
			Py_INCREF(Py_None);
			return Py_None;
		}

		list<PyObject*>::iterator i;
		for( i=window_list.begin() ; i!=window_list.end() ; i++ )
		{
			if( ((PyObject_Window*)(*i))->hwnd==hwnd )
			{
				Py_INCREF(*i);
				return (*i);
			}
		}

		PyObject_Window * pywnd;
		pywnd = PyObject_New( PyObject_Window, &PyType_Window );
		pywnd->hwnd = hwnd;

		window_list.push_back((PyObject*)pywnd);

		return (PyObject*)pywnd;
	}

	static void WindowObject_Remove( PyObject * obj )
	{
		list<PyObject*>::iterator i;
		for( i=window_list.begin() ; i!=window_list.end() ; i++ )
		{
			if( (*i)==obj )
			{
				window_list.erase(i);
				return;
			}
		}
	}
};

static void Window_dealloc(PyObject* self)
{
	WindowObject_Remove(self);
	self->ob_type->tp_free(self);
}

static PyObject * Window_getText(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	int len = ::GetWindowTextLength(hwnd);

	TCHAR * buf = new TCHAR[len+2];
	::GetWindowText( hwnd, buf, len+2 );
	
#ifdef _UNICODE
	PyObject * pyret = Py_BuildValue("u",buf);
#else
	wstring wstr = stringutil::MultiByteToWideChar( buf, len );
	PyObject * pyret = Py_BuildValue("u",wstr.c_str());
#endif

	delete [] buf;

	return pyret;
}

static PyObject * Window_getClassName(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	TCHAR buf[256];
	::GetClassName( hwnd, buf, sizeof(buf) );

#ifdef _UNICODE
	PyObject * pyret = Py_BuildValue("u",buf);
#else
	wstring wstr = stringutil::MultiByteToWideChar( buf, lstrlen(buf) );
	PyObject * pyret = Py_BuildValue("u",wstr.c_str());
#endif

	return pyret;
}

static bool _Window_getProcessInfoFromHWND( HWND hwnd, TCHAR * process_name, TCHAR * process_path )
{
	// ヒープ情報をスナップしない秘密のビット
	#define TH32CS_SNAPNOHEAPS (0x40000000)

	DWORD pid = 0;
	GetWindowThreadProcessId( hwnd, &pid );

	bool ret = false;
	HANDLE process_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0 );
	if(process_snapshot!=INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 process_entry;
		process_entry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First( process_snapshot, &process_entry ))
		{
			while(true)
			{
				if(process_entry.th32ProcessID == pid)
				{
					if(process_name)
					{
						lstrcpy( process_name, process_entry.szExeFile );
						ret = true;
					}
					
					if(process_path)
					{
						HANDLE module_handle = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPNOHEAPS, process_entry.th32ProcessID );
						if(module_handle==INVALID_HANDLE_VALUE)
						{
							PyErr_Format( PyExc_WindowsError, "CreateToolhelp32Snapshot failed : lineno=%d errno=%d", __LINE__, GetLastError() );
							break;
						}

						MODULEENTRY32 module_entry;
						module_entry.dwSize = sizeof(MODULEENTRY32);
						if( Module32First( module_handle, &module_entry ) )
						{
							lstrcpy( process_path, module_entry.szExePath );
							ret = true;
						}
						else
						{
							PyErr_Format( PyExc_WindowsError, "Module32First failed : lineno=%d errno=%d", __LINE__, GetLastError() );
						}
					
						CloseHandle(module_handle);
					}
				
					break;
				}
	
				if(!Process32Next( process_snapshot, &process_entry ))
				{
					if(GetLastError()==ERROR_NO_MORE_FILES)
					{
						if(process_name)
						{
							lstrcpy( process_name, L"" );
						}
						if(process_path)
						{
							lstrcpy( process_path, L"" );
						}
						ret = true;
						break;
					}

					PyErr_Format( PyExc_WindowsError, "Process32Next failed : lineno=%d errno=%d", __LINE__, GetLastError() );
					break;
				}
			}
		}
		else
		{
			PyErr_Format( PyExc_WindowsError, "Process32First failed : lineno=%d errno=%d", __LINE__, GetLastError() );
		}

		CloseHandle(process_snapshot);
	}
	else
	{
		PyErr_Format( PyExc_WindowsError, "CreateToolhelp32Snapshot failed : lineno=%d errno=%d", __LINE__, GetLastError() );
	}

	return ret;
}

static PyObject * Window_getProcessName(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}
	
	TCHAR buf[MAX_PATH];
	bool ret = _Window_getProcessInfoFromHWND( hwnd, buf, NULL );
	if(!ret)
	{
		//PyErr_SetFromWindowsErr(0); // _Window_getProcessInfoFromHWND のなかでエラーをセットする
		
		return NULL;
	}

#ifdef _UNICODE
	PyObject * pyret = Py_BuildValue("u",buf);
#else
	wstring wstr = stringutil::MultiByteToWideChar( buf, lstrlen(buf) );
	PyObject * pyret = Py_BuildValue("u",wstr.c_str());
#endif

	return pyret;
}

static PyObject * Window_getProcessPath(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}
	
	TCHAR buf[MAX_PATH];
	bool ret = _Window_getProcessInfoFromHWND( hwnd, NULL, buf );
	if(!ret)
	{
		//PyErr_SetFromWindowsErr(0); // _Window_getProcessInfoFromHWND のなかでエラーをセットする
		
		return NULL;
	}

#ifdef _UNICODE
	PyObject * pyret = Py_BuildValue("u",buf);
#else
	wstring wstr = stringutil::MultiByteToWideChar( buf, lstrlen(buf) );
	PyObject * pyret = Py_BuildValue("u",wstr.c_str());
#endif

	return pyret;
}

static PyObject * Window_getHWND(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}
	
	PyObject * pyret = Py_BuildValue("i",hwnd);
	return pyret;
}

static PyObject * Window_getRect(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	RECT rect;
	::GetWindowRect(hwnd,&rect);

	PyObject * pyret = Py_BuildValue( "(iiii)", rect.left, rect.top, rect.right, rect.bottom );
	return pyret;
}

static PyObject * Window_setRect(PyObject* self, PyObject* args)
{
	RECT rect;

	if( ! PyArg_ParseTuple(args, "(iiii)", &rect.left, &rect.top, &rect.right, &rect.bottom ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if( ::MoveWindow( hwnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, true ) )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * Window_getClientRect(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	RECT rect;
	::GetClientRect(hwnd,&rect);

	PyObject * pyret = Py_BuildValue( "(iiii)", rect.left, rect.top, rect.right, rect.bottom );
	return pyret;
}

static PyObject * Window_clientToScreen(PyObject* self, PyObject* args)
{
	int x;
	int y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	POINT point;
	point.x = x;
	point.y = y;

	ClientToScreen( hwnd, &point );

	PyObject * pyret = Py_BuildValue( "(ii)", point.x, point.y );
	return pyret;
}

static PyObject * Window_getFirstChild(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetWindow( hwnd, GW_CHILD );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getLastChild(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetWindow( hwnd, GW_CHILD );
	hwnd = ::GetWindow( hwnd, GW_HWNDLAST );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getPrevious(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetWindow( hwnd, GW_HWNDPREV );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getNext(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetWindow( hwnd, GW_HWNDNEXT );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getParent(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetAncestor( hwnd, GA_PARENT );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getOwner(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetWindow( hwnd, GW_OWNER );
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_getLastActivePopup(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	hwnd = ::GetLastActivePopup(hwnd);
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_isVisible(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if(::IsWindowVisible( hwnd ))
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject * Window_isEnabled(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if(::IsWindowEnabled( hwnd ))
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject * Window_isMinimized(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if(::IsIconic( hwnd ))
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject * Window_isMaximized(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if(::IsZoomed( hwnd ))
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject * Window_minimize(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if( ::SendMessage( hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0 )==0 )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * Window_maximize(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if( ::SendMessage( hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 )==0 )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * Window_restore(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if( ::SendMessage( hwnd, WM_SYSCOMMAND, SC_RESTORE, 0 )==0 )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * Window_getCheck(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	LRESULT result = ::SendMessage( hwnd, BM_GETCHECK, 0, 0 );

	switch(result)
	{
	case BST_CHECKED:
		return Py_BuildValue( "i", 1 );

	case BST_INDETERMINATE:
		return Py_BuildValue( "i", 2 );

	default:
		return Py_BuildValue( "i", 0 );
	}
}

static PyObject * Window_setCheck(PyObject* self, PyObject* args)
{
	int state;

	if( ! PyArg_ParseTuple(args, "i", &state ) )
        return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	WPARAM bst;
	switch(state)
	{
	case 0:
		bst = BST_UNCHECKED;
		break;

	case 1:
		bst = BST_CHECKED;
		break;

	case 2:
		bst = BST_INDETERMINATE;
		break;

	default:
		PyErr_SetString( PyExc_ValueError, "invalid argument." );
		return NULL;
	}

	::SendMessage( hwnd, BM_SETCHECK, bst, 0 );

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Window_postMessage( PyObject * self, PyObject * args )
{
	int msg_id = 0;
	int msg_wparam = 0;
	int msg_lparam = 0;

	if( ! PyArg_ParseTuple(args,"i|ii", &msg_id, &msg_wparam, &msg_lparam ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	if(::PostMessage( hwnd, msg_id, msg_wparam, msg_lparam ))
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * Window_sendMessage( PyObject * self, PyObject * args )
{	
	int msg_id = 0;
	int msg_wparam = 0;
	int msg_lparam = 0;

	if( ! PyArg_ParseTuple(args,"i|ii", &msg_id, &msg_wparam, &msg_lparam ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	LRESULT result = ::SendMessage( hwnd, msg_id, msg_wparam, msg_lparam );

	return Py_BuildValue( "i", result );
}

static PyObject * Window_setForeground( PyObject * self, PyObject * args )
{
	int force = 0;

	if( ! PyArg_ParseTuple( args, "|i", &force ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}
	
	// 強制的なフォアグラウンド化
	{
		DWORD error = 0;
		
		int nTargetID, nForegroundID;
		DWORD sp_time = 0;
		
		bool thread_attached = false;
		bool fg_locktime_got = false;
		bool fg_locktime_set = false;

		// フォアグラウンドウィンドウを作成したスレッドのIDを取得
		nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

		// 目的のウィンドウを作成したスレッドのIDを取得
		nTargetID = GetWindowThreadProcessId(hwnd, NULL );

		if(force && nForegroundID!=nTargetID)
		{
			// スレッドのインプット状態を結び付ける
			if( AttachThreadInput( nTargetID, nForegroundID, TRUE ) )
			{
				thread_attached = true;
			}
			else
			{
				printf("AttachThreadInput failed %d\n",GetLastError());
			}

			// 現在の設定を sp_time に保存
			if( ! SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0) )
			{
				printf("SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT) failed %d\n",GetLastError());
				
				fg_locktime_got = true;
			}
			
			// ウィンドウの切り替え時間を 0ms にする
			if(fg_locktime_got)
			{
				if( ! SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0) )
				{
					printf("SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT) failed %d\n",GetLastError());

					fg_locktime_set = true;
				}
			}
		}

		// ウィンドウをフォアグラウンドに持ってくる
		if( ! SetForegroundWindow(hwnd) )
		{
			printf("SetForegroundWindow failed %d\n",GetLastError());
			
			error = GetLastError();
		}

		if(force && nForegroundID!=nTargetID)
		{
			// 設定を元に戻す
			if(fg_locktime_got && fg_locktime_set)
			{
				if( ! SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(void*)sp_time,0) )
				{
					printf("SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT) failed %d\n",GetLastError());
				}
			}

			// スレッドのインプット状態を切り離す
			if(thread_attached)
			{
				if( ! AttachThreadInput(nTargetID, nForegroundID, FALSE ) )
				{
					printf("AttachThreadInput failed %d\n",GetLastError());
				}
			}
		}
		
		// エラーが起きてたら例外を発行する
		if(error)
		{
			PyErr_SetFromWindowsErr(error);
			return NULL;
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Window_getForeground( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	HWND hwnd = ::GetForegroundWindow();
	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_setActive( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple( args, "" ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}
	
    SetActiveWindow(hwnd);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Window_getFocus( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	GUITHREADINFO info;
	info.cbSize = sizeof(info);

	if( !GetGUIThreadInfo( NULL, &info ) )
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	return WindowObject_FromHWND(info.hwndFocus);
}

static PyObject * Window_getCaret( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	GUITHREADINFO info;
	info.cbSize = sizeof(info);

	if( !GetGUIThreadInfo( NULL, &info ) )
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	PyObject * pyret = Py_BuildValue( "O(iiii)", WindowObject_FromHWND(info.hwndCaret), info.rcCaret.left, info.rcCaret.top, info.rcCaret.right, info.rcCaret.bottom );
	return pyret;
}

static PyObject * Window_find( PyObject * self, PyObject * args )
{
	PyObject * pywindow_class;
	PyObject * pywindow_title;

	if( ! PyArg_ParseTuple(args,"OO", &pywindow_class, &pywindow_title ) )
		return NULL;

	const TCHAR * p_window_class = NULL;
	const TCHAR * p_window_title = NULL;
	_tString window_class;
	_tString window_title;
	if(stringutil::PyStringTo_tString( pywindow_class, &window_class ))
		p_window_class = window_class.c_str();
	if(stringutil::PyStringTo_tString( pywindow_title, &window_title ))
		p_window_title = window_title.c_str();

	HWND hwnd = ::FindWindow( p_window_class, p_window_title );
	return WindowObject_FromHWND(hwnd);
}

struct _EnumWindowsData
{
	PyObject * func;
	PyObject * arg;
};

static BOOL CALLBACK _EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
	_EnumWindowsData * data = (_EnumWindowsData*)lParam;

	PyObject * pyarglist = Py_BuildValue("(OO)", WindowObject_FromHWND(hwnd), data->arg );
	PyObject * pyresult = PyEval_CallObject( data->func, pyarglist );
	Py_DECREF(pyarglist);
	if(pyresult)
	{
		int result;
		PyArg_Parse(pyresult,"i", &result );
		Py_DECREF(pyresult);
		return result;
	}
	else
	{
		PyErr_Print();
		return FALSE; //列挙を中断
	}
}

static PyObject * Window_enum( PyObject * self, PyObject * args )
{
	PyObject * func;
	PyObject * arg;

	if( ! PyArg_ParseTuple(args,"OO", &func, &arg ) )
		return NULL;

	_EnumWindowsData data;
	data.func = func;
	data.arg = arg;

	::EnumWindows( _EnumWindowsCallback, (LPARAM)&data );

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Window_getDesktop( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	HWND hwnd = ::GetDesktopWindow();

	return WindowObject_FromHWND(hwnd);
}

static PyObject * Window_fromHWND( PyObject * self, PyObject * args )
{
	HWND hwnd;

	if( ! PyArg_ParseTuple( args,"i", &hwnd ) )
		return NULL;

	return WindowObject_FromHWND(hwnd);
}

static BOOL CALLBACK _EnumMonitorCallback( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	PyObject * monitor_info_list = (PyObject*)dwData;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &mi);

	PyObject * monitor_info = Py_BuildValue( 
		"[[iiii][iiii]i]", 
		mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
		mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom,
		mi.dwFlags
		);
		
	PyList_Append( monitor_info_list, monitor_info );
	
	Py_XDECREF(monitor_info);
	
	return TRUE;
}

static PyObject * Window_getMonitorInfo( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	PyObject * monitor_info_list = PyList_New(0);

	EnumDisplayMonitors( NULL, NULL, _EnumMonitorCallback, (LPARAM)monitor_info_list );

	return monitor_info_list;
}

static void Bitmap_ConvertBGRAtoRGB( char * dst, const char * src, int w, int h, int wb )
{
	for( int y=0 ; y<h ; y++ )
	{
		for( int x=0 ; x<w ; x++ )
		{
			dst[ y*w*3 + x*3 + 0 ] = src[ y*wb + x*4 + 2 ];
			dst[ y*w*3 + x*3 + 1 ] = src[ y*wb + x*4 + 1 ];
			dst[ y*w*3 + x*3 + 2 ] = src[ y*wb + x*4 + 0 ];
		}
	}
}

static void Bitmap_ConvertBGRtoRGB( char * dst, const char * src, int w, int h, int wb )
{
	for( int y=0 ; y<h ; y++ )
	{
		for( int x=0 ; x<w ; x++ )
		{
			dst[ y*w*3 + x*3 + 0 ] = src[ y*wb + x*3 + 2 ];
			dst[ y*w*3 + x*3 + 1 ] = src[ y*wb + x*3 + 1 ];
			dst[ y*w*3 + x*3 + 2 ] = src[ y*wb + x*3 + 0 ];
		}
	}
}

static void Bitmap_ConvertBGR565toRGB( char * dst, const char * src, int w, int h, int wb )
{
	for( int y=0 ; y<h ; y++ )
	{
		for( int x=0 ; x<w ; x++ )
		{
			short src_pixel = *(short*)(&src[ y*wb + x*2 ]);
			dst[ y*w*3 + x*3 + 0 ] = ((src_pixel & (0x001f<<11))>>11) * 255 / 0x001f;
			dst[ y*w*3 + x*3 + 1 ] = ((src_pixel & (0x003f<<5))>>5) * 255 / 0x003f;
			dst[ y*w*3 + x*3 + 2 ] = ((src_pixel & (0x001f<<0))>>0) * 255 / 0x001f;
		}
	}
}

static void Bitmap_GetPixeltoRGB( char * dst, HDC dc, int w, int h )
{
	for( int y=0 ; y<h ; y++ )
	{
		for( int x=0 ; x<w ; x++ )
		{
			COLORREF color = ::GetPixel( dc, x, y );
			dst[ y*w*3 + x*3 + 0 ] = GetRValue(color);
			dst[ y*w*3 + x*3 + 1 ] = GetGValue(color);
			dst[ y*w*3 + x*3 + 2 ] = GetBValue(color);
		}
	}
}

static PyObject * Window_getImage( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	RECT rect;
	::GetWindowRect(hwnd,&rect);

	if( rect.right-rect.left<=0 || rect.bottom-rect.top<=0 )
	{
		PyObject_Image * pyimg;
		pyimg = PyObject_New( PyObject_Image, &PyType_Image );
		pyimg->w = 0;
		pyimg->h = 0;
		pyimg->buf = malloc(0);
		pyimg->bufsize = 0;
		return (PyObject*)pyimg;
	}

	HDC dc_src = ::GetWindowDC(hwnd);
	HDC dc_dst = ::CreateCompatibleDC(dc_src);
	HBITMAP bitmap = ::CreateCompatibleBitmap( dc_src, rect.right-rect.left, rect.bottom-rect.top );

	::SelectObject( dc_dst, bitmap );
	::SetMapMode( dc_dst, MM_TEXT );
	::BitBlt( dc_dst, 0, 0, rect.right-rect.left, rect.bottom-rect.top, dc_src, 0, 0, SRCCOPY );

	BITMAP bm;
	::GetObject( bitmap, sizeof(bm), &bm );

	unsigned int bufsize = bm.bmWidthBytes * bm.bmHeight;
	void * buf = malloc(bufsize);

	::GetBitmapBits( bitmap, bufsize, buf );

	PyObject_Image * pyimg;
	pyimg = PyObject_New( PyObject_Image, &PyType_Image );
	pyimg->w = bm.bmWidth;
	pyimg->h = bm.bmHeight;
	pyimg->bufsize = pyimg->w * 3 * pyimg->h;
	pyimg->buf = malloc(pyimg->bufsize);

	if(bm.bmBitsPixel==32)
	{
		Bitmap_ConvertBGRAtoRGB( (char*)pyimg->buf, (const char*)buf, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes );
	}
	else if(bm.bmBitsPixel==24)
	{
		Bitmap_ConvertBGRtoRGB( (char*)pyimg->buf, (const char*)buf, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes );
	}
	else if(bm.bmBitsPixel==16)
	{
		Bitmap_ConvertBGR565toRGB( (char*)pyimg->buf, (const char*)buf, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes );
	}
	else
	{
		Bitmap_GetPixeltoRGB( (char*)pyimg->buf, dc_dst, bm.bmWidth, bm.bmHeight );
	}

	free(buf);

	DeleteObject(bitmap);
	DeleteDC(dc_dst);
	ReleaseDC(hwnd,dc_src);

	return (PyObject*)pyimg;
}

static PyObject * Window_getImeStatus( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"" ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	#define IMC_GETOPENSTATUS 5
    HWND hwnd_ime = ImmGetDefaultIMEWnd(hwnd);
	if( SendMessage( hwnd_ime, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0 )!=0 )
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}

static PyObject * Window_setImeStatus( PyObject * self, PyObject * args )
{
	int open;
	
	if( ! PyArg_ParseTuple( args, "i", &open ) )
		return NULL;

	HWND hwnd = ((PyObject_Window*)self)->hwnd;
	if(!hwnd)
	{
		PyErr_SetString( PyExc_ValueError, "invalid window object." );
		return NULL;
	}

	#define IMC_SETOPENSTATUS 6
    HWND hwnd_ime = ImmGetDefaultIMEWnd(hwnd);
	SendMessage( hwnd_ime, WM_IME_CONTROL, IMC_SETOPENSTATUS, open ? 1 : 0 );

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef Window_methods[] = {
    { "getText", Window_getText, METH_VARARGS, "" },
    { "getClassName", Window_getClassName, METH_VARARGS, "" },
    { "getProcessName", Window_getProcessName, METH_VARARGS, "" },
    { "getProcessPath", Window_getProcessPath, METH_VARARGS, "" },
    { "getHWND", Window_getHWND, METH_VARARGS, "" },
    { "getRect", Window_getRect, METH_VARARGS, "" },
    { "setRect", Window_setRect, METH_VARARGS, "" },
    { "getClientRect", Window_getClientRect, METH_VARARGS, "" },
    { "clientToScreen", Window_clientToScreen, METH_VARARGS, "" },
    { "getFirstChild", Window_getFirstChild, METH_VARARGS, "" },
    { "getLastChild", Window_getLastChild, METH_VARARGS, "" },
    { "getPrevious", Window_getPrevious, METH_VARARGS, "" },
    { "getNext", Window_getNext, METH_VARARGS, "" },
    { "getParent", Window_getParent, METH_VARARGS, "" },
	{ "getOwner", Window_getOwner, METH_VARARGS, "" },
    { "getLastActivePopup", Window_getLastActivePopup, METH_VARARGS, "" },
    { "isVisible", Window_isVisible, METH_VARARGS, "" },
    { "isEnabled", Window_isEnabled, METH_VARARGS, "" },
    { "isMinimized", Window_isMinimized, METH_VARARGS, "" },
    { "isMaximized", Window_isMaximized, METH_VARARGS, "" },
    { "minimize", Window_minimize, METH_VARARGS, "" },
    { "maximize", Window_maximize, METH_VARARGS, "" },
    { "restore", Window_restore, METH_VARARGS, "" },
    { "getCheck", Window_getCheck, METH_VARARGS, "" },
    { "setCheck", Window_setCheck, METH_VARARGS, "" },
    { "postMessage", Window_postMessage, METH_VARARGS, "" },
    { "sendMessage", Window_sendMessage, METH_VARARGS, "" },
    { "setForeground", Window_setForeground, METH_VARARGS, "" },
    { "getForeground", Window_getForeground, METH_STATIC|METH_VARARGS, "" },
    { "setActive", Window_setActive, METH_VARARGS, "" },
    { "getFocus", Window_getFocus, METH_STATIC|METH_VARARGS, "" },
	{ "getCaret", Window_getCaret, METH_STATIC|METH_VARARGS, "" },
    { "find", Window_find, METH_STATIC|METH_VARARGS, "" },
    { "enum", Window_enum, METH_STATIC|METH_VARARGS, "" },
    { "getDesktop", Window_getDesktop, METH_STATIC|METH_VARARGS, "" },
    { "fromHWND", Window_fromHWND, METH_STATIC|METH_VARARGS, "" },
    { "getMonitorInfo", Window_getMonitorInfo, METH_STATIC|METH_VARARGS, "" },
    { "getImage", Window_getImage, METH_VARARGS, "" },
    { "getImeStatus", Window_getImeStatus, METH_VARARGS, "" },
    { "setImeStatus", Window_setImeStatus, METH_VARARGS, "" },
	{NULL,NULL}
};

PyTypeObject pyauto::PyType_Window = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"Window",			/* tp_name */
	sizeof(PyObject_Window), /* tp_basicsize */
	0,					/* tp_itemsize */
	Window_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, 					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,/* tp_getattro */
	PyObject_GenericSetAttr,/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	"",					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	Window_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	PyType_GenericNew,	/* tp_new */
	0,					/* tp_free */
};

// ------------ PyObject_Image -----------------------------------------

static void Image_dealloc(PyObject* self)
{
    free(((PyObject_Image*)self)->buf);
	self->ob_type->tp_free(self);
}

static PyObject * Image_getSize(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;
	return Py_BuildValue( "(ii)", ((PyObject_Image*)self)->w, ((PyObject_Image*)self)->h );
}

static PyObject * Image_getMode(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;
	return Py_BuildValue( "s", "RGB" );
}

static PyObject * Image_getBuffer(PyObject* self, PyObject* args)
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;
	return Py_BuildValue( "y#", ((PyObject_Image*)self)->buf, ((PyObject_Image*)self)->bufsize );
}

static PyObject * Image_fromString( PyObject * self, PyObject * args )
{
	const char * mode;
	int width;
	int height;
	const char * buf;
	unsigned int bufsize;

	if( ! PyArg_ParseTuple(args,"s(ii)s#", &mode, &width, &height, &buf, &bufsize ) )
		return NULL;

	if( strcmp( mode, "RGB" )!=0 )
	{
		PyErr_SetString( PyExc_ValueError, "image mode unsupported." );
		return NULL;
	}

	if( (unsigned)(width * 3 * height) > bufsize )
	{
		PyErr_SetString( PyExc_ValueError, "insufficient buffer length." );
		return NULL;
	}

	if( width<=0 || height<=0 )
	{
		PyObject_Image * pyimg;
		pyimg = PyObject_New( PyObject_Image, &PyType_Image );
		pyimg->w = 0;
		pyimg->h = 0;
		pyimg->buf = malloc(0);
		pyimg->bufsize = 0;
		return (PyObject*)pyimg;
	}

	PyObject_Image * pyimg;
	pyimg = PyObject_New( PyObject_Image, &PyType_Image );
	pyimg->w = width;
	pyimg->h = height;
	pyimg->bufsize = pyimg->w * 3 * pyimg->h;
	pyimg->buf = malloc(pyimg->bufsize);

	memcpy( pyimg->buf, buf, pyimg->bufsize );

	return (PyObject*)pyimg;
}

static PyObject * Image_find( PyObject * self, PyObject * args )
{
	PyObject * obj;

	if( ! PyArg_ParseTuple(args,"O", &obj ) )
		return NULL;

	if( ! TypeCheck_Image(obj) )
	{
		PyErr_SetString( PyExc_TypeError, "first argument must be a pyautocore.Image object." );
		return NULL;
	}

	PyObject_Image * img1 = (PyObject_Image*)self;
	PyObject_Image * img2 = (PyObject_Image*)obj;

	bool found = false;
	int x, y;

	for( y=0 ; y<=img1->h-img2->h ; y++ )
	{
		for( x=0 ; x<=img1->w-img2->w ; x++ )
		{
			bool failed = false;

			for( int y2=0 ; y2<img2->h ; y2++ )
			{
				for( int x2=0 ; x2<img2->w ; x2++ )
				{
					int x1 = x+x2;
					int y1 = y+y2;
					if( ((const char*)img1->buf)[ y1 * img1->w * 3 + x1 * 3 + 0 ] != ((const char*)img2->buf)[ y2 * img2->w * 3 + x2 * 3 + 0 ]
					 || ((const char*)img1->buf)[ y1 * img1->w * 3 + x1 * 3 + 1 ] != ((const char*)img2->buf)[ y2 * img2->w * 3 + x2 * 3 + 1 ]
					 || ((const char*)img1->buf)[ y1 * img1->w * 3 + x1 * 3 + 2 ] != ((const char*)img2->buf)[ y2 * img2->w * 3 + x2 * 3 + 2 ] )
					{
						failed = true;
					}

					if(failed) break;
				}

				if(failed) break;
			}

			if( ! failed )
			{
				found = true;
				break;
			}
		}

		if(found) break;
	}

	if(found)
	{
		PyObject * pyret = Py_BuildValue( "(ii)", x, y );
		return pyret;
	}
	else
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
}

static PyMethodDef Image_methods[] = {
	{ "getSize", Image_getSize, METH_VARARGS, "" },
	{ "getMode", Image_getMode, METH_VARARGS, "" },
	{ "getBuffer", Image_getBuffer, METH_VARARGS, "" },
	{ "fromString", Image_fromString, METH_STATIC|METH_VARARGS, "" },
	{ "find", Image_find, METH_VARARGS, "" },
	{NULL,NULL}
};

PyTypeObject pyauto::PyType_Image = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"Image",			/* tp_name */
	sizeof(PyObject_Image), /* tp_basicsize */
	0,					/* tp_itemsize */
	Image_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, 					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,/* tp_getattro */
	PyObject_GenericSetAttr,/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	"",					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	Image_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	PyType_GenericNew,	/* tp_new */
	0,					/* tp_free */
};

// ------------ PyObject_Input -----------------------------------------

static int Input_init( PyObject * self, PyObject * args, PyObject * kwds)
{
	if( ! PyArg_ParseTuple( args, "" ) )
        return -1;

	((PyObject_Input*)self)->num = 0;

	return 0;
}

#if !defined(_MSC_VER)
# define _snprintf_s( a, b, c, ... ) snprintf( a, b, __VA_ARGS__ )
#endif

static PyObject * Input_repr( PyObject * self )
{
	char buf[64];
	buf[0] = 0;

	switch( ((PyObject_Input*)self)->num )
	{
	case 0:
		break;
	
	case 1:
		{
			INPUT & input = ((PyObject_Input*)self)->input[0];
			
			switch(input.type)
			{
			case INPUT_KEYBOARD:
				{
					if( input.ki.dwFlags & KEYEVENTF_UNICODE )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "Char(%d)", input.ki.wScan );
					}
					else if( input.ki.dwFlags & KEYEVENTF_KEYUP )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "KeyUp(%d)", input.ki.wVk );
					}
					else
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "KeyDown(%d)", input.ki.wVk );
					}
				}
				break;

			case INPUT_MOUSE:
				{
					if( input.mi.dwFlags & MOUSEEVENTF_LEFTDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseLeftDown(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_LEFTUP )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseLeftUp(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_RIGHTDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseRightDown(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_RIGHTUP )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseRightUp(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_MIDDLEDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseMiddleDown(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_MIDDLEUP )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseMiddleUp(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_WHEEL )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseWheel(%d,%d,%d)", input.mi.dx, input.mi.dy, input.mi.mouseData );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_HWHEEL )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseHorizontalWheel(%d,%d,%d)", input.mi.dx, input.mi.dy, input.mi.mouseData );
					}
					else
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseMove(%d,%d)", input.mi.dx, input.mi.dy );
					}
				}
				break;
			}
		}
		break;
	
	case 2:
		{
			INPUT & input = ((PyObject_Input*)self)->input[0];

			switch(input.type)
			{
			case INPUT_KEYBOARD:
				{
					_snprintf_s( buf, sizeof(buf), _TRUNCATE, "Key(%d)", input.ki.wVk );
				}
				break;

			case INPUT_MOUSE:
				{
					if( input.mi.dwFlags & MOUSEEVENTF_LEFTDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseLeftClick(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_RIGHTDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseRightClick(%d,%d)", input.mi.dx, input.mi.dy );
					}
					else if( input.mi.dwFlags & MOUSEEVENTF_MIDDLEDOWN )
					{
						_snprintf_s( buf, sizeof(buf), _TRUNCATE, "MouseMiddleClick(%d,%d)", input.mi.dx, input.mi.dy );
					}
				}
				break;
			}	
		}
		break;
	}
	
	if(buf[0]==0)
	{
		_snprintf_s( buf, sizeof(buf), _TRUNCATE, "<unknown Input object>" );
	}

	PyObject * pyret = Py_BuildValue( "s", buf );
	return pyret;
}

static int IsExtendedKey( int vk )
{
	switch(vk)
	{
	case VK_CANCEL:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_END:
	case VK_HOME:
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_SNAPSHOT:
	case VK_INSERT:
	case VK_DELETE:
	case VK_DIVIDE:
	case VK_NUMLOCK:
	case VK_RSHIFT:
	case VK_RCONTROL:
	case VK_RMENU:
		return true;
	
	default:
		return false;
	}
}

static PyObject * Input_setKeyDown(PyObject* self, PyObject* args)
{
	int vk;

	if( ! PyArg_ParseTuple(args, "i", &vk ) )
        return NULL;

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = (WORD)vk;
	input.ki.wScan = MapVirtualKey(vk,0);
	input.ki.dwFlags = (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
	input.ki.dwExtraInfo = 0;
	input.ki.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setKeyUp(PyObject* self, PyObject* args)
{
	int vk;

	if( ! PyArg_ParseTuple(args, "i", &vk ) )
        return NULL;

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = (WORD)vk;
	input.ki.wScan = MapVirtualKey(vk,0);
	input.ki.dwFlags = KEYEVENTF_KEYUP | (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
	input.ki.dwExtraInfo = 0;
	input.ki.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setKey(PyObject* self, PyObject* args)
{
	int vk;

	if( ! PyArg_ParseTuple(args, "i", &vk ) )
        return NULL;

	((PyObject_Input*)self)->num = 2;

	{
		INPUT & input = ((PyObject_Input*)self)->input[0];
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = (WORD)vk;
		input.ki.wScan = MapVirtualKey(vk,0);
		input.ki.dwFlags = (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
		input.ki.dwExtraInfo = 0;
		input.ki.time = 0;
	}

	{
		INPUT & input = ((PyObject_Input*)self)->input[1];
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = (WORD)vk;
		input.ki.wScan = MapVirtualKey(vk,0);
		input.ki.dwFlags = KEYEVENTF_KEYUP | (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
		input.ki.dwExtraInfo = 0;
		input.ki.time = 0;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setChar(PyObject* self, PyObject* args)
{
	int unicode;

	if( ! PyArg_ParseTuple(args, "i", &unicode ) )
        return NULL;

	((PyObject_Input*)self)->num = 2;

	{
		INPUT & input = ((PyObject_Input*)self)->input[0];
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0;
		input.ki.wScan = unicode;
		input.ki.dwFlags = KEYEVENTF_UNICODE;
		input.ki.dwExtraInfo = 0;
		input.ki.time = 0;
	}

	{
		INPUT & input = ((PyObject_Input*)self)->input[1];
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0;
		input.ki.wScan = unicode;
		input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		input.ki.dwExtraInfo = 0;
		input.ki.time = 0;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static inline int sign( const int & i )
{
	if(i==0) return 0;
	else if(i>0) return 1;
	else return -1;
}

static inline void MousePositionAsPixel( int * x, int * y )
{
	*x = (int)(( *x + 0.5f * sign(*x) ) * 65536 / (GetSystemMetrics(SM_CXSCREEN)));
	*y = (int)(( *y + 0.5f * sign(*y) ) * 65536 / (GetSystemMetrics(SM_CYSCREEN)));
}

static PyObject * Input_setMouseMove(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseLeftDown(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseLeftUp(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseLeftClick(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 2;

	{
		INPUT & input = ((PyObject_Input*)self)->input[0];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	{
		INPUT & input = ((PyObject_Input*)self)->input[1];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseRightDown(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseRightUp(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseRightClick(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 2;

	{
		INPUT & input = ((PyObject_Input*)self)->input[0];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	{
		INPUT & input = ((PyObject_Input*)self)->input[1];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseMiddleDown(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseMiddleUp(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseMiddleClick(PyObject* self, PyObject* args)
{
	int x,y;

	if( ! PyArg_ParseTuple(args, "ii", &x, &y ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 2;

	{
		INPUT & input = ((PyObject_Input*)self)->input[0];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	{
		INPUT & input = ((PyObject_Input*)self)->input[1];
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.mouseData = 0;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP;
		input.mi.dwExtraInfo = 0;
		input.mi.time = 0;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseWheel(PyObject* self, PyObject* args)
{
	int x,y;
	float wheel;

	if( ! PyArg_ParseTuple(args, "iif", &x, &y, &wheel ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = (DWORD)(wheel * WHEEL_DELTA);
	input.mi.dwFlags = MOUSEEVENTF_WHEEL | MOUSEEVENTF_ABSOLUTE;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_setMouseHorizontalWheel(PyObject* self, PyObject* args)
{
	int x,y;
	float wheel;

	if( ! PyArg_ParseTuple(args, "iif", &x, &y, &wheel ) )
        return NULL;

	MousePositionAsPixel( &x, &y );

	((PyObject_Input*)self)->num = 1;

	INPUT & input = ((PyObject_Input*)self)->input[0];
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = (DWORD)(wheel * WHEEL_DELTA);
	input.mi.dwFlags = MOUSEEVENTF_HWHEEL | MOUSEEVENTF_ABSOLUTE;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_send( PyObject * self, PyObject * args )
{
	PyObject * py_input_list;

	if( ! PyArg_ParseTuple(args,"O", &py_input_list ) )
		return NULL;

	if( !PyTuple_Check(py_input_list) && !PyList_Check(py_input_list) )
	{
		PyErr_SetString( PyExc_TypeError, "argument must be a tuple or list." );
		return NULL;
	}

	int item_num = PySequence_Length(py_input_list);

	INPUT * input = new INPUT[ item_num * 2 ];
	int num_input = 0;

	for( int i=0 ; i<item_num ; i++ )
	{
		PyObject * pyitem = PySequence_GetItem( py_input_list, i );

		if( !TypeCheck_Input(pyitem) )
		{
			PyErr_SetString( PyExc_TypeError, "argument must be a sequence of Input object." );
			delete [] input;
			return NULL;
		}

		// 擬似入力のタイムスタンプを、最後に受けたキーのタイムスタンプにする
        for( int k=0 ; k<((PyObject_Input*)pyitem)->num ; k++ )
		{
			input[num_input] = ((PyObject_Input*)pyitem)->input[k];
			
			switch(input[num_input].type)
			{
			case INPUT_MOUSE:
				input[num_input].mi.time = g.last_key_time;
				break;

			case INPUT_KEYBOARD:
				input[num_input].ki.time = g.last_key_time;
				break;
			}

			++num_input;
		}

		Py_XDECREF(pyitem);
	}

	UINT num_sent = SendInput( num_input, input, sizeof(INPUT) );

	delete [] input;
	
	if( num_sent != num_input )
	{
		PyErr_Format( PyExc_WindowsError, "SendInput failed : num_input=%d, num_sent=%d, errno=%d", num_input, num_sent, GetLastError() );
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_getCursorPos( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"") )
		return NULL;
	
	POINT pos;
	GetCursorPos(&pos);

	PyObject * pyret = Py_BuildValue( "(ii)", pos.x, pos.y );
	return pyret;
}

static PyObject * Input_getKeyboardState( PyObject * self, PyObject * args )
{
	if( ! PyArg_ParseTuple(args,"") )
		return NULL;
	
	BYTE buf[256];
	if( ! GetKeyboardState(buf) )
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	PyObject * pyret = Py_BuildValue( "y#", buf, sizeof(buf) );
	return pyret;
}

static PyObject * Input_setKeyboardState( PyObject * self, PyObject * args )
{
	BYTE * buf;
	unsigned int bufsize;

	if( ! PyArg_ParseTuple( args, "s#", &buf, &bufsize ) )
		return NULL;
	
	if( ! SetKeyboardState(buf) )
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Input_getKeyState( PyObject * self, PyObject * args )
{
	int vk;
	if( ! PyArg_ParseTuple(args,"i",&vk) )
		return NULL;
	
	SHORT state = GetKeyState(vk);

	PyObject * pyret = Py_BuildValue( "H", (USHORT)state );
	return pyret;
}

static PyObject * Input_getAsyncKeyState( PyObject * self, PyObject * args )
{
	int vk;
	if( ! PyArg_ParseTuple(args,"i",&vk) )
		return NULL;
	
	SHORT state = GetAsyncKeyState(vk);

	PyObject * pyret = Py_BuildValue( "H", (USHORT)state );
	return pyret;
}

static PyMethodDef Input_methods[] = {
    { "setKeyDown", Input_setKeyDown, METH_VARARGS, "" },
    { "setKeyUp", Input_setKeyUp, METH_VARARGS, "" },
    { "setKey", Input_setKey, METH_VARARGS, "" },
    { "setChar", Input_setChar, METH_VARARGS, "" },
    { "setMouseMove", Input_setMouseMove, METH_VARARGS, "" },
    { "setMouseLeftDown", Input_setMouseLeftDown, METH_VARARGS, "" },
    { "setMouseLeftUp", Input_setMouseLeftUp, METH_VARARGS, "" },
    { "setMouseLeftClick", Input_setMouseLeftClick, METH_VARARGS, "" },
    { "setMouseRightDown", Input_setMouseRightDown, METH_VARARGS, "" },
    { "setMouseRightUp", Input_setMouseRightUp, METH_VARARGS, "" },
    { "setMouseRightClick", Input_setMouseRightClick, METH_VARARGS, "" },
    { "setMouseMiddleDown", Input_setMouseMiddleDown, METH_VARARGS, "" },
    { "setMouseMiddleUp", Input_setMouseMiddleUp, METH_VARARGS, "" },
    { "setMouseMiddleClick", Input_setMouseMiddleClick, METH_VARARGS, "" },
    { "setMouseWheel", Input_setMouseWheel, METH_VARARGS, "" },
	{ "setMouseHorizontalWheel", Input_setMouseHorizontalWheel, METH_VARARGS, "" },
    { "send", Input_send, METH_STATIC|METH_VARARGS, "" },
	{ "getCursorPos", Input_getCursorPos, METH_STATIC|METH_VARARGS, "" },
	{ "getKeyboardState", Input_getKeyboardState, METH_STATIC|METH_VARARGS, "" },
	{ "setKeyboardState", Input_setKeyboardState, METH_STATIC|METH_VARARGS, "" },
	{ "getKeyState", Input_getKeyState, METH_STATIC|METH_VARARGS, "" },
	{ "getAsyncKeyState", Input_getAsyncKeyState, METH_STATIC|METH_VARARGS, "" },
	{NULL,NULL}
};

PyTypeObject pyauto::PyType_Input = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"Input",			/* tp_name */
	sizeof(PyType_Input), /* tp_basicsize */
	0,					/* tp_itemsize */
	0,					/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	Input_repr,			/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,/* tp_getattro */
	PyObject_GenericSetAttr,/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	"",					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	Input_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	Input_init,			/* tp_init */
	0,					/* tp_alloc */
	PyType_GenericNew,	/* tp_new */
	0,					/* tp_free */
};

// ------------ PyObject_Hook -----------------------------------------

// フックのフラグを、擬似入力のフラグに変換する
static int flags_conv( DWORD flags )
{
	int ret = 0;
	if( flags & LLKHF_EXTENDED ) ret |= KEYEVENTF_EXTENDEDKEY;
	if( flags & LLKHF_UP ) ret |= KEYEVENTF_KEYUP;
	return ret;
}

static LRESULT CALLBACK Hook_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PythonUtil::GIL_Ensure gil_ensure;

	switch(message)
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

	case WM_DRAWCLIPBOARD:
	case WM_CHANGECBCHAIN:
		return Hook_Clipboard_wndProc(hWnd, message, wParam, lParam );
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static int Hook_init( PyObject * self, PyObject * args, PyObject * kwds)
{
	if( ! PyArg_ParseTuple( args, "" ) )
        return -1;

	// Hookオブジェクトはシステム上に1つだけ
	hook_instance_mutex = CreateMutex(NULL, TRUE, _T(PYAUTO_WINDOW_NAME) );
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		PyErr_SetString( PyExc_ValueError, "multiple pyauto.Hook objects cannot exist on a system." );
		hook_instance_mutex = NULL;
		return -1;
	}

	((PyObject_Hook*)self)->keydown = 0;
	((PyObject_Hook*)self)->keyup = 0;
	((PyObject_Hook*)self)->mousedown = 0;
	((PyObject_Hook*)self)->mouseup = 0;
	((PyObject_Hook*)self)->mousedblclk = 0;
	((PyObject_Hook*)self)->mousemove = 0;
	((PyObject_Hook*)self)->mousewheel = 0;
	((PyObject_Hook*)self)->mousehorizontalwheel = 0;
	((PyObject_Hook*)self)->clipboard = 0;

	// ウィンドウクラスの登録
	{
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)Hook_wndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= g.module_handle;
		wcex.hIcon			= 0;
		wcex.hCursor		= 0;
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= _T(PYAUTO_WINDOW_NAME);
		wcex.hIconSm		= 0;

		RegisterClassEx(&wcex);
	}
	
	// フックのイベントを受け取るためのウィンドウを作成
	g.pyauto_window = CreateWindow( _T(PYAUTO_WINDOW_NAME), _T(PYAUTO_WINDOW_NAME), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g.module_handle, NULL );
	
	// Hookオブジェクトを記録する
	g.pyhook = (PyObject_Hook*)self;

	return 0;
}

static void _Hook_destroy()
{
	if(g.pyhook)
	{
		HookEnd_Key();
		HookEnd_Clipboard();
	
		DestroyWindow(g.pyauto_window);
		g.pyauto_window = NULL;

		if(hook_instance_mutex)
		{
			CloseHandle(hook_instance_mutex);
			hook_instance_mutex=NULL;
		}

		g.pyhook = NULL;
	}
}

static void Hook_dealloc( PyObject * self )
{
	_Hook_destroy();

	Py_XDECREF(((PyObject_Hook*)self)->keydown);
	Py_XDECREF(((PyObject_Hook*)self)->keyup);
	Py_XDECREF(((PyObject_Hook*)self)->mousedown);
	Py_XDECREF(((PyObject_Hook*)self)->mouseup);
	Py_XDECREF(((PyObject_Hook*)self)->mousedblclk);
	Py_XDECREF(((PyObject_Hook*)self)->mousemove);
	Py_XDECREF(((PyObject_Hook*)self)->mousewheel);
	Py_XDECREF(((PyObject_Hook*)self)->mousehorizontalwheel);
	Py_XDECREF(((PyObject_Hook*)self)->clipboard);

	self->ob_type->tp_free((PyObject*)self);
}

static PyObject * Hook_destroy( PyObject* self, PyObject* args )
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	_Hook_destroy();

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * Hook_getattr( PyObject_Hook * self, PyObject * pyattrname )
{
	const wchar_t * attrname = PyUnicode_AS_UNICODE(pyattrname);

	if( attrname[0]=='k' && wcscmp(attrname,L"keydown")==0 )
	{
		if(self->keydown)
		{
			Py_INCREF(self->keydown);
			return self->keydown;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='k' && wcscmp(attrname,L"keyup")==0 )
	{
		if(self->keyup)
		{
			Py_INCREF(self->keyup);
			return self->keyup;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousedown")==0 )
	{
		if(self->mousedown)
		{
			Py_INCREF(self->mousedown);
			return self->mousedown;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mouseup")==0 )
	{
		if(self->mouseup)
		{
			Py_INCREF(self->mouseup);
			return self->mouseup;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousedblclk")==0 )
	{
		if(self->mousedblclk)
		{
			Py_INCREF(self->mousedblclk);
			return self->mousedblclk;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousemove")==0 )
	{
		if(self->mousemove)
		{
			Py_INCREF(self->mousemove);
			return self->mousemove;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousewheel")==0 )
	{
		if(self->mousewheel)
		{
			Py_INCREF(self->mousewheel);
			return self->mousewheel;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousehorizontalwheel")==0 )
	{
		if(self->mousehorizontalwheel)
		{
			Py_INCREF(self->mousehorizontalwheel);
			return self->mousehorizontalwheel;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else if( attrname[0]=='c' && wcscmp(attrname,L"clipboard")==0 )
	{
		if(self->clipboard)
		{
			Py_INCREF(self->clipboard);
			return self->clipboard;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	else
	{
		return PyObject_GenericGetAttr((PyObject*)self,pyattrname);
	}
}

static int Hook_setattr( PyObject_Hook * self, PyObject * pyattrname, PyObject * pyvalue )
{
	const wchar_t * attrname = PyUnicode_AS_UNICODE(pyattrname);

	if( attrname[0]=='k' && wcscmp(attrname,L"keydown")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->keydown);
			self->keydown = pyvalue;

			HookStart_Key();
		}
		else
		{
			Py_XDECREF(self->keydown);
			self->keydown = NULL;
		}
	}
	else if( attrname[0]=='k' && wcscmp(attrname,L"keyup")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->keyup);
			self->keyup = pyvalue;

			HookStart_Key();
		}
		else
		{
			Py_XDECREF(self->keyup);
			self->keyup = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousedown")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mousedown);
			self->mousedown = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mousedown);
			self->mousedown = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mouseup")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mouseup);
			self->mouseup = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mouseup);
			self->mouseup = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousedblclk")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mousedblclk);
			self->mousedblclk = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mousedblclk);
			self->mousedblclk = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousemove")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mousemove);
			self->mousemove = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mousemove);
			self->mousemove = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousewheel")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mousewheel);
			self->mousewheel = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mousewheel);
			self->mousewheel = NULL;
		}
	}
	else if( attrname[0]=='m' && wcscmp(attrname,L"mousehorizontalwheel")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->mousehorizontalwheel);
			self->mousehorizontalwheel = pyvalue;

			HookStart_Mouse();
		}
		else
		{
			Py_XDECREF(self->mousehorizontalwheel);
			self->mousehorizontalwheel = NULL;
		}
	}
	else if( attrname[0]=='c' && wcscmp(attrname,L"clipboard")==0 )
	{
		if(pyvalue!=Py_None)
		{
			Py_INCREF(pyvalue);
			Py_XDECREF(self->clipboard);
			self->clipboard = pyvalue;

			HookStart_Clipboard();
		}
		else
		{
			Py_XDECREF(self->clipboard);
			self->clipboard = NULL;

			HookEnd_Clipboard();
		}
	}
	else
	{
		return PyObject_GenericSetAttr((PyObject*)self,pyattrname,pyvalue);
	}

	return 0;
}

static PyObject * Hook_reset( PyObject_Hook * self, PyObject* args )
{
	if( ! PyArg_ParseTuple(args, "" ) )
        return NULL;

	// フック解除
	{
		PythonUtil_DebugPrintf("unset hook\n");

		HookEnd_Key();
		HookEnd_Mouse();
		HookEnd_Clipboard();
	}

	// フック再スタート
	{
		PythonUtil_DebugPrintf("set hook\n");

		if( self->keydown || self->keyup )
		{
			HookStart_Key();
		}

		if( self->mousedown || self->mouseup || self->mousedblclk || self->mousemove || self->mousewheel || self->mousehorizontalwheel )
		{
			HookStart_Mouse();
		}

		HookStart_Clipboard();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef Hook_methods[] = {
	{ "destroy", Hook_destroy, METH_VARARGS, "" },
	{ "reset", (PyCFunction)Hook_reset, METH_VARARGS, "" },
	{NULL,NULL}
};

PyTypeObject pyauto::PyType_Hook = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"Hook",				/* tp_name */
	sizeof(PyType_Hook), /* tp_basicsize */
	0,					/* tp_itemsize */
	Hook_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, 					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	(getattrofunc)Hook_getattr,	/* tp_getattro */
	(setattrofunc)Hook_setattr,	/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	"",					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	Hook_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	Hook_init,			/* tp_init */
	0,					/* tp_alloc */
	PyType_GenericNew,	/* tp_new */
	0,					/* tp_free */
};

// ------------ global function -----------------------------------------

static PyObject * _shellExecute( PyObject * self, PyObject * args )
{
	BOOL result;
	PyObject * pyverb;
	PyObject * pyfile;
	PyObject * pyparam=NULL;
	PyObject * pydirectory=NULL;
	PyObject * pyswmode=NULL;

	if( ! PyArg_ParseTuple(args,"OO|OOO", &pyverb, &pyfile, &pyparam, &pydirectory, &pyswmode ) )
		return NULL;

	const TCHAR * p_file = NULL;
	const TCHAR * p_verb = NULL;
	const TCHAR * p_param = NULL;
	const TCHAR * p_directory = NULL;
	const TCHAR * p_swmode = NULL;

	_tString verb;
	_tString file;
	_tString param;
	_tString directory;
	_tString swmode;

	if( stringutil::PyStringTo_tString( pyverb, &verb ) )
		p_verb = verb.c_str();

	if( stringutil::PyStringTo_tString( pyfile, &file ) )
		p_file = file.c_str();

	if( pyparam && stringutil::PyStringTo_tString( pyparam, &param ) )
		p_param = param.c_str();

	if( pydirectory && stringutil::PyStringTo_tString( pydirectory, &directory ) )
		p_directory = directory.c_str();

	if( pyswmode && stringutil::PyStringTo_tString( pyswmode, &swmode ) )
		p_swmode = swmode.c_str();
	
	SHELLEXECUTEINFO sei;
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
	sei.hwnd = NULL;
	sei.lpVerb = p_verb;
	sei.lpFile = p_file;
	sei.lpParameters = p_param;
	sei.lpDirectory = p_directory;

	if( p_swmode==NULL || swmode==_T("") || swmode==_T("normal") )
	{
		sei.nShow = SW_SHOWNORMAL;
	}
	else if(swmode==_T("minimized"))
	{
		sei.nShow = SW_SHOWMINIMIZED;
	}
	else if(swmode==_T("maximized"))
	{
		sei.nShow = SW_SHOWMAXIMIZED;
	}
	else
	{
		PyErr_SetString( PyExc_ValueError, "invalid show mode." );
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	result = ShellExecuteEx(&sei);
	Py_END_ALLOW_THREADS

	if(result)
	{
    	Py_INCREF(Py_None);
    	return Py_None;
	}
	else
	{
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject * _messageLoop( PyObject * self, PyObject * args )
{
    if( ! PyArg_ParseTuple( args, "" ) )
        return NULL;

	MSG msg;
	for(;;)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				goto end;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Py_BEGIN_ALLOW_THREADS

        WaitMessage();

		Py_END_ALLOW_THREADS
	}

end:

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject * _setDebug( PyObject * self, PyObject * args )
{
	int enabled;

    if( ! PyArg_ParseTuple( args, "i", &enabled ) )
        return NULL;

	g.debug = enabled!=0;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef pyauto_funcs[] = {
    { "shellExecute", _shellExecute, METH_VARARGS, "" },
    { "messageLoop", _messageLoop, METH_VARARGS, "" },
    { "setDebug", _setDebug, METH_VARARGS, "" },
    {NULL, NULL, 0, NULL}
};

// ------------ module init -----------------------------------------

static PyModuleDef pyautocore_module =
{
    PyModuleDef_HEAD_INIT,
    MODULE_NAME,
    "pyautocore module.",
    -1,
    pyauto_funcs,
	NULL, NULL, NULL, NULL
};

extern "C" PyMODINIT_FUNC PyInit_pyautocore()
{
	if( PyType_Ready(&PyType_Window)<0 ) return NULL;
	if( PyType_Ready(&PyType_Image)<0 ) return NULL;
	if( PyType_Ready(&PyType_Input)<0 ) return NULL;
	if( PyType_Ready(&PyType_Hook)<0 ) return NULL;

	PyObject *m;

    m = PyModule_Create(&pyautocore_module);
    if(m == NULL) return NULL;

    Py_INCREF(&PyType_Window);
    PyModule_AddObject( m, "Window", (PyObject*)&PyType_Window );

    Py_INCREF(&PyType_Image);
    PyModule_AddObject( m, "Image", (PyObject*)&PyType_Image );

    Py_INCREF(&PyType_Input);
    PyModule_AddObject( m, "Input", (PyObject*)&PyType_Input );

	Py_INCREF(&PyType_Hook);
    PyModule_AddObject( m, "Hook", (PyObject*)&PyType_Hook );

	g.Error = PyErr_NewException( MODULE_NAME".Error", NULL, NULL);
    Py_INCREF(g.Error);
    PyModule_AddObject( m, "Error", (PyObject*)g.Error );

	if( PyErr_Occurred() )
	{
		Py_FatalError( "can't initialize module " MODULE_NAME );
	}

	return m;
}

// ------------ DllMain -----------------------------------------

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    (void)lpReserved;
    if(ul_reason_for_call==DLL_PROCESS_ATTACH)
	{
		g.module_handle = (HMODULE)hModule;
    }
    return TRUE;
}

