
## @addtogroup pyauto
## @{

## ウインドウを表すクラス
class Window:
    
    ## ウインドウのタイトル文字列を取得する
    #
    #  @param self      -
    #  @return          ウインドウのタイトル文字列
    #
    def getText(self):
        pass
        
    ## ウインドウのクラス名を取得する
    #
    #  @param self      -
    #  @return          ウインドウのクラス名
    #
    def getClassName(self):
        pass

    ## ウインドウの所属するプロセスの名前を取得する
    #
    #  @param self      -
    #  @return          ウインドウの所属するプロセスの名前 (実行ファイルのディレクトリを除いたファイル名)
    #
    def getProcessName(self):
        pass

    ## ウインドウの所属するプロセスの実行ファイル名を取得する
    #
    #  @param self      -
    #  @return          ウインドウの所属するプロセスの実行ファイル名
    #
    def getProcessPath(self):
        pass

    ## ウインドウハンドルを取得する
    #
    #  @param self      -
    #  @return          ウインドウハンドル
    #
    #  ウインドウオブジェクトに関連付けられているウインドウハンドルを取得します。
    #
    def getHWND(self):
        pass

    ## ウインドウの左上と右下の座標をスクリーン座標で取得する
    #
    #  @param self      -
    #  @return          ウインドウの左上と右下の座標 (left,top,right,bottom)
    #
    #  スクリーン座標とは、画面の左上端を(0,0)とし右下方向を正とする、ピクセル単位の座標系です。
    #
    def getRect(self):
        pass

    ## ウインドウのクライアント領域の矩形を取得する
    #
    #  @param self      -
    #  @return          ウインドウのクライアント領域の矩形 (left,top,right,bottom)
    #
    def getClientRect(self):
        pass

    ## ウインドウのクライアント領域の座標をスクリーン座標に変換する
    #
    #  @param self      -
    #  @param x         クライアント領域のx座標
    #  @param y         クライアント領域のy座標
    #  @return          変換されたスクリーン座標
    #
    def clientToScreen(self,x,y):
        pass

    ## ウインドウの領域をスクリーン座標で設定する
    #
    #  @param self      -
    #  @param rect      ウインドウの左上と右下の座標 (left,top,right,bottom)
    #  @return          -
    #
    #  スクリーン座標とは、画面の左上端を(0,0)とし右下方向を正とする、ピクセル単位の座標系です。
    #
    def setRect(self,rect):
        pass


    ## Zオーダーが最初の子ウインドウを取得する
    #
    #  @param self      -
    #  @return          Zオーダーが最初の子ウインドウ
    #
    #  ウインドウが子ウインドウを一つも持たない場合はNoneが返ります。
    #
    def getFirstChild(self):
        pass


    ## Zオーダーが最後の子ウインドウを取得する
    #
    #  @param self      -
    #  @return          Zオーダーが最後の子ウインドウ
    #
    #  ウインドウが子ウインドウを一つも持たない場合はNoneが返ります。
    #
    def getLastChild(self):
        pass


    ## Zオーダーが１つ手前の兄弟ウインドウを取得する
    #
    #  @param self      -
    #  @return          Zオーダーが１つ手前の兄弟ウインドウ
    #
    #  ウインドウが兄弟の中で最も手前のウインドウである場合はNoneが返ります。
    #
    def getPrevious(self):
        pass

    ## Zオーダーが１つ奥の兄弟ウインドウを取得する
    #
    #  @param self      -
    #  @return          Zオーダーが１つ奥の兄弟ウインドウ
    #
    #  ウインドウが兄弟の中で最も奥のウインドウである場合はNoneが返ります。
    #
    def getNext(self):
        pass

    ## 親ウインドウを取得する
    #
    #  @param self      -
    #  @return          親ウインドウ
    #
    #  ウインドウがトップレベルウインドウである場合はNoneが返ります。
    #
    def getParent(self):
        pass


    ## オーナーウインドウを取得する
    #
    #  @param self      -
    #  @return          オーナーウインドウ
    #
    #  オーナーが存在しない場合はNoneが返ります。
    #
    def getOwner(self):
        pass


    ## 所有するポップアップウインドウの中で最後にアクティブだったウインドウを取得する
    #
    #  @param self      -
    #  @return          最後にアクティブだったウインドウ
    #
    def getLastActivePopup(self):
        pass


    ## 表示状態を調べる
    #
    #  @param self      -
    #  @return          True:表示 False:非表示
    #
    def isVisible(self):
        pass


    ## 有効か無効か(マウスやキーボードの入力を受け付ける状態であるか)を調べる
    #
    #  @param self      -
    #  @return          True:有効 False:無効
    #
    def isEnabled(self):
        pass

    ## 最小化状態であるかどうかを調べる
    #
    #  @param self      -
    #  @return          True:最小化されている False:最小化されていない
    #
    def isMinimized(self):
        pass

    ## 最大化状態であるかどうかを調べる
    #
    #  @param self      -
    #  @return          True:最大化されている False:最大化されていない
    #
    def isMaximized(self):
        pass

    ## 最小化状態にする
    #
    #  @param self      -
    #  @return          -
    #
    def minimize(self):
        pass

    ## 最大化状態にする
    #
    #  @param self      -
    #  @return          -
    #
    def maximize(self):
        pass

    ## 最小化状態および最大化状態を解除する
    #
    #  @param self      -
    #  @return          -
    #
    def restore(self):
        pass

    ## ラジオボタンやチェックボックスのチェック状態を取得する
    #
    #  @param self      -
    #  @return          0:チェックされていない 1:チェックされている 2:不確定(3ステートのボタンのときのみ)
    #
    def getCheck(self):
        pass

    ## ラジオボタンやチェックボックスのチェック状態を設定する
    #
    #  @param self      -
    #  @param status    0:チェックされていない 1:チェックされている 2:不確定(3ステートのボタンのときのみ)
    #  @return          -
    #
    def setCheck(self,status):
        pass

    ## ウインドウのIMEの状態を取得する
    #
    #  @param self      -
    #  @return          0:OFF 1:ON
    #
    def getImeStatus(self):
        pass

    ## ウインドウのIMEの状態を設定する
    #
    #  @param self      -
    #  @param status    0:OFF 1:ON
    #  @return          -
    #
    def setImeStatus(self,status):
        pass

    ## ウインドウのメッセージキューに、メッセージをポストする
    #
    #  @param self      -
    #  @param msg       メッセージの種別
    #  @param wparam    メッセージ特有の追加情報１
    #  @param lparam    メッセージ特有の追加情報２
    #  @return          -
    #
    #  postMessageは、メッセージをポストしてから、そのメッセージが処理されるのを待たずに返ります。
    #
    def postMessage(self,msg,wparam=0,lparam=0):
        pass

    ## ウインドウにメッセージを送信する
    #
    #  @param self      -
    #  @param msg       メッセージの種別
    #  @param wparam    メッセージ特有の追加情報１
    #  @param lparam    メッセージ特有の追加情報２
    #  @return          -
    #
    #  sendMessageは、メッセージがウインドウプロシージャによって処理されるのを待ってから制御を返します。
    #
    def sendMessage(self,msg,wparam=0,lparam=0):
        pass

    ## ウインドウをフォアグラウンドにする
    #
    #  @param self      -
    #  @param force     スレッドのインプット状態を切り替えるか否か
    #  @return          -
    #
    #  引数 force に True を与えると、スレッドのインプット状態を切り替えてから、ウインドウをフォアグラウンド化します。ウインドウをフォアグラウンドにしても、タスクバーのボタンが点滅する場合は、引数 force に True を与えてみてください。
    #
    def setForeground(self,force=False):
        pass


    ## ウインドウをアクティブにする
    #
    #  @param self      -
    #  @return          -
    #
    def setActive(self):
        pass

    ## ウインドウの画面イメージを取得する
    #
    #  @param self      -
    #  @return          ウインドウの画面イメージを表すImageクラスのオブジェクト
    #
    #  このメソッドを使用して取得した画面イメージを調査して、ウインドウの状態を特定することができます。
    #
    def getImage(self):
        pass

    ## (static method) フォアグラウンドウインドウを取得する
    #
    #  @return          フォアグラウンドウインドウ
    #
    #  フォアグラウンドウインドウが存在しない場合はNoneが返ります。
    #
    @staticmethod
    def getForeground():
        pass

    ## (static method) フォーカスされているウインドウを取得する
    #
    #  @return          フォーカスされているウインドウ
    #
    #  フォーカスされているウインドウが存在しない場合はNoneが返ります。
    #
    @staticmethod
    def getFocus():
        pass

    ## (static method) キャレットの情報を取得する
    #
    #  @return  キャレットを保持しているウインドウとキャレットの矩形 ( wnd, (left,top,right,bottom) )
    #
    #  キャレットを保持しているウインドウが存在しない場合は None, (0,0,0,0) が返ります。
    #
    @staticmethod
    def getCaret():
        pass

    ## (static method) 指定されたクラス名とタイトルを持つトップレベルウインドウを取得する
    #
    #  @param class_name    クラス名
    #  @param text          タイトル
    #  @return              ウインドウ
    #
    #  引数textにNoneを渡した場合は、あらゆるタイトルが該当するものとみなされます。
    #
    @staticmethod
    def find( class_name, text ):
        pass

    ## (static method) トップレベルウインドウを列挙する
    #
    #  @param callable  呼び出し可能オブジェクト
    #  @param arg       任意の引数
    #  @return          -
    #
    #  すべてのトップレベルウインドウに関して、引数callableに渡したオブジェクトが呼び出されます。
    #  callableには、第1引数にウインドウオブジェクト、第2引数に引数argがそのまま渡されます。
    #  callableは列挙を続行する場合はTrueを返してください。列挙を中止する場合はFalseを返してください。
    #
    @staticmethod
    def enum( callable, arg ):
        pass

    ## (static method) デスクトップウインドウを取得する
    #
    #  @return          デスクトップウインドウ
    #
    @staticmethod
    def getDesktop():
        pass

    ## (static method) ウインドウハンドルからWindowオブジェクトを作成する
    #
    #  @param  hwnd     ウインドウハンドル
    #  @return          引数hwndに関連付けられたWindowオブジェクト
    #
    #  ウインドウハンドルを受け取り、そのウインドウハンドルに関連付けられたWindowオブジェクトを返します。
    #  ほかのライブラリがウインドウハンドルを ウインドウハンドルを返すライブラリを使用する際に、このメソッドを使用します。
    #
    @staticmethod
    def fromHWND(hwnd):
        pass

    ## (static method) モニター情報を取得する
    #
    #  @return          モニターの矩形,モニターの作業領域の矩形(タスクバーを除いた領域),プライマリモニターであるか [ [ monitor_rect, work_rect, flag ], ... ]
    #
    #  ウインドウハンドルを受け取り、そのウインドウハンドルに関連付けられたWindowオブジェクトを返します。
    #  ほかのライブラリがウインドウハンドルを ウインドウハンドルを返すライブラリを使用する際に、このメソッドを使用します。
    #
    @staticmethod
    def getMonitorInfo(hwnd):
        pass


## 画像を表すクラス
class Image:

    ## 画像のサイズを取得する
    #
    #  @param self      -
    #  @return          矩形のサイズ (width,height)
    #
    def getSize(self):
        pass

    ## 画像のピクセルフォーマットを取得する
    #
    #  @param self      -
    #  @return          ピクセルフォーマットを表す文字列
    #
    #  返値modeには必ず"RGB"が返ります。現在"RGB"以外のピクセルフォーマットはサポートされていません。
    #
    def getMode(self):
        pass

    ## 画像のピクセルフォーマットを取得する
    #
    #  @param self      -
    #  @return          画素情報を格納した文字列オブジェクト
    #
    #  画像の画素情報を文字列オブジェクトとして返します。返値bufには RGBRGBRGB.... のように、RGB並びの1ピクセル24bitの情報が格納されています。
    #
    def getBuffer(self):
        pass

    ## 画像の中に指定されたサブ画像を検索する
    #
    #  @param self      -
    #  @param subimg    検索するサブ画像
    #  @return          見つかったサブ画像の座標
    #
    #  画像の中にサブ画像が含まれていなかった場合はNoneが返ります。
    #
    def find(self,subimg):
        pass

    ## (static method) 文字列オブジェクトからImageオブジェクトを作成する
    #
    #  @param mode      ピクセルフォーマットを表す文字列
    #  @param size      幅と高さを収めたタプル
    #  @param buf       画素情報を収めた文字列オブジェクト
    #  @return          生成されたImageオブジェクト
    #
    @staticmethod
    def fromString(mode,size,buf):
        pass

## マウスやキーボードの擬似的な入力を表すクラス
class Input:
    
    ## (static method) 擬似的な入力を送信する
    #
    #  @param input_sequence    Inputオブジェクトのリストまたはタプル
    #  @return                  -
    # 
    # 引数input_sequenceには、InputオブジェクトまたはInputクラスの派生クラスのオブジェクトを、リストまたはタプルに格納して渡します。
    # 
    # Inputクラスの派生クラスのリストは以下のとおりです。
    # 
    # KeyDown
    #   キーボードのキーを押す
    #   vk:仮想キーコード
    # KeyUp
    #   キーボードのキーを離す
    #   vk:仮想キーコード
    # Key
    #   キーボードのキーを押して離す
    #   vk:仮想キーコード
    # Char
    #   文字の入力
    #   c:ユニコード１文字を表す整数値または文字
    # MouseMove
    #   マウスの移動
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseLeftDown
    #   マウスの左ボタンを押す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseLeftUp
    #   マウスの左ボタンを離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseLeftClick
    #   マウスの左ボタンを押して離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseRightDown
    #   マウスの右ボタンを押す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseRightUp
    #   マウスの右ボタンを離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseRightClick
    #   マウスの右ボタンを押して離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseMiddleDown
    #   マウスの中ボタンを押す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseMiddleUp
    #   マウスの中ボタンを離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseMiddleClick
    #   マウスの中ボタンを押して離す
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標)
    # MouseWheel
    #   マウスのホイールを回転させる
    #   x:横方向位置 y:横方向位置 (画面左上端を原点とするピクセル単位の座標) wheel:回転量 (1.0=奥に向かって1クリック、-1.0=手前に向かって1クリック)
    #
    @staticmethod
    def send(input_sequence):
        pass
    
    ## (static method) 現在のマウスカーソル座標を取得する
    #
    #  @return          現在のマウスカーソル位置(ディスプレイの左上を原点とするピクセル座標系)
    #
    @staticmethod
    def getCursorPos():
        pass

    ## (static method) キーボードの各キーの状態を取得する
    #
    #  @return          各キーの状態を表す、256byteの長さのバッファ
    #
    #  返値 には、以下のようにして情報が格納されています。
    #
    # ord(state[VK_RETURN]) & 0x80  : 現在Returnが押されているかどうか
    # ord(state[VK_CAPITAL]) & 0x01 : 現在CapsLockが点灯しているかどうか
    #
    @staticmethod
    def getKeyboardState():
        pass

    ## (static method) キーボードの各キーの状態を設定する
    #
    #  @param state     各キーの状態を表す、256byteの長さのバッファ
    #  @return          -
    #
    # 引数 state には、以下のようなバッファを渡します。
    #
    # ord(state[VK_RETURN]) & 0x80  : 現在Returnが押されているかどうか
    # ord(state[VK_CAPITAL]) & 0x01 : 現在CapsLockが点灯しているかどうか
    #
    @staticmethod
    def setKeyboardState(state):
        pass

    ## (static method) キーボードの１つのキーの状態を取得する
    #
    #  @param vk        調査したいキーの仮想キーコード
    #  @return          キーの状態
    #
    # 詳細は Win32 API の GetKeyState() の解説を参照してください。
    # http://msdn.microsoft.com/ja-jp/library/cc364676.aspx
    #
    @staticmethod
    def getKeyState(vk):
        pass

    ## (static method) キーボードの１つのキーの状態を取得する
    #
    #  @param vk        調査したいキーの仮想キーコード
    #  @return          キーの状態
    #
    # 詳細は Win32 API の GetAsyncKeyState() の解説を参照してください。
    # http://msdn.microsoft.com/ja-jp/library/cc364583.aspx
    #
    @staticmethod
    def getAsyncKeyState(vk):
        pass

## システム全体のキー入力イベントやフォーカスの移動イベントをフックするためのクラス
class Hook:

    def __init__(self):
        
        ## キーの押し込み時に呼び出されるフック関数
        #
        #  @param vk    仮想キーコード
        #  @param scan  スキャンコード
        #  @return      フック関数呼出し後にデフォルトのイベント処理を続行するか否か ( None/False:続行する、True:続行しない )
        # 
        self.keydown = None

        ## キーが離された時に呼び出されるフック関数
        #
        #  @param vk    仮想キーコード
        #  @param scan  スキャンコード
        #  @return      フック関数呼出し後にデフォルトのイベント処理を続行するか否か ( None/False:続行する、True:続行しない )
        # 
        self.keyup = None

        ## マウスのボタンの押し込み時に呼び出されるフック関数
        #
        #  @param x     X座標
        #  @param y     Y座標
        #  @param vk    仮想キーコード
        #  @return      フック関数呼出し後にデフォルトのイベント処理を続行するか否か ( None/False:続行する、True:続行しない )
        # 
        self.mousedown = None

        ## マウスのボタンが離されたときに呼び出されるフック関数
        #
        #  @param x     X座標
        #  @param y     Y座標
        #  @param vk    仮想キーコード
        #  @return      フック関数呼出し後にデフォルトのイベント処理を続行するか否か ( None/False:続行する、True:続行しない )
        # 
        self.mouseup = None

        ## マウスのボタンのダブルクリック時に呼び出されるフック関数
        #
        #  @param x     X座標
        #  @param y     Y座標
        #  @param vk    仮想キーコード
        #  @return      フック関数呼出し後にデフォルトのイベント処理を続行するか否か ( None/False:続行する、True:続行しない )
        # 
        self.mousedblclk = None

        ## クリップボードの内容が変化したときに呼び出されるフック関数
        #
        #  クリップボードの内容を取得するには、ctypesでwin32のAPIを使う必要があります。
        #
        self.clipboard = None
    
    ## フックを解除する
    #
    #  @param self      -
    #  @return          ピクセルフォーマットを表す文字列
    #
    #  フックを明示的に解除します。ガベージコレクタによってHookオブジェクトが自動的に破棄されるときにもフックは解除されますが、明示的に解除したい場合にはこのメソッドを使用します。
    #
    def destroy(self):
        pass

## pyautoの実行時エラーを表す例外オブジェクト
Error = None


## プログラムの起動またはファイル/URLの関連付け実行を行います
#
#  @param verb      実行する操作
#  @param file      実行するまたは開くファイル
#  @param param     パラメタ
#  @param directory 作業ディレクトリ
#  @param swmode    ウインドウ表示モード
#
#  引数verbには、実行する操作を文字列で渡します。指定可能な文字列は対象によって異なりますが、一般的には次のような操作が指定可能です。
#
#  open
#       ファイルを開きます。またはプログラムを起動します。
#  edit
#       ファイルを編集します。
#  properties
#       ファイルのプロパティを表示します。
#
#
#  引数swmodeには、以下のいずれかの文字列(またはNone)を渡します。
#
#  "normal"または""またはNone
#       アプリケーションを通常の状態で起動します。
#  "maximized"
#       アプリケーションを最大化状態で起動します。
#  "minimized"
#       アプリケーションを最小化状態で起動します。
#
def shellExecute(verb,file,param=None,directory=None,swmode=None):
    pass

## メッセージループを処理する
#
#  Hookクラスを使用するには、ウインドウメッセージを処理しなければなりません。
#
#  アプリケーションの中のほかの箇所で、メッセージ処理が実行される場合は、この関数を呼び出す必要はありません。
#
def messageLoop():
    pass


## デバッグメッセージの出力をOn/Offする
#
#  @param enabled   True:デバッグメッセージの出力をOn False:デバッグメッセージの出力を Off
#
#  pyauto自体のデバッグを行うための、デバッグメッセージの出力のOn/Offを制御します。
#
def setDebug(enabled):
    pass

## @} pyauto

