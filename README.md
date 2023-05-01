# FSDirBrowseTool
 
Version 0.1 Preview

指定ディレクトリに含まれるファイルの情報を表示します。

古いWDKとSDKを使ってWindowsのネイティブAPIを使用するWin32アプリケーションを作成するために、利用可能なソースコードを寄せ集め、整理したものを公開しています。
現状では実用的なツールというより、MDI Win32アプリケーション作成のひな形（サンプルアプリケーション）という位置づけです。

そのため、アプリケーションの機能として表示されるファイル情報は基本的なもののみで、最低限の実装となっています。今後徐々に追加する予定です。

**このアプリケーションは開発中のため、現状の仕様，機能，構成，UIデザイン，ファイル名，ビルド構成などは常に変更されます。**

<br>

## 使い方

1. wfsdirbrowse.exeを実行します。

1. メニューの `File > New` を選択します。

カレントディレクトリの内容が表示されます。現状ではUIによる場所の指定方法を提供していない為、開いた場所と同じボリューム内のディレクトリ間のみ移動できます。

直接場所を指定するには、コマンドライン引数を利用します。

以下は、起動時に Windowsディレクトリと HarddiskVolume1のルート 及び HarddiskVolume2\foo を開く例です。


    wfsdirbrowse C:\windows \Device\HarddsikVolume1\ \??\HarddiskVolume2\foo

<br>

## Build

### 開発環境
ソースからビルドするには　Windows Driver Kit Version 7.1.0 (WDK) と Windows SDK for Windows 7 and .NET Framework 4 (Veriosn 7.1)が必要です。

https://www.microsoft.com/en-us/download/details.aspx?id=11800

https://www.microsoft.com/en-us/download/details.aspx?id=8442


現在のビルド環境は、上記WDKとSDKが以下の位置にインストールされている前提になっているので下記の場所へインストールしてください。

> **Warning**   
sourcesファイル内に記述されたWDK/SDKルートパスがハードコードされているためです。

WDK   
`C:\WinDDK\7600.16385.1`

SDK   
`C:\WinSDK\7.1`

別の場所にインストールされている場合は、その場所へのリンクを持つ上記パスと同じ名前のシンボリックリンクディレクトリ
（ジャンクションポイント）をC:ドライブのルートに作成してください。

例)
`C:\WinDDK\7600.16385.1 -> C:\Program Files\Windows Driver Kit 7.1.0`

独自のインストール先を設定したい場合は、sourcesファイルの内容を編集する必要があります。

> **Warning**   
sourcesファイルに記述するパスにはスペースを含めないでください。

<br>

### ビルド方法
スタートメニューの以下の項目を開きます。

`Windows Driver Kits > WDK 7600.16385.1 > Build Environments>Windows 7`

から

64ビット版をビルドする場合は、`x64 Free Build Environment`

32ビット版をビルドする場合は、 `x86 Free Build Environment`

のどちらかを開きます。

> **Warning**   
Windows 10の目的の環境を開けない場合があります。その場合はスタートメニューからではなく、直接目的のビルド環境のシェルリンクファイルを開いてください。

<br>
コマンドプロンプトが開くので、ソースの展開先ディレクトリへ移動して、以下の`build`コマンドを実行します。
<br>
<br>

    build -c

最初のビルドでは以下のオプションをお勧めします。

    build -c -M 1



## License

Copyright (C) YAMASHITA Katsuhiro. All rights reserved.

Licensed under the [MIT](LICENSE) License.
