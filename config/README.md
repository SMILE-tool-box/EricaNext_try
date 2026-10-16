# 設定ファイルの書き方

ここには `run_script/run_chain.py`(再構成)と `run_script/run_sim.py`(シミュレーション)が読む設定ファイルを置いています。
このページでは設定ファイルの書き方だけを説明します。スクリプトの使い方は [`run_script/README.md`](../run_script/README.md) にあります。

| 場所 | 読むスクリプト | 中身 |
| --- | --- | --- |
| `config/*.json` | `run_chain.py` | 再構成のチェーン(モジュールの順番とパラメータ、データと較正ファイルの場所) |
| `config/simulation/*.json` | `run_sim.py` | シミュレーション(検出器、物理モデル、線源、乱数、記録、検出器の応答) |

スクリプトには、名前(拡張子なし)かファイルのパスを渡します。
新しい設定は、近いファイルをコピーして書き換えるのが簡単です(このディレクトリの外に置いても構いません)。

## 共通の決まり

- 書式は JSON です。数は単位を付けずに書くので、単位は下の `_units` で確かめてください。
- **`_` で始まるキーはコメント** で、読み込むときに捨てます(モジュールには渡りません)。`_comment`、`_note` など自由に使えます。
- 各モジュールのパラメータには、次の 2 つを付けます。**パラメータを足したときは、この 2 つにも書いてください。**

  | キー | 中身 |
  | --- | --- |
  | `_doc` | `{パラメータ名: 説明}`。全パラメータに書く |
  | `_units` | `{パラメータ名: 単位}`。単位のあるパラメータに書く(例 `"cut_value": "mm"`) |

- 文字列の中の次の置き換え文字は、読み込むときに実際のディレクトリに置き換わります(`ericanext_config.py`)。
  置き換え文字を使わずに絶対パスを書いても構いません。

  | 置き換え文字 | 置き換わるもの |
  | --- | --- |
  | `{data_top}` | 実データと較正ファイル: `<データ>/calibration_data` |
  | `{tpc_caldata}` | TPC だけの較正データ(2017 年の Ba-133 など): `<データ>/TPC_caldata` |
  | `{sim_data}` | シミュレーションのデータ(SMILE_tool_box_v2.20 の `data/` と `emulator_param/`): `<データ>/simulation_data/parameters/smile2p_sim_data` |
  | `{parma_flux}` | PARMA の宇宙線のフラックスのファイル(シミュレーションの設定ファイルだけ): `<データ>/simulation_data/parma_flux` |
  | `{home}` | ホームディレクトリ |

  `<データ>` は環境変数 `SMILE2P_DATA`、無ければ研究室の NAS の `/mnt/nas_eureka/onoda/EricaNext_try_data/SMILE2+`(`ericanext_config.py` の `DATA_DIR`)です。
  中の並びはリポジトリ直下の README の「データの置き場所」にあります。

- 実際に使われる値(置き換え後、コメントを除いたもの)は、次で確かめられます。

  ```bash
  cd config
  python3 -c "import ericanext_config as ec, json; print(json.dumps(ec.load('smile2p_20190701_Per02'), indent=1))"
  python3 -c "import ericanext_config as ec, json; print(json.dumps(ec.load_sim('smile2p_flight_parma_thermal_neutron_d10'), indent=1))"
  ```

---

## 再構成の設定(`config/*.json`)

```json
{
  "_comment": ["SMILE-2+ 20190701 Per02 ground-calibration run.", "..."],
  "period": "Per02",
  "reference": {"setting_mac": "...", "root": "..."},
  "chain": ["ReadData", "CalcFADC", "TPC_Calibration", "HA_CenterGravity", "HA_Calc", "TOT_Skewness", "Reconstruct"],
  "parameters": {
    "ReadData": {
      "data_dir": "{data_top}/sm2+_cal_rawdata/20190701_Per02",
      "reverse_TPC": false,
      "_doc": {"data_dir": "生データ(*.raw)のディレクトリ", "reverse_TPC": "..."}
    },
    "TOT_Skewness": {
      "drift_velocity_file": "{data_top}/sm2+_cal_rawdata/20190701_Per02/drift_velocity.dat",
      "_doc": {"drift_velocity_file": "ドリフト速度のファイル(1 行目 offset [clock]、以降 時間 [hour] と速度 [mm/clock])"}
    }
  }
}
```

| キー | 中身 |
| --- | --- |
| `chain` | つなぐモジュールの順番(`EricaPy` のモジュール名)。`run_chain.py --raw` のときは使わず、`ReadData → RawTreeWriter` になる |
| `parameters` | `{モジュール名: {パラメータ名: 値}}`。書かなかったパラメータはモジュールの既定値。`RawTreeWriter` のように `chain` に無いモジュールの値も書いておける |
| `period` | 測定期間の名前(チュートリアルが表示などに使う) |
| `reference` | SMILE_tool_box の参照(`setting_mac`、出力の `root`)。比較用で、チェーンの実行には使わない |

- 小数のパラメータは `10.0` のように小数点を付けて書きます(`10` だと整数として渡り、型が合わずに止まる)。
- 実データの置き場が `{data_top}` の形と違うときに書き換えるのは、`ReadData.data_dir`、`TPC_Calibration.ParameterFile`、
  `HA_Calc.parameter_dir`、`HA_Calc.Modified_Parameter_Data_Dir`、`TOT_Skewness.drift_velocity_file` です。

| ファイル | 中身 |
| --- | --- |
| `smile2p_20190701_Per02.json`、`..._Per03.json` | SMILE-2+ の地上較正(2019-07-01)。SMILE_tool_box の参照の run と同じ値 |
| `smile2p_sim_d10_recon.json` | シミュレーションで作った raw の再構成(参照のシミュレーションの再構成と同じ値)。`--set ReadData.data_dir=<raw のディレクトリ>` で使う |
| `tpc_caldata_2017_Ba133.json` | TPC だけの Ba-133 の較正 run(2017-12-19/20)。ゲインマップ用。`runs`(run の一覧)と `peaks_keV`(使うピーク [keV])を持つ |

---

## シミュレーションの設定(`config/simulation/*.json`)

節ごとに、使うモジュール(`module`)とそのパラメータ(`parameters`)を書きます。

```json
{
  "_comment": ["..."],
  "environment": {"G4NEUTRONHPDATA": "{home}/opt/geant4-data-extra/G4NDL4.7.1_Ar36fix"},
  "detector": {"module": "SMILE2pGeometry", "parameters": {"geometry_type": "flight", "...": "..."}},
  "physics":  {"module": "SMILEPhysicsList", "parameters": {"em_msc": "gs", "cut_value": 0.001,
               "_units": {"cut_value": "mm"}, "_doc": {"em_msc": "...", "cut_value": "..."}}},
  "source":   {"module": "ParmaCRPrimaryGen", "parameters": {"...": "..."}},
  "geant4":   {"module": "Geant4Body", "parameters": {"random_seed": 1}},
  "pickup":   [{"module": "EdepPickUp", "parameters": {"output": "edep.root"}}],
  "response": [{"module": "SimTrigger", "parameters": {}}, {"module": "TPCEmulator", "parameters": {}}, "..."]
}
```

チェーンは `detector → physics → source → geant4 → StandardUserActionAssembly → pickup → response` の順に組まれます。
`module` は `EricaSimPy`(`simulation_source/modules/`)、無ければ anlgeant4 から探します。

| 節 | 形 | モジュール | 主なパラメータ |
| --- | --- | --- | --- |
| `environment` | `{環境変数: 値}` | (なし。Geant4 を動かす前に環境変数に入れる) | `G4NEUTRONHPDATA`: 中性子の反応のデータ(G4NDL)のディレクトリ |
| `detector` | 1 つ | `SMILE2pGeometry` | `geometry_type`(flight / ground / no_vessel)、`ha_position_file`、`gas_pressure` [atm] |
| `physics` | 1 つ | `SMILEPhysicsList` | `em_msc`、`gd_physics`、`gd155_table` / `gd157_table`、`cut_value` [mm]、`low_energy_limit` [keV] |
| `source` | 1 つ | `ParmaCRPrimaryGen`、`PointSourcePrimaryGen` など | 粒子、エネルギー、位置など |
| `geant4` | 1 つ | `Geant4Body` | `random_seed` など |
| `pickup` | リスト | `EdepPickUp` など | `output`、`ha_off_file` |
| `response` | リスト(無くてもよい) | `SimTrigger`、`TPCEmulator`、`HAEmulator`、`RawWriter2p` | DAQ モード、TPC・HA の応答(SMILE_tool_box と同じ名前)、raw の出力先 `output_dir` |

- anlgeant4 のモジュールの **長さの単位は cm** です(例 `"plane_size": 300.0` は 300 cm = sim.mac の 3000 mm)。必ず `_units` を見てください。
- 3 つの数のリストは 3 次元ベクトルとして渡します(`position` など)。
- `response` を書くと、実データと同じ形式の raw ファイルも書きます。書かなければ `pickup` のツリーだけです。

### 物理モデルの選択肢

| パラメータ | 値 | 中身 |
| --- | --- | --- |
| `environment.G4NEUTRONHPDATA` | `{home}/opt/geant4-data-extra/G4NDL4.7.1_Ar36fix`(既定) | G4NDL4.7.1(JEFF-3.3)に G4NDL4.5 の Ar-36/38 を補ったもの(`scripts/env/make_g4ndl_ar36fix.sh`)。素の 4.7.1 は Ar-36 を Ar-37 のデータで扱い、Ar のガスに偽の (n,α) が入る |
| | `{home}/opt/geant4-data-extra/G4NDL4.5` | SMILE_tool_box(Geant4 10.4)と同じ ENDF/B-VII.1 |
| `physics.em_msc` | `gs`(既定) | Geant4 11 の `G4EmLivermorePhysics`(e± の多重散乱は Goudsmit-Saunderson) |
| | `urban` | e± の多重散乱だけ SMILE_tool_box(Geant4 10.4)と同じ Urban(RangeFactor 0.02 など) |
| `physics.gd_physics` | `true` / `false` | Gd の中性子捕獲のガンマを ANNRI-Gd 系のモデルで作る(SMILE_tool_box の `GdPhysics 1`) |

違いを調べた結果は [`docs/simulation_port_log.md`](../docs/simulation_port_log.md) にあります。

| ファイル | 中身 |
| --- | --- |
| `smile2p_flight_parma_thermal_neutron_d10.json` | flight、PARMA の宇宙線の熱中性子(深さ 10 g/cm²)。SMILE_tool_box の run(`hikaru3_analysis2/BGsim/thermal_neutron_g10/simulation/d10`)と同じ設定 |
| `smile2p_flight_parma_thermal_neutron_d10_raw.json` | 上と同じ + 検出器の応答(`response`)。実データと同じ形式の raw ファイルを書く |
| `smile2p_ground_cs137.json` | ground、Cs-137 の点線源(z = −1626 mm)。ground の形状に要る `etcc_base_no_battery.mqo` が `SMILE_tool_box_v2.20/data` に無いので注意 |
