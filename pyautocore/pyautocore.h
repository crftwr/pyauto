#ifndef PYAUTOCORE_H
#define PYAUTOCORE_H


namespace pyauto
{
	extern PyTypeObject PyType_Window;

	struct PyObject_Window
	{
		PyObject_HEAD
		HWND hwnd;
	};

	extern PyTypeObject PyType_Image;

	struct PyObject_Image
	{
		PyObject_HEAD
		int w;
		int h;
		void * buf;
		unsigned int bufsize;
	};

	extern PyTypeObject PyType_Input;

	struct PyObject_Input
	{
		PyObject_HEAD
		int num;
		INPUT input[2];
	};

	extern PyTypeObject PyType_Hook;

	struct PyObject_Hook
	{
		PyObject_HEAD
		
		PyObject * keydown;
		PyObject * keyup;

		PyObject * mousedown;
		PyObject * mouseup;
		PyObject * mousedblclk;
		PyObject * mousemove;			// FIXME : not implemented
		PyObject * mousewheel;
		PyObject * mousehorizontalwheel;

		PyObject * clipboard;
	};

	struct Globals
	{
		HMODULE module_handle;
		PyObject * Error;
		PyObject_Hook * pyhook;
		HWND pyauto_window;
		DWORD last_key_time;
		bool debug;
	};
	
	extern Globals g;
};

#define TypeCheck_Window(op) PyObject_TypeCheck(op, &PyType_Window)
#define TypeCheck_Image(op) PyObject_TypeCheck(op, &PyType_Image)
#define TypeCheck_Input(op) PyObject_TypeCheck(op, &PyType_Input)
#define TypeCheck_Hook(op) PyObject_TypeCheck(op, &PyType_Hook)


#endif//PYAUTOCORE_H
