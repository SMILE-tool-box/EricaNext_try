# EricaNext シミュレーションのチュートリアル

実データのチュートリアルは [`../tutorials_caldata/`](../tutorials_caldata/README.md) にあります。
座標系と検出器の構成は [`../tutorials_caldata/0_Introduction/`](../tutorials_caldata/0_Introduction/Introduction.ipynb) を参照してください。

| フォルダ | 内容 |
| --- | --- |
| [`0_Introduction/`](0_Introduction/README.md) | シミュレーションの全体の流れ、EricaNext での作り方、SMILE-2+ の形状(.mqo と GDML)、必要なデータと環境 |
| [`1_DataIO/`](1_DataIO/DataIO_example.ipynb) | シミュレーションの実行(`run_sim.py`)、出力(エネルギー損失のツリー、raw、真の値)の読み方、raw の再構成(`run_chain.py`)と真の値との比較 |
| [`2_PointSourceSim/`](2_PointSourceSim/) | 点線源のシミュレーション(較正データと同じ配置の Cs-137 など)(準備中) |
| [`3_ParallelBeamSim/`](3_ParallelBeamSim/) | 平行光シミュレーション(準備中) |
| [`4_CosmicBGSim/`](4_CosmicBGSim/) | 宇宙線バックグラウンドシミュレーション(準備中) |
| [`5_PARMAFlux/`](5_PARMAFlux/README.md) | PARMA で宇宙線のフラックスを作る(`4_CosmicBGSim` の入力)(準備中) |
