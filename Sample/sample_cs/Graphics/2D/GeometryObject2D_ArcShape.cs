﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace sample_cs.Graphics._2D
{
    /// <summary>
    /// 図形を表示する。
    /// </summary>
    class GeometryObject2D_ArcShape : ISample
    {
        public void Run()
        {
            // aceを初期化する
            asd.Engine.Initialize("GeometryObject2D_ArcShape", 640, 480, new asd.EngineOption());

            var texture = asd.Engine.Graphics.CreateTexture2D("Data/Texture/Sample1.png");

            //図形描画クラスのコンストラクタを呼び出す
            var geometryObj = new asd.GeometryObject2D();

            //図形描画クラスをレイヤーに追加する。
            asd.Engine.AddObject2D(geometryObj);

            //弧を図形描画クラス1にて描画する。
            {
                //弧の外径、内径、頂点数、中心位置、開始頂点番号、終了頂点番号を指定。
                var arc = new asd.ArcShape();
                arc.OuterDiameter = 100;
                arc.InnerDiameter = 10;
                arc.NumberOfCorners = 96;
                arc.Position = new asd.Vector2DF(300, 100);
                arc.StartingCorner = 90;
                arc.EndingCorner = 5;

                //弧を描画する図形として設定し、合成するテクスチャも設定。
                geometryObj.Shape = arc;
                geometryObj.Texture = texture;
                geometryObj.Position = new asd.Vector2DF(0, 0);
            }

            // aceが進行可能かチェックする。
            while (asd.Engine.DoEvents())
            {
                // aceを更新する。
                asd.Engine.Update();
            }

            // aceを終了する。
            asd.Engine.Terminate();
        }
    }
}