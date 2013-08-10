# -*- coding: utf_8 -*-

from pyauto import *
from keymap import *

# ------------- Modifier -------------------------------------------------

defineModifier( 235, MODKEY_USER0_L )


# ------------- Global ---------------------------------------------------

class Keymap_Global( Keymap ):

    def __init__(self,wnd):
        Keymap.__init__(self,wnd)

        #print "Keymap_Global.__init__"

        def getTopLevelWindow( wnd ):
            parent = wnd.getParent()
            while parent != Window.getDesktop():
                wnd = parent;
                parent = wnd.getParent()
            return wnd

        # アプリの最上位ウインドウを取得する
        self.wnd_top = getTopLevelWindow(wnd)

        # Ctrl-F12 : pyautoのmessageLoopを中断する
        self.registerKeyDownHandler( K( VK_F12, MODKEY_USER0 ), quitKeymap )
        self.registerKeyDownHandler( K( VK_F5, MODKEY_USER0 ), loadConfigFile )

        # USER0-↑↓←→ : 10pixel単位のウインドウの移動
        self.registerKeyDownHandler( K( VK_LEFT, MODKEY_USER0 ), self.WindowMoveLeft )
        self.registerKeyDownHandler( K( VK_RIGHT, MODKEY_USER0 ), self.WindowMoveRight )
        self.registerKeyDownHandler( K( VK_UP, MODKEY_USER0 ), self.WindowMoveUp )
        self.registerKeyDownHandler( K( VK_DOWN, MODKEY_USER0 ), self.WindowMoveDown )

        # USER0-Shift-↑↓←→ : 1pixel単位のウインドウの移動
        self.registerKeyDownHandler( K( VK_LEFT, MODKEY_USER0 | MODKEY_SHIFT ), self.WindowMoveLeft1dot )
        self.registerKeyDownHandler( K( VK_RIGHT, MODKEY_USER0 | MODKEY_SHIFT ), self.WindowMoveRight1dot )
        self.registerKeyDownHandler( K( VK_UP, MODKEY_USER0 | MODKEY_SHIFT ), self.WindowMoveUp1dot )
        self.registerKeyDownHandler( K( VK_DOWN, MODKEY_USER0 | MODKEY_SHIFT ), self.WindowMoveDown1dot )

        # USER0-Ctrl-↑↓←→ : 画面の端まで移動
        self.registerKeyDownHandler( K( VK_LEFT, MODKEY_CTRL | MODKEY_USER0 ), self.WindowMoveLeftEdge )
        self.registerKeyDownHandler( K( VK_RIGHT, MODKEY_CTRL | MODKEY_USER0 ), self.WindowMoveRightEdge )
        self.registerKeyDownHandler( K( VK_UP, MODKEY_CTRL | MODKEY_USER0 ), self.WindowMoveUpEdge )
        self.registerKeyDownHandler( K( VK_DOWN, MODKEY_CTRL | MODKEY_USER0 ), self.WindowMoveDownEdge )

    def WindowMoveLeft( self ):
        rect = list(self.wnd_top.getRect())
        rect[0] -= 10
        rect[2] -= 10
        self.wnd_top.setRect(rect)

    def WindowMoveRight( self ):
        rect = list(self.wnd_top.getRect())
        rect[0] += 10
        rect[2] += 10
        self.wnd_top.setRect(rect)

    def WindowMoveUp( self ):
        rect = list(self.wnd_top.getRect())
        rect[1] -= 10
        rect[3] -= 10
        self.wnd_top.setRect(rect)

    def WindowMoveDown( self ):
        rect = list(self.wnd_top.getRect())
        rect[1] += 10
        rect[3] += 10
        self.wnd_top.setRect(rect)

    def WindowMoveLeft1dot( self ):
        rect = list(self.wnd_top.getRect())
        rect[0] -= 1
        rect[2] -= 1
        self.wnd_top.setRect(rect)

    def WindowMoveRight1dot( self ):
        rect = list(self.wnd_top.getRect())
        rect[0] += 1
        rect[2] += 1
        self.wnd_top.setRect(rect)

    def WindowMoveUp1dot( self ):
        rect = list(self.wnd_top.getRect())
        rect[1] -= 1
        rect[3] -= 1
        self.wnd_top.setRect(rect)

    def WindowMoveDown1dot( self ):
        rect = list(self.wnd_top.getRect())
        rect[1] += 1
        rect[3] += 1
        self.wnd_top.setRect(rect)

    def getDesktopRect(self):
        return Window.getDesktop().getRect()

    def getWorkingArea(self):
        desktop_rect = Window.getDesktop().getRect()
        taskbar_rect = Window.find("Shell_TrayWnd",None).getRect()

        rect = list(desktop_rect)

        if taskbar_rect[0]>desktop_rect[0] and taskbar_rect[0]<desktop_rect[2] : rect[2]=taskbar_rect[0]
        if taskbar_rect[1]>desktop_rect[1] and taskbar_rect[1]<desktop_rect[3] : rect[3]=taskbar_rect[1]
        if taskbar_rect[2]>desktop_rect[0] and taskbar_rect[2]<desktop_rect[2] : rect[0]=taskbar_rect[2]
        if taskbar_rect[3]>desktop_rect[1] and taskbar_rect[3]<desktop_rect[3] : rect[1]=taskbar_rect[3]

        return rect

    def WindowMoveLeftEdge( self ):
        desktop_rect = self.getWorkingArea()
        rect = list(self.wnd_top.getRect())
        delta = desktop_rect[0]-rect[0]
        rect[0] += delta
        rect[2] += delta
        self.wnd_top.setRect(rect)

    def WindowMoveRightEdge( self ):
        desktop_rect = self.getWorkingArea()
        rect = list(self.wnd_top.getRect())
        delta = desktop_rect[2]-rect[2]
        rect[0] += delta
        rect[2] += delta
        self.wnd_top.setRect(rect)

    def WindowMoveUpEdge( self ):
        desktop_rect = self.getWorkingArea()
        rect = list(self.wnd_top.getRect())
        delta = desktop_rect[1]-rect[1]
        rect[1] += delta
        rect[3] += delta
        self.wnd_top.setRect(rect)

    def WindowMoveDownEdge( self ):
        desktop_rect = self.getWorkingArea()
        rect = list(self.wnd_top.getRect())
        delta = desktop_rect[3]-rect[3]
        rect[1] += delta
        rect[3] += delta
        self.wnd_top.setRect(rect)

registorKeymap( ( None, None, None ), Keymap_Global )


# ------------- Notepad --------------------------------------------------

# メモ帳を 少しだけ Emacs風にするサンプル
class Keymap_Notepad( Keymap_Global ):

    PREFIX_CTRL_X = 1

    def __init__(self,wnd):
        Keymap_Global.__init__(self,wnd)

        #print "Keymap_Notepad.__init__"

        # Ctrl-S : 前方検索
        self.registerKeyDownHandler( K( 'S', MODKEY_CTRL ),
            [
                KeyDown(VK_LCONTROL),
                Key('F'),
                KeyUp(VK_LCONTROL),
                KeyDown(VK_LMENU),
                Key('D'),
                Key('N'),
                KeyUp(VK_LMENU)
            ]
        )

        # Ctrl-R : 後方検索
        self.registerKeyDownHandler( K( 'R', MODKEY_CTRL ),
            [
                KeyDown(VK_LCONTROL),
                Key('F'),
                KeyUp(VK_LCONTROL),
                KeyDown(VK_LMENU),
                Key('U'),
                Key('N'),
                KeyUp(VK_LMENU)
            ]
        )

        # Ctrl-B : 一文字左に移動
        self.registerKeyDownHandler( K( 'B', MODKEY_CTRL ),
            [
                Key(VK_LEFT)
            ]
        )

        # Alt-B : 一単語左に移動
        self.registerKeyDownHandler( K( 'B', MODKEY_ALT ),
            [
                KeyDown(VK_LCONTROL),
                Key(VK_LEFT),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-F : 一文字右に移動
        self.registerKeyDownHandler( K( 'F', MODKEY_CTRL ),
            [
                Key(VK_RIGHT)
            ]
        )

        # Alt-F : 一単語右に移動
        self.registerKeyDownHandler( K( 'F', MODKEY_ALT ),
            [
                KeyDown(VK_LCONTROL),
                Key(VK_RIGHT),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-A : 行頭へ移動
        self.registerKeyDownHandler( K( 'A', MODKEY_CTRL ),
            [
                Key(VK_HOME)
            ]
        )

        # Ctrl-E : 行末へ移動
        self.registerKeyDownHandler( K( 'E', MODKEY_CTRL ),
            [
                Key(VK_END)
            ]
        )

        # Ctrl-N : 下に移動
        self.registerKeyDownHandler( K( 'N', MODKEY_CTRL ),
            [
                Key(VK_DOWN)
            ]
        )

        # Ctrl-P : 上に移動
        self.registerKeyDownHandler( K( 'P', MODKEY_CTRL ),
            [
                Key(VK_UP)
            ]
        )

        # Alt-V : 上にスクロール
        self.registerKeyDownHandler( K( 'V', MODKEY_ALT ),
            [
                Key(VK_PRIOR)
            ]
        )

        # Ctrl-V : 下にスクロール
        self.registerKeyDownHandler( K( 'V', MODKEY_CTRL ),
            [
                Key(VK_NEXT)
            ]
        )

        # Alt-DEL : 直前の単語を削除する
        self.registerKeyDownHandler( K( VK_DELETE, MODKEY_ALT ),
            [
                KeyDown(VK_LCONTROL),
                KeyDown(VK_LSHIFT),

                Key(VK_LEFT),

                KeyUp(VK_LSHIFT),
                KeyUp(VK_LCONTROL),

                Key(VK_DELETE)
            ]
        )

        # Ctrl-D : キャレットの右側の文字を削除する
        self.registerKeyDownHandler( K( 'D', MODKEY_CTRL ),
            [
                Key(VK_DELETE)
            ]
        )

        # Alt-D : 単語の末尾まで削除する
        self.registerKeyDownHandler( K( 'D', MODKEY_ALT ),
            [
                KeyDown(VK_LCONTROL),
                KeyDown(VK_LSHIFT),

                Key(VK_RIGHT),

                KeyUp(VK_LSHIFT),
                KeyUp(VK_LCONTROL),

                Key(VK_DELETE)
            ]
        )

        # Ctrl-K : 行末まで削除する
        self.registerKeyDownHandler( K( 'K', MODKEY_CTRL ),
            [
                KeyDown(VK_LCONTROL),
                Key(VK_DELETE),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-H : キャレットの左側の文字を削除する
        self.registerKeyDownHandler( K( 'H', MODKEY_CTRL ),
            [
                Key(VK_BACK)
            ]
        )

        # Ctrl-W : リージョンを切り取る
        self.registerKeyDownHandler( K( 'W', MODKEY_CTRL ),
            [
                KeyDown(VK_LCONTROL),
                Key('X'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Alt-W : リージョンをコピーする
        self.registerKeyDownHandler( K( 'W', MODKEY_ALT ),
            [
                KeyDown(VK_LCONTROL),
                Key('C'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-Y : 貼り付け
        self.registerKeyDownHandler( K( 'Y', MODKEY_CTRL ),
            [
                KeyDown(VK_LCONTROL),
                Key('V'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-X : prefix
        self.registerKeyDownHandler( K( 'X', MODKEY_CTRL ),
            self._setPrefix_CTRL_X
        )

        # Ctrl-X Ctrl-F : ファイルを開く
        self.registerKeyDownHandler( K( 'F', MODKEY_CTRL, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LCONTROL),
                Key('O'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-X Ctrl-S : 上書き保存
        self.registerKeyDownHandler( K( 'S', MODKEY_CTRL, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LCONTROL),
                Key('S'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-X Ctrl-W : 名前を付けて保存
        self.registerKeyDownHandler( K( 'W', MODKEY_CTRL, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LMENU),
                Key('F'),
                Key('A'),
                KeyUp(VK_LMENU)
            ]
        )

        # Ctrl-X U : 元に戻す
        self.registerKeyDownHandler( K( 'U', 0, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LCONTROL),
                Key('Z'),
                KeyUp(VK_LCONTROL)
            ]
        )

        # Ctrl-X K : ファイルを閉じる
        self.registerKeyDownHandler( K( 'K', 0, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LMENU),
                Key(VK_F4),
                KeyUp(VK_LMENU)
            ]
        )

        # Ctrl-X Ctrl-C : ファイルを閉じる
        self.registerKeyDownHandler( K( 'C', MODKEY_CTRL, Keymap_Notepad.PREFIX_CTRL_X ),
            [
                KeyDown(VK_LMENU),
                Key(VK_F4),
                KeyUp(VK_LMENU)
            ]
        )

    def _setPrefix_CTRL_X(self):
        self.setPrefix(Keymap_Notepad.PREFIX_CTRL_X)

registorKeymap( ( r".*\\notepad\.exe", "Edit", None ), Keymap_Notepad )


# ------------- Putty ----------------------------------------------------

class Keymap_Putty(Keymap_Global):
    def __init__(self,wnd):

        Keymap_Global.__init__(self,wnd)

        #print 'Keymap_Putty.__init__'

        # Ctrl-TAB : ウインドウを切り替える
        self.registerKeyDownHandler( K( VK_TAB, MODKEY_CTRL ), self.selectNext )

        self.wnd = wnd

    def selectNext( self ):
        #print 'Keymap_Putty.selectNext'

        wnd = self.wnd.getNext()
        last_found_wnd = None
        while wnd:
            if wnd.getClassName()=="PuTTY" and not wnd.isMinimized():
                last_found_wnd = wnd
            wnd = wnd.getNext()
        if last_found_wnd:
            #print last_found_wnd
            last_found_wnd.setForeground()

registorKeymap( ( r".*\\puttyjp\.exe", "PuTTY", None ), Keymap_Putty )

