#ifndef CLIPBOARD_H
#define CLIPBOARD_H

extern void HookStart_Clipboard();
extern void HookEnd_Clipboard();
extern LRESULT Hook_Clipboard_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif//CLIPBOARD_H
