# EricaNext チュートリアル

どのノートブックも「1. データのダウンロード」から始まり、単独で上から順に実行できます。

番号の順に読むことを想定しています。データの入手方法は **1_DataIO** の「1. データのダウンロード」にまとめてあり、
他のノートブックはそこを参照します。

| フォルダ | 内容 |
| --- | --- |
| [`0_Introduction/`](0_Introduction/Introduction.ipynb) | SMILE-2+ の構成、ETCC で測る量、**座標系の定義**、検出器の配置図 |
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
| [`3_Backprojection/`](3_Backprojection/Backprojection.ipynb) | 較正データのバックプロジェクション像 |
| `4_MakeCalibration/` | 較正ファイルを作る(全イベントの生データツリーから) |
| ├ [`DriftVelocity`](4_MakeCalibration/DriftVelocity.ipynb) | ドリフト速度と offset(`drift_velocity.dat`)。研究室のマクロ `drift_speed_s2p_v3.C` と同じ手順 |
| ├ [`GainMap`](4_MakeCalibration/GainMap.ipynb) | TPC のゲインマップ(`Calibration*.dat`、$K_a$, $K_b$)。Ba-133 の較正データ(2017-12-19/20、12 run)の 1 ヒットのスペクトルの 30.85 / 8.04 keV のピークにガウス関数を当てて作る |
| └ [`reference/GainMap_Calamity`](4_MakeCalibration/reference/GainMap_Calamity.ipynb) | 参照用: 研究室のプログラム Calamity と同じ手順(ピークの位置は `TSpectrum` のビンの位置)で作ったゲインマップ |

シミュレーションのチュートリアルは別のフォルダ [`../tutorials_simulation/`](../tutorials_simulation/README.md) にあります。

## 準備

1. EricaNext_try をビルドしておく(リポジトリ直下の `README.md` の「インストール」)。
2. データは研究室の NAS(`/mnt/nas_eureka/onoda/EricaNext_try_data/SMILE2+`)にあり、NAS を読める計算機ではそのまま使えます。
   手元にコピーした場合は環境変数 `SMILE2P_DATA` を設定してください(1_DataIO の 1 節、リポジトリ直下の README の「データの置き場所」)。

チュートリアル専用の関数は使わず、実際に使うときと同じ手順で書いています。

- チェーンの実行: `run_script/run_chain.py`(設定ファイル `config/*.json` を指定)をノートブックから `subprocess` で実行
- 結果の読み込み・計算: ROOT の `RDataFrame` と numpy で各ノートブックに直接記述

チェーンの出力(Per02 の先頭 50000 イベントの再構成ツリーと、先頭 5000 イベントの生データツリー)は
`tutorials_caldata/cache/` に置き、各ノートブックで共有します。最初に実行したノートブックがチェーンを流し、以降は再利用します。
4_MakeCalibration は**全イベント**の生データツリー(`cache/Per02_raw_cal_thr10.root`、約 80 分・約 4.6 GB)を使います。
