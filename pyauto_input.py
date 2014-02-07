# -*- coding: utf_8 -*-

import pyauto.pyautocore as pyautocore

## @addtogroup pyauto
## @{

## 仮想的なキーダウンを生成するクラス
class KeyDown( pyautocore.Input ):

    def __init__( self, vk ):

        pyautocore.Input.__init__(self)
        if type(vk)==str and len(vk)==1 : vk=ord(vk)
        self.setKeyDown(vk)

## 仮想的なキーアップを生成するクラス
class KeyUp( pyautocore.Input ):

    def __init__( self, vk ):

        pyautocore.Input.__init__(self)
        if type(vk)==str and len(vk)==1 : vk=ord(vk)
        self.setKeyUp(vk)

## 仮想的なキーのダウンとアップを生成するクラス
class Key( pyautocore.Input ):

    def __init__( self, vk ):

        pyautocore.Input.__init__(self)
        if type(vk)==str and len(vk)==1 : vk=ord(vk)
        self.setKey(vk)

## 仮想的な文字入力を生成するクラス
class Char( pyautocore.Input ):

    def __init__( self, c ):

        pyautocore.Input.__init__(self)
        if type(c)==str and len(c)==1 : c=ord(c)
        self.setChar(c)

## 仮想的なマウス移動を生成するクラス
class MouseMove( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseMove( x, y )

## 仮想的なマウスの左ボタンダウンを生成するクラス
class MouseLeftDown( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseLeftDown( x, y )

## 仮想的なマウスの左ボタンアップを生成するクラス
class MouseLeftUp( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseLeftUp( x, y )

## 仮想的なマウスの左ボタンのダウンとアップを生成するクラス
class MouseLeftClick( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseLeftClick( x, y )

## 仮想的なマウスの右ボタンダウンを生成するクラス
class MouseRightDown( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseRightDown( x, y )

## 仮想的なマウスの右ボタンアップを生成するクラス
class MouseRightUp( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseRightUp( x, y )

## 仮想的なマウスの右ボタンのダウンとアップを生成するクラス
class MouseRightClick( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseRightClick( x, y )

## 仮想的なマウスの中ボタンダウンを生成するクラス
class MouseMiddleDown( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseMiddleDown( x, y )

## 仮想的なマウスの中ボタンアップを生成するクラス
class MouseMiddleUp( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseMiddleUp( x, y )

## 仮想的なマウスの中ボタンのダウンとアップを生成するクラス
class MouseMiddleClick( pyautocore.Input ):

    def __init__( self, x, y ):

        pyautocore.Input.__init__(self)
        self.setMouseMiddleClick( x, y )

## 仮想的なマウスのホイール回転を生成するクラス
class MouseWheel( pyautocore.Input ):

    def __init__( self, x, y, wheel ):

        pyautocore.Input.__init__(self)
        self.setMouseWheel( x, y, wheel )

## 仮想的なマウスの水平のホイール回転を生成するクラス
class MouseHorizontalWheel( pyautocore.Input ):

    def __init__( self, x, y, wheel ):

        pyautocore.Input.__init__(self)
        self.setMouseHorizontalWheel( x, y, wheel )

## @} pyauto

