﻿## エンジン (Engine)

### 概要

Altseedの機能を提供します。初期化、終了処理及び、殆どの機能を使用するためのプロパティを持っています。Altseedを使用するためには必ずエンジンを使用する必要があります。

エンジンでAltseedの初期化を行えます。その時、様々なオプションを指定できます。例えば、フルスクリーンかどうかなどです。

初期化時のオプションとしてリロード機能の有効無効をIsReloadingEnabledを変更することで設定できます。リロードはゲームを起動中でも画像を編集するとリアルタイムにゲーム中に反映してくれう機能です。詳細は[リロード](./../Misc/Reloading.md)を参照してくだい。

シーンの変更や終了処理もエンジンで行います。

フレームレート、及びゲーム内時間に関する詳細に関しては[基本設計](./../BasicDesign.md)を参照してください。


### 主なメソッド

| 名称 | 説明 |
|---|---|
| Initialize | Altseedを初期化します。使用開始時に必ず実行します。 |
| DoEvents | Altseedのフレーム開始処理を実行します。毎フレームの開始時に必ず実行します。falseを返した場合、ACEを終了させる必要があります。 |
| Update | Altseedの毎フレームの更新処理を実行します。毎フレーム、かならず実行します。 |
| Terminate | Altseedの終了処理を実行します。Altseedを終了時に必ず実行します。 |
| ChangeScene | 更新するシーンを変更します。 |
| ChangeSceneWithTransition | トランジションを適用し、更新するシーンを変更します。 |
| Close | Altseedのウインドウを閉じます。 |
| TakeScreenshot | スクリーンショットを撮影し、画像として保存します。 |
| CaptureScreenAsGifAnimation | 画面を録画し、gifアニメとして保存します。 |

### 主なプロパティ

#### フレームレート系

| 名称 | 説明 |
|---|---|
| CurrentFPS | 現在のFPS |
| TargetFPS | 目標のFPS。このFPSに近づくように内部の更新が実行されます。 |
| DeltaTime | 1フレームの処理にかかった時間 |
| TimeSpan | 処理にかかった時間に対する時間指定可能な機能が進行する時間の倍率 |
| FramerateMode | フレームレート制御方法 |

### 主なイベント

なし

### 使用方法

全てのサンプルで使用されているので、そちらを参照してください。
