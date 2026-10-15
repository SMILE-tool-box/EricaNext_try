# シミュレーションの概要

> 移植中です。ここには方針と、SMILE_tool_box(SMILE_tool_box_v2.20)のシミュレーションの中身をまとめています。

## 1. 全体の流れ

SMILE_tool_box では、次の 4 モジュールをつないでシミュレーションしていました(haboc のマクロの `[Def]`)。

| 順 | SMILE_tool_box | 役割 |
| --- | --- | --- |
| 1 | `SMILE_2p` | Geant4 で検出器の形状を作り、粒子を飛ばして、TPC のガスとシンチでのエネルギー損失を記録する。線源の種類(`SourceType`: Point, Isotope, CR(宇宙線), Parallel など)を切り替える |
| 2 | `TPC_emulator` | TPC のエネルギー損失から、電離(W 値・Fano 因子)、拡散、GEM の増幅、しきい値を経て、エンコーダのヒットと FADC 波形を作る |
| 3 | `Emu_HA` | シンチのエネルギー損失から、ピクセルごとのエネルギー分解能を入れて HA の ADC を作る |
| 4 | `EmuWrite2p` | それを**実データと同じ形式の raw ファイル**に書き出す |

最後に実データと同じ形式で書き出すので、シミュレーションの結果も EricaNext の解析チェーン
(ReadData → … → Reconstruct、`../../tutorials_caldata/1_DataIO/`)にそのまま通せます。
EricaNext でもこの形を引き継ぎます。

## 2. EricaNext での作り方

SMILE-3 も同じ仕組みで扱えるように、Geant4 の部分は ANLNext の作者による Geant4 連携の枠組み
**anlgeant4**([ComptonSoft](https://github.com/odakahirokazu/ComptonSoft) の一部)の上に作ります。

| 部分 | 作り方 |
| --- | --- |
| Geant4 の実行、粒子の発生(点線源・平行光・等方・核種) | anlgeant4 の既製のモジュール(`Geant4Body`, `PointSourcePrimaryGen`, `PlaneWavePrimaryGen`, `IsotropicPrimaryGen`, `NucleusPrimaryGen` など) |
| 形状 | GDML(下の 3 節) |
| 物理リスト | haboc の `SMILE_PhysicsList`(Livermore の電磁相互作用、FTFP_BERT_HP、Gd の中性子捕獲など)を `VANLPhysicsList` として移植 |
| 宇宙線(PARMA のフラックス) | `SMILE_2p_PA_CR` を `VANLPrimaryGen` として移植 |
| エネルギー損失の記録、検出器の応答、raw ファイルの書き出し | `EdepPickUp`(SMILE_2p_Step)、`SimTrigger`(DAQ_Mode)、`TPCEmulator`(TPC_emulator)、`HAEmulator`(Emu_HA)、`RawWriter2p`(EmuWrite2p) |

リファレンス(SMILE_tool_box)とはビット単位では一致させず(乱数の使い方が変わるため)、
エネルギー損失のスペクトルや検出効率、実データ(Per02 の Cs-137)との比較で確かめます。

## 3. SMILE-2+ の形状

SMILE_tool_box の `SMILE_2p_Geo` は **GDML を使っていません**。形状は次の 2 種類の組み合わせで、プログラムの中で直接組み立てています。

1. **`.mqo` の 3 次元メッシュ**(Metasequoia の形式)。SMLib の `GGG_Model` が読み込んで Geant4 の立体に変換する。

   | ファイル | 中身 | 使う場面 |
   | --- | --- | --- |
   | `tpc_gas.mqo` | TPC のガス領域 | 常に |
   | `etcc_vessel.mqo` | 圧力容器 | 常に |
   | `etcc_inside.mqo` | 容器の中の構造物 | 常に |
   | `etcc_base.mqo`(`etcc_base_no_battery.mqo`) | 架台 | 常に |
   | `etcc_anti.mqo` | 反同時計数の検出器 | 常に |
   | `air.mqo` | 空気 | 常に |
   | `bCALET.mqo`, `gondola.mqo` | 気球のゴンドラなど | `GeometryType : flight` のときだけ |

2. **C++ のコードで直接書いた立体**(`G4Box` など)。シンチ(HA)のモジュールやピクセル、バラストなど。
   HA の位置と向きは `HAs.pos` から読む(実データの HA_Calc と同じファイル、`../../tutorials_caldata/0_Introduction/`)。

つまり形状の情報が「`.mqo` + コード + `HAs.pos`」に分かれています。

**EricaNext では GDML にまとめます。**

1. 一度だけ、SMILE_2p と同じ手順(`.mqo` の読み込みとコードでの組み立て)で SMILE-2+ の形状を Geant4 の中に作る。
2. anlgeant4 の `WriteGDML` で GDML に書き出す(`.mqo` のメッシュは三角形の面の集まり `G4TessellatedSolid` として書ける)。
3. 以降のシミュレーションは、その GDML を `ReadGDML` で読むだけにする。

形状が 1 つのファイルにまとまり、中身を確認したり他の Geant4 のツールで表示したりできます。
SMILE-3 の形状(まだ作っていない)も、GDML で作れば同じ仕組みに載せられます。

## 4. 必要なデータ

SMILE_tool_box_v2.20 の `data/` 一式(174 MB)を使います。研究室の NAS の `/mnt/nas_eureka/onoda/EricaNext_try_data/SMILE2+/simulation_data/parameters/smile2p_sim_data/`
(設定ファイルの `{sim_data}`)に置いています(元は研究室の `SMILE_tool_box_v2.20/data/`)。

| マクロのパラメータ | ファイル |
| --- | --- |
| `GeometryCADdir` | `SMILE_2p_Geo/`(上の `.mqo`) |
| `HA_OffFile` | `HA_OFF.dat` |
| `IwakiTemplate`(TPC_emulator) | `IwakiTemp.root` |
| `ClusterData`(TPC_emulator) | `cluster.txt` |
| `GainMap`(TPC_emulator) | `param/20190528/Calibration20190130.dat`(実データと同じ TPC の較正ファイル) |
| `E_Res_662`(Emu_HA) | `Pixel_Resolution_20190528.dat` |
| `CR_FluxFile`(宇宙線) | PARMA で作ったフラックスの ROOT ファイル |

## 5. 環境

Geant4 と anlgeant4 は、実データの解析とは別の環境に入れています(`/usr/local` の ANLNext と EricaNext はそのまま)。

| | 場所 |
| --- | --- |
| Geant4 11.4.3(GDML、マルチスレッド、`GEANT4_BUILD_TLS_MODEL=global-dynamic`) | `~/opt/geant4-11.4.3`(物理データは `~/opt/geant4-data-11.4.3`) |
| ANLNext(ベクトル型のパラメータ有効、`ANLNEXT_USE_G4VECTOR=ON`) | `~/opt/anlnext-g4` |
| ComptonSoft(anlgeant4 を含む、Python 拡張あり) | `~/opt/comptonsoft` |

作り方は `scripts/sim_env/`(`build_geant4.sh` → `build_anlnext_comptonsoft.sh`)、作業の記録は `docs/simulation_port_log.md` にあります。
シミュレーションを使うときは、先に環境を読み込みます(`scripts/sim_env/setup_sim.sh` を `~/opt/` に置いたもの)。

```bash
source ~/opt/setup_sim.sh
```

- `GEANT4_BUILD_TLS_MODEL=global-dynamic` は、Python から Geant4 のライブラリを読み込むために必要です
  (既定の設定だと `cannot allocate memory in static TLS block` で止まる)。
- anlgeant4 は ANLNext をベクトル型のパラメータ有効でビルドしてあることを前提にしているので、別に ANLNext を入れています。
- 中性子のデータは G4NDL4.7.1 に **Ar-36 / Ar-38 のファイルを G4NDL4.5 から補ったもの**(`scripts/sim_env/make_g4ndl_ar36fix.sh` で作る、
  `setup_sim.sh` が `G4NEUTRONHPDATA` に設定)。G4NDL4.7.1 のままだと Ar-36 が Ar-37 のデータで扱われ、Ar のガスに偽の (n,α) が入ります
  (熱中性子で約 6.7 b。`docs/simulation_port_log.md`)。
