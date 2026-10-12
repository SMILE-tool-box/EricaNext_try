# EricaNext チュートリアル

どのノートブックも「1. データのダウンロード」から始まり、単独で上から順に実行できます。

番号の順に読むことを想定しています。データの入手方法は **1_DataIO** の「1. データのダウンロード」にまとめてあり、
他のノートブックはそこを参照します。

| フォルダ | 内容 |
| --- | --- |
| [`1_DataIO/`](1_DataIO/DataIO_example.ipynb) | Erica によるデータ再構成。データのダウンロードから再構成ツリー `Reconstruct/single_hit` を作るまで |
| `1_DataIO/modules/` | 各モジュールの解説(番号はチェーンの順) |
| ├ [`1_ReadData/`](1_DataIO/modules/1_ReadData/ReadData.ipynb) | 生データの読み込みと検出器間のイベント同期 |
| ├ [`2_CalcFADC/`](1_DataIO/modules/2_CalcFADC/CalcFADC.ipynb) | TPC の FADC 波形 → 電荷 |
| ├ [`3_TPC_Calibration/`](1_DataIO/modules/3_TPC_Calibration/TPC_Calibration.ipynb) | アノード×カソード同時計数 → 反跳電子エネルギー |
| ├ [`4_HA_CenterGravity/`](1_DataIO/modules/4_HA_CenterGravity/HA_CenterGravity.ipynb) | シンチレータ PMT の重心 |
| ├ [`5_HA_Calc/`](1_DataIO/modules/5_HA_Calc/HA_Calc.ipynb) | MDF 変換・ピクセル同定とガンマ線エネルギー |
| ├ [`6_TOT_Skewness/`](1_DataIO/modules/6_TOT_Skewness/TOT_Skewness.ipynb) | 電子飛跡(散乱点・方向・飛程) |
| ├ [`7_Reconstruct/`](1_DataIO/modules/7_Reconstruct/Reconstruct.ipynb) | 1 イベント 1 行の再構成ツリー |
| └ [`8_RawTreeWriter/`](1_DataIO/modules/8_RawTreeWriter/RawTreeWriter.ipynb) | 同期した生データを TTree に書き出す(較正・診断用。ReadData の直後につなぐ) |
| [`2_EventSelection/`](2_EventSelection/EventSelection.ipynb) | イベント選択(`SetCrit`)。カットごとのエネルギースペクトルの変化 |
| [`3_Backprojection_caldata/`](3_Backprojection_caldata/Backprojection_caldata.ipynb) | 較正データのバックプロジェクション像 |
| [`4_ParallelBeamSim/`](4_ParallelBeamSim/) | 平行光シミュレーション(準備中) |
| [`5_CosmicBGSim/`](5_CosmicBGSim/) | 宇宙線バックグラウンドシミュレーション(準備中) |

## 準備

1. EricaNext_try をビルドしておく(リポジトリ直下の `README.md` の「インストール」)。
2. データを研究室サーバーから `~/smile2p_data` にコピーする(手順は 1_DataIO の 1 節)。
   別の場所に置いた場合は環境変数 `SMILE2P_DATA` を設定してください。

チュートリアル専用の関数は使わず、実際に使うときと同じ手順で書いています。

- チェーンの実行: `run_script/run_chain.py`(設定ファイル `config/*.json` を指定)をノートブックから `subprocess` で実行
- 結果の読み込み・計算: ROOT の `RDataFrame` と numpy で各ノートブックに直接記述

チェーンの出力(Per02 の先頭 50000 イベントの再構成ツリーと、先頭 5000 イベントの生データツリー)は
`tutorials/cache/` に置き、各ノートブックで共有します。最初に実行したノートブックがチェーンを流し、以降は再利用します。
