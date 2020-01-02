#ifndef _PYTHONUTIL_H_
#define _PYTHONUTIL_H_

#if defined(_DEBUG)
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif

namespace PythonUtil
{
	//#define GIL_Ensure_TRACE printf("%s(%d) : %s\n",__FILE__,__LINE__,__FUNCTION__)
	#define GIL_Ensure_TRACE

	class GIL_Ensure
	{
	public:
		GIL_Ensure()
		{
			GIL_Ensure_TRACE;
			state = PyGILState_Ensure();
			GIL_Ensure_TRACE;
		};

		~GIL_Ensure()
		{
			GIL_Ensure_TRACE;
			PyGILState_Release(state);
			GIL_Ensure_TRACE;
		};
		
	private:
		PyGILState_STATE state;
	};

	#define PythonUtil_Printf(...) PySys_WriteStdout(__VA_ARGS__)
	#define PythonUtil_DebugPrintf(...) if(g.debug) { PySys_WriteStdout(__VA_ARGS__); }
};

#endif // _PYTHONUTIL_H_
