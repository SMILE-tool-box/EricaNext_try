# シミュレーションの設定ファイル

シミュレーションの設定(検出器、物理モデル、線源、乱数、記録)を 1 つの JSON にまとめたものです。
`run_script/run_sim.py` が読んで、ANLNext のチェーンを組み立てて実行します(先に `source ~/opt/setup_sim.sh`)。

```bash
python3 run_script/run_sim.py smile2p_flight_parma_thermal_neutron_d10 -n 1000000 -o out.root --seed 1
python3 run_script/run_sim.py smile2p_flight_parma_thermal_neutron_d10 -n 1000 -o test.root --set physics.em_msc=urban
```

| オプション | 意味 |
| --- | --- |
| `-n` | 一次粒子の数 |
| `-o` | 出力の ROOT ファイル(`pickup` の最初のモジュールの `output`) |
| `--seed` | 乱数の種(`geant4.random_seed`) |
| `--set 節.キー=値` | パラメータを上書き(何回でも)。例: `physics.em_msc=urban`、`environment.G4NEUTRONHPDATA=...`、`response.TPCEmulator.Vth_Anode=6000` |
| `--raw-dir` | raw ファイルの出力先(`RawWriter2p.output_dir`。`sim_truth.root` と `Cal_HA*.dat` もここ) |

## ファイル

| ファイル | 中身 |
| --- | --- |
| `smile2p_flight_parma_thermal_neutron_d10.json` | flight、PARMA の宇宙線の熱中性子(深さ 10 g/cm²)。SMILE_tool_box の run(`hikaru3_analysis2/BGsim/thermal_neutron_g10/simulation/d10`)と同じ設定 |
| `smile2p_flight_parma_thermal_neutron_d10_raw.json` | 上と同じ + 検出器の応答と raw ファイル(`response` の節)。`--raw-dir` に実データと同じ形式の raw ファイルを書く |
| `smile2p_ground_cs137.json` | ground、Cs-137 の点線源(z = −1626 mm)。ground の形状に要る `etcc_base_no_battery.mqo` が `SMILE_tool_box_v2.20/data` に無いので注意 |

## 節

チェーンは `detector → physics → source → geant4 → StandardUserActionAssembly → pickup → response` の順です。
`module` は EricaSimPy、無ければ anlgeant4 から探します。

| 節 | モジュール | 主なパラメータ |
| --- | --- | --- |
| `environment` | (環境変数) | `G4NEUTRONHPDATA`: 中性子のデータ。既定は G4NDL4.7.1 に Ar-36/38 を補ったもの(下) |
| `detector` | `SMILE2pGeometry` | `geometry_type`(flight / ground / no_vessel)、`ha_position_file`、`gas_pressure` [atm] |
| `physics` | `SMILEPhysicsList` | `em_msc`(`gs` / `urban`)、`gd_physics`、`gd155_table` / `gd157_table`、`cut_value` [mm]、`low_energy_limit` [keV] |
| `source` | `ParmaCRPrimaryGen`、`PointSourcePrimaryGen` など | 粒子、エネルギー、位置など |
| `geant4` | `Geant4Body` | `random_seed` など |
| `pickup` | `EdepPickUp` など(リスト) | `output`、`ha_off_file` |
| `response` | `SimTrigger`、`TPCEmulator`、`HAEmulator`、`RawWriter2p`(リスト、無くてもよい) | DAQ モード、TPC・HA の応答のパラメータ(SMILE_tool_box と同じ名前)、raw の出力先 |

- 文字列の `{sim_data}` は `$SMILE2P_SIM_DATA`、無ければ `~/smile2p_sim_data`(SMILE_tool_box_v2.20/data の写し)、`{home}` はホーム、`{data_top}` は実データの置き場(`config/*.json` と同じ)に置き換わります。
- `_` で始まるキーはコメント。
- anlgeant4 のモジュールの長さの単位は cm。3 つの数のリストは 3 次元ベクトル(`position` など)。

## 物理モデルの選択肢

| パラメータ | 値 | 中身 |
| --- | --- | --- |
| `environment.G4NEUTRONHPDATA` | `{home}/opt/geant4-data-extra/G4NDL4.7.1_Ar36fix`(既定) | G4NDL4.7.1(JEFF-3.3)に G4NDL4.5 の Ar-36/38 を補ったもの(`scripts/sim_env/make_g4ndl_ar36fix.sh`)。素の 4.7.1 は Ar-36 を Ar-37 のデータで扱い、Ar のガスに偽の (n,α) が入る |
| | `{home}/opt/geant4-data-extra/G4NDL4.5` | SMILE_tool_box(Geant4 10.4)と同じ ENDF/B-VII.1 |
| `physics.em_msc` | `gs`(既定) | Geant4 11 の `G4EmLivermorePhysics`(e± の多重散乱は Goudsmit-Saunderson) |
| | `urban` | e± の多重散乱だけ SMILE_tool_box(Geant4 10.4)と同じ Urban(RangeFactor 0.02 など) |
| `physics.gd_physics` | `true` / `false` | Gd の中性子捕獲のガンマを ANNRI-Gd 系のモデルで作る(SMILE_tool_box の `GdPhysics 1`) |

違いの調べた結果は `docs/simulation_port_log.md`。
