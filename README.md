# win32api-progs

* Visual Studio による Win32API プログラミングの練習

## プロジェクト

* **001-MainWindow**
	* ウィンドウ表示の基本形
	* WM_PAINT: 背景を塗りつぶす
	* WM_SETCURSOR: マウスカーソルを設定する
	* WM_DESTROY: PostQuitMessage()
* **002-WindowSize**
	* WM_GETMINMAXINFO: 最大サイズと最小サイズを設定する
	* タイトルバーの最大化ボタンを無効化する
	* 終了時のサイズをINIファイルに保存し、次回起動時に復元する
	* util.cpp にユーティリティ関数を追加
		* SaveSettings(): 設定をINIファイルに保存
		* LoadSettings(): 設定をINIファイルから復元
		* MyDrawText(): 改行(\n)を含むテキストを画面に表示