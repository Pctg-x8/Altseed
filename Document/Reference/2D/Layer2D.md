﻿## レイヤー2D (Layer2D)

### 概要

レイヤー2Dは2D機能を持った[レイヤー](../Basic/Layer.md)です。レイヤーを継承することでレイヤーに2D機能を追加しています。基本的にレイヤー2Dにオブジェクト2Dを追加することで画像を描画します。ただし、それだけでは実現しにくい描画のために、レイヤー2Dにも直接画像を描画する機能を持っています。

カメラオブジェクト2Dが追加されていない場合、ウインドウサイズが(x,y)とすると左上を(0,0)、右下を(x,y)とした四角形の領域が描画に表示されます。カメラオブジェクト2Dが追加された場合、カメラオブジェクト2Dの設定に従って描画が表示されます。

レイヤー共通のメソッド等は、[レイヤー](../Basic/Layer.md)を参照してください。

### 主なメソッド

| 名称 | 説明 |
|---|---|
| AddObject | オブジェクト2Dをレイヤーに追加する。 |
| RemoveObject | オブジェクト2Dをレイヤーから取り除く。 |
| DrawSpriteAdditionally | 直接四角形をレイヤーに描画する。 |


### 主なプロパティ

### 主なイベント

### 使用方法

ほぼ全ての2Dのサンプルで使用されているので、そちらを参照してください。