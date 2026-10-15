# PARMA の使い方(宇宙線のフラックスの作り方)

> 準備中です。

宇宙線バックグラウンドのシミュレーション(`4_CosmicBGSim`)で入射させる粒子のフラックスを、
PARMA(EXPACS)で作る手順をここにまとめる予定です。

## 分かっていること

シミュレーションの線源 `ParmaCRPrimaryGen` は、PARMA のフラックスを ROOT ファイルで読みます
(設定ファイルの `source.parameters.flux_file`、SMILE_tool_box の sim.mac の `CR_FluxFile`)。

| ヒストグラム | 粒子 | 横軸 | 縦軸 | 中身 |
| --- | --- | --- | --- | --- |
| `h2d0` | 中性子 | エネルギー [MeV](対数の不等間隔のビン) | cosθ(−1〜1) | フラックス [/cm²/s/sr/MeV] |
| `h2d1` | 陽子 | 同上 | 同上 | 同上 |
| `h2d31` / `h2d32` | 電子 / 陽電子 | 同上 | 同上 | 同上 |
| `h2d33` | ガンマ線 | 同上 | 同上 | 同上 |

参照の run で使ったファイル(深さ 3、6、10、20、30、60、100 g/cm²)。熱中性子のものは NAS の `simulation_data/parma_flux/`(設定ファイルの `{parma_flux}`)にも置いてあります(元と同じファイル):

- 熱中性子: `hikaru3_analysis2/BGsim/thermal_neutron_g10/parma_flux/parma_flux_d<深さ>_g10_thermal.root`
  (エネルギーは 1e-9〜0.01 MeV、700 × 1000 ビン。`h2d0` だけに中身がある)
- 中性子: `hikaru3_analysis2/BGsim/neutron_g10/parma_flux/parma_flux_d<深さ>_g10_neutron.root`
