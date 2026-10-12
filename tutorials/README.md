# EricaNext チュートリアル

どのノートブックも「1. データのダウンロード」から始まり、単独で上から順に実行できます。

| フォルダ | 内容 |
| --- | --- |
| [`DataIO/`](DataIO/DataIO_example.ipynb) | Erica によるデータ再構成。生データから再構成ツリー `Reconstruct/single_hit` を作るまで |
| `DataIO/modules/` | 各モジュールの解説 |
| ├ [`RawTreeWriter/`](DataIO/modules/RawTreeWriter/RawTreeWriter.ipynb) | 同期した生データを TTree に書き出す(較正・診断用) |
| ├ [`ReadData/`](DataIO/modules/ReadData/ReadData.ipynb) | 生データの読み込みと検出器間のイベント同期 |
| ├ [`CalcFADC/`](DataIO/modules/CalcFADC/CalcFADC.ipynb) | TPC の FADC 波形 → 電荷 |
| ├ [`TPC_Calibration/`](DataIO/modules/TPC_Calibration/TPC_Calibration.ipynb) | アノード×カソード同時計数 → 反跳電子エネルギー |
| ├ [`HA_CenterGravity/`](DataIO/modules/HA_CenterGravity/HA_CenterGravity.ipynb) | シンチレータ PMT の重心 |
| ├ [`HA_Calc/`](DataIO/modules/HA_Calc/HA_Calc.ipynb) | ピクセル同定とガンマ線エネルギー |
| ├ [`TOT_Skewness/`](DataIO/modules/TOT_Skewness/TOT_Skewness.ipynb) | 電子飛跡(散乱点・方向・飛程) |
| └ [`Reconstruct/`](DataIO/modules/Reconstruct/Reconstruct.ipynb) | 1 イベント 1 行の再構成ツリー |
| [`EventSelection/`](EventSelection/EventSelection.ipynb) | イベント選択(`SetCrit`)。カットごとのエネルギースペクトルの変化 |
| [`Backprojection_caldata/`](Backprojection_caldata/Backprojection_caldata.ipynb) | 較正データのイベント選択とバックプロジェクション像 |
| [`ParallelBeamSim/`](ParallelBeamSim/) | 平行光シミュレーション(準備中) |
| [`CosmicBGSim/`](CosmicBGSim/) | 宇宙線バックグラウンドシミュレーション(準備中) |

## 準備

1. EricaNext_try をビルドしておく(リポジトリ直下の `README.md` の「インストール」)。
2. データを研究室サーバーから `~/smile2p_data` にコピーする(各ノートブックの 1 節にコマンドがあります)。
   別の場所に置いた場合は環境変数 `SMILE2P_DATA` を設定してください。

チュートリアル専用の関数は使わず、実際に使うときと同じ手順で書いています。

- チェーンの実行: `run_script/run_chain.py`(設定ファイル `config/*.json` を指定)をノートブックから `subprocess` で実行
- 結果の読み込み・計算: ROOT の `RDataFrame` と numpy で各ノートブックに直接記述

チェーンの出力(Per02 の先頭 50000 イベントの再構成ツリーと、先頭 5000 イベントの生データツリー)は
`tutorials/cache/` に置き、各ノートブックで共有します。最初に実行したノートブックがチェーンを流し、以降は再利用します。
