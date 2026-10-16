# SMILE_tool_box シミュレーションの EricaNext への移植ログ

対象: SMILE_tool_box_v2.20 の `SMILE_2p` / `TPC_emulator` / `Emu_HA` / `EmuWrite2p`
(haboc のマクロ `[Def] SMILE_2p, TPC_emulator, Emu_HA, EmuWrite2p`)。
方針と全体像は `tutorials_simulation/0_Introduction/README.md`。

---

## 2026-10-12 方針

- **Geant4 の部分は anlgeant4(ComptonSoft の一部、ANLNext の作者による Geant4 連携)の上に作る。**
  SMILE-3 も同じ仕組みに載せるため(形状は GDML で差し替え、粒子の発生・物理・記録は共通)。
  SMILE_2p をそのまま移す案は、リファレンスとのビット一致を狙えるが SMILE-2+ 専用になるので採らない。
- リファレンスとの比較は統計的に行う(乱数の使い方が変わるのでイベントごとの一致は狙わない)。
  最後に Per02 の実データ(Cs-137 点線源)と比べる。
- 移植の元は `SMILE_tool_box_v2.20`(マクロと同じ版)。`SMILE3_tool_box` にある新しい版
  (`SMILE_2p_20220411` など)は使わない。
- SMILE-3 の形状はまだ作られていない。

## 2026-10-12 データ

- `hikaru3/SMILE_tool_box_v2.20/data/`(174 MB)を `~/smile2p_sim_data/` にコピー(元とは `diff -rq` で一致)。
  `hikaru3` は読むだけで、中は変更していない。
- 最初 `~/smile2p_data/sim/` に置いたら、`config/ericanext_config.py` の `data_top()` が
  `~/smile2p_data` を実データの置き場と判定してしまった(`Per02_ETCC` がないのでチェーンが動かなくなる)。
  → 置き場所を `~/smile2p_sim_data/` に変えた。
- SMILE-2+ の形状は GDML ではなく、`.mqo`(Metasequoia のメッシュ、SMLib の `GGG_Model` で読む)と
  コードで書いた立体(HA など、`HAs.pos` で配置)の組み合わせだった。

## 2026-10-12 環境の構築

すべて `~/opt/` に入れた。`/usr/local` の ANLNext(実データの EricaNext が使う)は変えていない。
スクリプトは `scripts/sim_env/`(`build_geant4.sh` → `build_anlnext_comptonsoft.sh`)、環境は `setup_sim.sh`。
ビルドの生の出力は `~/opt/logs/`(リポジトリには入れない)。

| | 版 | 場所 |
| --- | --- | --- |
| Geant4 | 11.4.3(GDML、MT、`GEANT4_BUILD_TLS_MODEL=global-dynamic`) | `~/opt/geant4-11.4.3`、物理データ `~/opt/geant4-data-11.4.3` |
| xerces-c, expat | システム(`dnf install xerces-c-devel expat-devel`、3.2.5 / 2.5.0) | `/usr` |
| ANLNext | c22eb3e、`ANLNEXT_USE_G4VECTOR=ON` | `~/opt/anlnext-g4` |
| ComptonSoft | bbe3486、`CS_ALL_OFF` + `CS_USE_GDML` + `CS_USE_PYTHON`(Ruby・Qt なし) | `~/opt/comptonsoft` |

詰まったところ:

1. **xerces-c / expat が無い** → 最初は自前でビルドしかけたが、`sudo dnf install xerces-c-devel expat-devel`
   してもらい、システムのものを使う形に変更(`GEANT4_USE_SYSTEM_EXPAT=ON`)。
2. **anlgeant4 は ANLNext をベクトル型パラメータ有効でビルドしてあることが前提**(`-DANLNEXT_USE_HEPVECTOR`)。
   `/usr/local` の ANLNext は無効なので、`ANLNEXT_USE_G4VECTOR=ON`(Geant4 の CLHEP を使う)で別に入れた。
3. ANLNext の cmake で `RUBY_CONFIG_FILE_DIR-NOTFOUND` → `/usr/local` 版のビルドと同じく `-DRUBY_CONFIG_FILE_DIR=/usr/include` を渡す。
4. ANLNext のリンクで `cannot find -lG4clhep`(リンク先のディレクトリを指定していない)→ `LIBRARY_PATH` に Geant4 の `lib64` を入れる。
5. **Python で anlgeant4 を import すると `libG4geometry.so: cannot allocate memory in static TLS block`。**
   Geant4 の MT 版の既定のスレッドローカル変数の方式(initial-exec)が、Python から後でライブラリを読む使い方に合わない。
   → `GEANT4_BUILD_TLS_MODEL=global-dynamic` で Geant4 を作り直して解消。
   作り直しのとき物理データ(約 2 GB)を再ダウンロードしないよう、`~/opt/geant4-data-11.4.3` に移して
   `GEANT4_INSTALL_DATA=OFF -DGEANT4_INSTALL_DATADIR=...` で参照させた。
   Geant4 11 は `G4LEDATA` などの環境変数を設定せず、データの場所をビルド時に埋め込む(`geant4-config --check-datasets` で全部 INSTALLED を確認)。

確認: `source ~/opt/setup_sim.sh` のあと、Python で `import anlnext`、`import anlgeant4` が通る。

anlgeant4 で Python から使えるモジュール: `Geant4Body`, `Geant4Simple`, `VANLPhysicsList`, `VANLGeometry`,
`ReadGDML`, `WriteGDML`, `VANLPrimaryGen`, `BasicPrimaryGen`, `PointSourcePrimaryGen`, `PlaneWavePrimaryGen`,
`PlaneWaveRectanglePrimaryGen`, `GaussianBeamPrimaryGen`, `IsotropicPrimaryGen`, `PrimaryGenUniformSourceInVolume`,
`NucleusPrimaryGen`, `NucleusPrimaryGenInVolume`, `StandardUserActionAssembly` など。
ComptonSoft 自身の物理リストは Ruby 側にしか出ていないので、SMILE の物理リストは EricaNext 側に作る。

## 2026-10-12 シミュレーション用パッケージの骨組み(`simulation/`、2026-10-14 に `simulation_source/` に改名)

実データ用(`source/`、`/usr/local` の ANLNext)とは使う ANLNext が違う(ベクトル有効)ので、別の CMake プロジェクトにした。

```
simulation_source/
├── CMakeLists.txt          libEricaSim.so と Python 拡張 EricaSimPy
├── include/, src/          モジュール
├── pyext/EricaSimPy.i
└── test/                   動作確認用(smoke_test.gdml, run_smoke_test.py)
```

ビルド: `source ~/opt/setup_sim.sh; cmake -S simulation_source -B simulation/build -DCMAKE_INSTALL_PREFIX=simulation/install; cmake --build simulation/build --target install`

作ったモジュール:

| モジュール | 元 | 中身 |
| --- | --- | --- |
| `SMILEPhysicsList`(`VANLPhysicsList`) | haboc `SMILE_PhysicsList` | Livermore EM、崩壊・放射性崩壊、HP 弾性散乱、FTFP_BERT_HP、stopping、イオン、step limiter。カット 1 μm、カットの表は 250 eV から |
| `EdepPickUp`(`VAppendableUserActionAssembly`) | `SMILE_2p_Step` | エネルギー損失のあるステップを、TPC のガス(`TPC_detectable`)・シンチ(名前に `scinti`)・`anti` ごとに記録。位置は SMILE_2p_Step と同じ `track->GetPosition()`。一次粒子の情報も。TTree `edep` に書く |

チェーン(ComptonSoft の `setup_minimal` と同じ順): 形状(`ReadGDML`) → 物理(`SMILEPhysicsList`) → 粒子の発生
(`PointSourcePrimaryGen`) → `Geant4Body` → `StandardUserActionAssembly` → `EdepPickUp`。

詰まったところ:

6. `G4StepLimiterPhysics(ver)` がコンパイルできない → Geant4 11 では引数に verbose を取らない。引数なしにした。
7. `import EricaSimPy` で `libEricaSim.so` が見つからない → Python 拡張に RPATH が付いていなかった。
   `CMAKE_INSTALL_RPATH` をターゲットを作る前に設定。
8. `anlgeant4` に `ReadGDML` が無い → ComptonSoft を `CS_ALL_OFF=ON -DCS_USE_GDML=ON` で設定していたが、
   `CS_ALL_OFF` が後から GDML を切っていた。`CS_ALL_OFF` を外してビルドし直し(他の追加機能はもともと既定で無効)。
9. 3 次元ベクトルのパラメータ(`position` など)を Python のリストで渡すと型エラー
   (ANLNext の Python の `with_parameters` はリストを `vector<double>` にする)→
   `a.with_setter(lambda m: m.set_parameter("position", x, y, z))` で渡す。anlgeant4 の長さの単位は **cm**。
10. `ReadGDML` がネットワーク越しに GDML のスキーマを取りに行って失敗 → `validate: False`。

**動作確認(テスト用の形状。SMILE-2+ の形状ではない)**: TPC のガスの箱(Ar 2 気圧、307.2 × 307.2 × 305 mm)と
底面の GSO の板(320 × 320 × 13 mm)に、Cs-137(661.657 keV)を (0, 0, −1626) mm から半頂角 0.25 rad の円錐で 20000 個飛ばした。

- エネルギーを落としたイベント 1333(シンチ 1303、そのうち全吸収 771 = 59%、TPC 38(中央値 83 keV)、両方 8)
- 入射エネルギー・向き・各量の大きさは妥当。

## 2026-10-12 SMILE-2+ の形状

SMILE_tool_box のコードを `simulation_source/vendor/` にそのまま取り込み(`vendor/README.md`)、
`SMILE2pGeometry`(`VANLGeometry`)から `SMILE_2p_Geo` を呼ぶ形にした。

| 取り込んだもの | 元 | 直したところ |
| --- | --- | --- |
| `vendor/ggg/` | SMLib `GGG`(`.mqo` → `G4TessellatedSolid`) | なし |
| `vendor/haboc_g4/SMILE_Material` | haboc | なし |
| `vendor/ha_common/` | SMLib `HA_common`, `HAPositionManager` | `SM_Log.h` の代わりの最小限のファイル(`namespace smlib {}` も) |
| `vendor/smile2p_geo/SMILE_2p_Geo` | Erica `SMILE_2p` | `G4VisAttributes::Invisible` → `GetInvisible()`(Geant4 11) |

`SMILE2pGeometry` のパラメータ: `geometry_dir`(GeometryCADdir)、`geometry_type`(ground / flight / no_vessel)、
`ha_position_file`(HA_PositionFile)、`gas_pressure`(TPC_Gas_Pressure、atm)。

動作確認(`simulation_source/test/run_smile2p_cs137.py`、ground、Cs-137 を (0, 0, −1626) mm から半頂角 0.25 rad、2000 個):

- 50 個のメッシュのボリュームの重なりチェックはすべて OK。
- **座標系は実データと同じ**(原点 μ-PIC、z 下向き)。TPC でのエネルギー損失は x −150〜69, y −140〜141, z 48〜319 mm、シンチは z 149〜368 mm。
- エネルギーを落としたイベント 320(シンチ 319、全吸収 105、TPC 7、anti 7)。シンチのボリューム名は `scinti_side_21_2_62` のように HA 番号・PMT・ピクセル。

**GDML への書き出し**: `WriteGDML` で 37 MB(メッシュが多い)。ボリューム名はそのまま残る(`TPC_detectable` など)。
読み直すとイベントごとに結果がずれた → `SMILE_2p_Geo` が `TPC_detectable` に付けている**最大ステップ長 40 μm(`G4UserLimits`)は GDML に書き出されない**ため。
`StepLimit` モジュール(名前で指定したボリュームに最大ステップ長を付ける。`Geant4Body` の後につなぐ)を作り、付け直すと
全 320 イベントが一致(エネルギーの差は最大 2×10⁻¹⁰ keV、GDML の数値の桁数による丸め)。

気づいたこと(要確認):

- **`etcc_base_no_battery.mqo`(地上の架台)がない。** コピー元の `SMILE_tool_box_v2.20/data/SMILE_2p_Geo/` にもない。
  ground では `ERROR: can't load .../etcc_base_no_battery.mqo (GGG)` と出て、架台なしで組み立てられる
  (`etcc_base.mqo` はある。SMILE_2p_Geo のコメントアウトされた行では ground でも `etcc_base.mqo` を使っていた)。
- **HA の位置のファイルが 2 つあり、値が違う。** シミュレーションの既定 `data/HA_Parameters_20190403_position_only/HAs.pos` は
  底面の HA の z が 354.6 mm・側面が ±182.1 mm、実データの再構成(HA_Calc)の `param/20190528/HAs.pos` は 365.9 mm・±188.9 mm。
- `SMILE_2p_Geo` はカレントディレクトリに `SMILE_2p.weight`(質量の一覧)を書き出す。

## 2026-10-12 リファレンスのシミュレーションの実行結果との比較(形状)

リファレンス: `hikaru3_analysis2/BGsim/thermal_neutron_g10/simulation/`(読み取りのみ)。
深さごと(`d3`〜`d100`)に `sim.mac` で 20 本(`v1`〜`v20`)を並列実行(`hikaru_sim_para.sh`)。
各 `v*/` に `HabocStatus.scl`(実際に使われたパラメータ)、`erica.log`、`SMILE_2p.weight`(部品ごとの質量)、
`haboc_etcc_emulator.root`、`rawdata/`(raw ファイル 129 個、314 MB)、`ETCCcount.scl` がある。

- `HabocStatus.scl` によると **`HA_PositionFile` は既定の `data/HA_Parameters_20190403_position_only/HAs.pos`**
  (マクロでは指定していない)。実データの再構成の `param/20190528/HAs.pos` とは値が違う(上の要確認)。
- flight の配置では `bCALET.mqo`, `gondola.mqo`, `etcc_base.mqo` を読む(`etcc_base_no_battery.mqo` は ground だけ)。
- `erica.log` に `Can't find .../data/PMT_Map/HA01_4.map` などが 11 個(Emu_HA の PMT のマップ。移植のときに確認)。
- `ETCCcount.scl`(d10/v1): 生成 43,317,582、TPC 方向 710,755、トリガ 7,640。

**形状の一致**: 同じ設定(flight、同じ `HAs.pos`、2 気圧)で `SMILE2pGeometry` を組み立てると、
`SMILE_2p.weight` の質量が表示されている桁まですべて一致した(全体 729.202 kg、圧力容器 65.2381、ETCC の容器 90.8362、
ETCC の中 52.4398、下部の回路 138.403、anti 2.14172、ゴンドラ 53.3303、バラスト 325.984)。
→ 形状と物質はリファレンスと同じ。

### HAs.pos の違いの影響(見積もり)

- 2 つのファイルの違いは一様: 底面の HA はすべて z が +11.3 mm(20190528 の方が深い)、側面の HA はすべて外向きに +6.8 mm。向きは同じ。
- シミュレーション(`HAPositionManager::GetPixPositionByID`)では、この位置を **GSO のピクセル(底面 26 mm 厚、側面 13 mm 厚)の中心**として置く。
- 20190403 で作った raw を 20190528 で再構成すると、吸収点が一様にずれる。TPC 内の一様な散乱点と全ピクセルで見積もると、
  散乱ガンマの方向のずれは平均 1.4°(底面 1.9°、側面 1.1°)、90% 点 2.5°、散乱点が HA に近いと最大 ~10°
  (`scratchpad/hapos_effect.py`)。ランダムではなく系統的なずれなので、ARM の中心と幅、α のカット(幾何と運動学の散乱角の整合)の効率に効く。エネルギーには効かない。
- 2026-10-13 決定(小野田): **今のまま、ずれたままでよい**。シミュレーションは 20190403(リファレンスと同じ)、再構成は実データと同じ 20190528。
  ARM などを比べるときはこのずれ(上の見積もり)を念頭に置く。

## 2026-10-13 Gd の中性子捕獲(GdPhysics 1)の調査

- リファレンス: `haboc/src/haboc_g4/` の `SMILE_Gd_PhysicsList`(= `SMILE_PhysicsList` + `SKNeutronCapturePhysics`)。
  `SKNeutronCapturePhysics` が中性子の `nCapture` を外して、`GdParticleHPCapture`(0–20 MeV)+ `G4NeutronRadCapture`(>20 MeV)を登録する。
- `GdParticleHPCapture` は Geant4 10 の `G4ParticleHPCapture` の写しで、Z = 64 の元素だけ final state を `GdParticleHPCaptureFS` に差し替える。
  FS はガンマを ENDF ではなく `GdCaptureGammas_ggarnet`(ANNRI-Gd 系、K. Zbiri 版を改造)で作る(Gd-155 / Gd-157 を断面積×存在比で選ぶ、離散カスケード + 連続成分)。
- GSO(Gd2SiO5)に Gd が入っているので、熱中性子のバックグラウンドには必須。Geant4 11.4 に ANNRI-Gd は入っていない → 移植する。
- Geant4 11 での変更点: `G4HadronCaptureProcess` → `G4NeutronCaptureProcess`、`G4ParticleHPCapture` → `G4NeutronHPCapture`、
  `Init` の引数が const 参照、`G4FindDataDir`。捕獲チャンネルの表は `G4ParticleHPManager` で共有されるので、標準の捕獲が先に作ると Gd の FS が使われない → Gd 用に別の表を持たせる。
- **連続成分の表**(`kopecky155_int`, `kopecky157_int`, 200×200)を **ハードコードのパス**
  `/home/nis_home/ikeda_sim/BG_study/G4/thermal_neutron_ANNRI_test/cont_dat/Gd155.dat`, `Gd157_org.dat` から読む。
  ファイルが無くてもエラーにならず、表が 0 のままだと連続成分のループが終わらない。
  → 同じファイルが `hikaru3_analysis2/Gd155.dat`, `Gd157_org.dat` にあった(小野田)。`~/smile2p_sim_data/Gd_capture/` にコピー(md5 一致、各 200×200 = 40000 個、値は 0〜1)。

### 移植(`simulation_source/vendor/gd_capture/`)

- `GdCaptureGammas_ggarnet`: そのまま。表のパスを `SetDataFiles()` で渡す形にし、開けない・数が足りないときは `FatalException`。
- `GdParticleHPCaptureFS`: Geant4 11 の API(`Init` の const 参照、`G4Cache` の `theResult`、整数の `theBaseA/Z`)に合わせた。
  物理は同じ(リファレンスの癖もそのまま: ガンマは同位体によらず Gd-155/157 を断面積×存在比で選ぶ、ガンマが 1 本のときは反跳核を 2 つ出す)。
  スレッドで共有されるので、`nucleus`・`theTwo`・`targetMass` の書き換えをローカル変数にした。一時的な `G4Fragment` のリークも直した。
- `GdParticleHPCapture`: Geant4 11.4 の `G4NeutronHPCapture` と同じで、Z = 64 だけ `GdParticleHPCaptureFS`。
  チャンネルの表は `G4ParticleHPManager` と共有せず、静的メンバで持つ(マスターで作り、ワーカーは読むだけ)。
  `GetFatalEnergyCheckLevels` はリファレンスと同じ (10 %, DBL_MAX)。
- `GdNeutronCapturePhysics`(`SKNeutronCapturePhysics`): `nCapture` を外し、`G4NeutronCaptureProcess` に上の模型(0–20 MeV)と `G4NeutronRadCapture`(>20 MeV)、
  断面積 `G4NeutronCaptureXS` + `G4ParticleHPCaptureData` を登録。
- `SMILEPhysicsList` のパラメータ `gd_physics`, `gd155_table`, `gd157_table`。リファレンスと同じく FTFP_BERT_HP の直後に登録。

### 確認(`simulation_source/test/run_gd_capture_test.py`)

1 m の GSO の立方体の中心から熱中性子(25 meV)を 2000 個。シンチのエネルギー損失の合計(≒ 捕獲ガンマの合計):

| | 7.80–7.96 MeV | 8.40–8.56 MeV | 平均 | 主なピーク |
| --- | --- | --- | --- | --- |
| `gd_physics` あり | 71 % | 17 % | 7.98 MeV | 7.87, 8.45 MeV |
| なし(Geant4 標準) | 15 % | 15 % | 7.03 MeV | 6.35, 7.93, 8.53 MeV |

- あり: ggarnet の合計エネルギー(Gd-157 7.87、Gd-155 8.46 MeV)にピーク。比 71:17 ≒ 81:19 はコードの断面積×存在比(39.751/48.764 = 81.5 %)と合う。
  平均 7.98 MeV ≒ 0.815×7.87 + 0.185×8.46 = 7.98 → ほぼ全部吸収されている。
- なし: Geant4 標準(ENDF)では Q 値(7.94, 8.54)のピークに加えて 6.35 MeV にピークがあり、ガンマの出方がかなり違う。
  → 熱中性子のバックグラウンドでは Gd の模型の有無が効く。
- `.gitignore` の `simulation_source/test/*.gdml` が手書きの試験用 GDML まで無視していたので、`smoke_test.gdml`, `gd_capture_test.gdml` を例外にした。

## 2026-10-13 宇宙線の一次粒子(SMILE_2p_PA_CR → ParmaCRPrimaryGen)

- リファレンス: `Erica/SMILE_2p/SMILE_2p_PA_CR.{h,cxx}`(ビルドされているのは `Erica/SMILE_2p/`)。
  PARMA の ROOT ファイルの TH2D `h2d<id>`(x: E [MeV]、y: cosθ、中身 /cm²/s/sr/MeV。id: neutron 0, proton 1, e- 31, e+ 32, gamma 33)に
  dE × dcosθ × 2π を掛けた分布から (E, cosθ) を `TH2::GetRandom2` で選び、方向の天頂角 = acos(cosθ)、方位角は一様。
  出発点は方向に垂直な一辺 `ParaSrcSize` の正方形の上で一様、原点から `ParaSrcDistance` だけ上流。
- リファレンスは毎イベント `gRandom->SetSeed(CLHEP の種)` していた(種は毎イベント時刻から作るので実害なし)。
  EricaNext では同じアルゴリズム(累積和で bin を選び、bin の中で一様)を Geant4 の乱数で行う → anlgeant4 の種で再現できる。
- `simulation_source/modules/ParmaCRPrimaryGen/ParmaCRPrimaryGen.cc`(`anlgeant4::BasicPrimaryGen`)。パラメータ `particle`, `flux_file`, `flux_histogram`(空なら粒子から)、
  `plane_size`, `plane_distance`(既定はリファレンスのコードの既定 3 m / 1.5 m。リファレンスの run は sim.mac で 3000 / 3000 mm)。
  run の終わりに等価な観測時間 N / (面積 × 全フラックス) を表示する(d10 熱中性子: 全フラックス 0.2048 /cm²/s、1M 個で 54.3 s)。
- `EdepPickUp` に `ha_off_file`: リファレンスの `SMILE_2p::DepositInPSA` と同じく、`HA_OFF.dat` の PMT(8 個)のシンチのエネルギー損失を捨てる
  (`HA_OnOff[HA][Get_PMT_ID(OFF_PMT-1, ID_TO_NAME)-1]`、体積名 `scinti_<bottom|side>_<HA>_<PMT>_<pixel>` の PMT 名と比べる)。
- リファレンスのトリガ(`GetETCCTriger`): anti にエネルギー損失なし && シンチにあり && TPC にあり(しきい値なし)。
  `ETCCcount.scl` の `Triggered`。`DAQ_Mode 3` では TPC && シンチ のイベントを `Collect_Number`(10000)個集めたら止まる → `EmuWrite2p/sim` の 10000 イベント。
- 実行: `simulation_source/test/run_smile2p_parma_cr.py <flux.root> [n] [--seed] [-o]`(sim.mac と同じ設定: flight、Gd、2 気圧、20190403 の HAs.pos、HA_OFF)。
  1 プロセス 1.1 GB、初期化 20 秒、約 2000 イベント/秒(d10 熱中性子)。
- 形状の警告(`flange_support` と `bCALET` の 12 µm の重なりなど)はリファレンスの `erica.log` にも同じものがある(`.mqo` 由来)。

### 確認 1: 一次粒子だけ(TowardTPC)

空の世界で 1M 個を生成し、リファレンスの `G4ETCCbase::TowardTPC`(値も癖もそのまま: uPIC 30.72 cm、z の面は `tpc_offset/10` = 0.3 mm と 330 mm、
x・y の面は z が 9〜309 mm)を numpy で計算: **1.631 ± 0.013 %**(リファレンス d10/v1: 710755 / 43317582 = 1.641 %、差は 0.7σ)。

### 確認 2: リファレンスの run と同じ設定(8 × 1M イベント)

`run_smile2p_parma_cr.py`(seed 1〜8、各 1M、8 並列で約 10 分)。リファレンスは d10 の v1〜v20 全部(生成 866,517,717、集めたもの 200,000)。

| | リファレンス(/生成) | EricaNext(/生成) | 比 |
| --- | --- | --- | --- |
| TPC && シンチ | 2.308e-4 | 2.285e-4 (1828) | 0.990 ± 0.023 |
| トリガ(+ anti なし) | 1.757e-4 | 1.753e-4 (1402) | 0.997 ± 0.027 |
| TPC && シンチ のうち anti あり | 23.9 % | 23.3 % | |

- TPC・シンチのエネルギー損失の分布: KS p = 0.15(TPC)、0.85(シンチ)。中央値 73.5 / 75.5 keV(TPC)、2036 / 2091 keV(シンチ)。
- 一次粒子のエネルギー(集めたイベント): KS p = 0.004。エネルギー帯ごとの比(EricaNext / リファレンス、/生成):
  1–10 meV 0.48 ± 0.28、10–30 meV 0.57 ± 0.18、30–100 meV 0.97 ± 0.13、0.1–1 eV 0.87–0.92 ± 0.08、1 eV–20 keV 0.95–1.05 ± 0.05。
  → 合計は合うが、30 meV 以下(熱中性子の中でも低い側)で少なめの傾向(2σ 程度)。
  生成の段階では合っている(1 keV 以上の割合 28.10 % / 28.07 %、CDF の差 0.0014(`InitInc_E` と比較)、6 meV 以下 0.092 % / 0.090 %(ヒストグラム))
  ので、輸送(Geant4 10 → 11.4、G4NDL のバージョン、熱中性子の散乱の扱い)の違いの可能性。今の統計では断定できない。
- 入力の PARMA のフラックスが同じことの確認: リファレンスが実際に使ったヒストグラム(各 `v*/G4init_para.root` の `h2d0`、`SMILE_2p::Exit` が書き出したもの)と、
  EricaNext で読んだ `hikaru3_analysis2/BGsim/thermal_neutron_g10/parma_flux/parma_flux_d10_g10_thermal.root` の `h2d0` を比べると、
  v1〜v20 のすべてで軸(bin の端)と中身(under/overflow も含む)が完全に一致。

### 確認 3: 統計を 4 倍(32M)にして、ずれの原因を探る(2026-10-13)

- 一次粒子のエネルギー(集めたイベント): 32M では χ² = 4.1/9(p = 0.91)、30 meV 以下 81 / 予想 94.8(p = 0.08)、KS p = 0.08
  → 8M で見えた低エネルギー側のずれは **統計のゆらぎ** だった。率も 0.994 ± 0.012(TPC && シンチ)、0.998 ± 0.013(トリガ)。
- 一方で **TPC のエネルギー損失の分布** が KS p = 1e-5 でずれる(500 keV 以上が 1.63 倍、200–500 keV も 1.1 倍)。
- 原因を見るため `EdepPickUp` に `tpc_pdg`(粒子)と `tpc_creator`(トラックを作った過程)を追加。同じ seed で同じイベントが再現する(seed 1 で確認)。
  - TPC のエネルギー損失はほとんどがコンプトン電子。500 keV 以上の 28 イベント中 6 イベントは **³⁶Ar(n,α)³³S の α**(`neutronInelastic`、残留核はすべて ³³S)。
- バージョンの違い(リファレンスの `erica.log` より):

  | | リファレンス | EricaNext |
  | --- | --- | --- |
  | Geant4 | 10.4.2 | 11.4.3 |
  | 中性子データ | G4NDL4.5(主に ENDF/B-VII.1) | G4NDL4.7.1(JEFF-3.3。**Ar-36, Ar-38 のファイルを削除**、熱散乱を作り直し) |
  | e± の多重散乱 | UrbanMsc、RangeFactor 0.02、DistanceToBoundary、skin 1、e± の finalRange 0.1 mm | GoudsmitSaunderson、0.08、SafetyPlus、skin 3、10 µm |

- 切り分けの道具:
  - G4NDL4.5 を `~/opt/geant4-data-extra/G4NDL4.5` に置き、`G4NEUTRONHPDATA` で切り替え(存在しないパスにすると落ちるので、環境変数が効くことを確認)。
  - `SMILEPhysicsList` の `em_msc`: `gs`(既定、Geant4 11 の `G4EmLivermorePhysics`)/ `urban`(`SMILEEmLivermoreUrbanPhysics`:
    11.4 の `G4EmLivermorePhysics` の写しで、e± の多重散乱だけリファレンスと同じにしたもの。verbose 1 の表示で UrbanMsc / DistanceToBoundary / 0.02 / skin 1 を確認)。
    `run_smile2p_parma_cr.py --em-msc urban`。
- 8M ずつの比較(TPC のエネルギー損失の帯ごとの比 EricaNext/リファレンス、帯は 0–1, 1–10, 10–30, 30–100, 100–200, 200–300, 300–500, 500– keV):

  | 設定 | 比 | ³⁶Ar の α のイベント | χ² p | KS p |
  | --- | --- | --- | --- | --- |
  | G4NDL4.7.1 + GS(今の既定) | 0.93 1.05 1.01 0.98 0.93 1.11 1.00 2.03 | 20 | 0.009 | 0.15 |
  | G4NDL4.5 + GS | 0.96 0.96 0.88 0.93 1.05 1.27 1.06 0.51 | **0** | 0.003 | 0.002 |
  | G4NDL4.5 + Urban(リファレンス相当) | 1.11 0.80 0.94 0.95 1.05 1.12 1.03 0.65 | 0 | 0.05 | 0.03 |

  → ³⁶Ar(n,α) の α は G4NDL4.7.1 でだけ出る(Ar-36 のファイルが無くなったことによる)。500 keV 以上の超過の一部はこれ。
  残りの形の違いは 8M では判断しきれないので、G4NDL4.5 + Urban を 32M にして比較する。

### ³⁶Ar(n,α) の正体: G4NDL4.7.1 で Ar-36 が Ar-37 のデータで扱われる(2026-10-13)

- G4NDL4.7.1 の Ar のファイル: `18_37`, `18_39`, `18_40`, `18_41` だけ(README: 「Ar-36, Ar-38 のファイルを全部削除(ENDF/B-VIII.0 と大きく違うため)」)。
  G4NDL4.5 は `18_36`, `18_38`, `18_40`。
- ファイルが無い同位体は、Geant4 の HP が近い同位体のデータで代用する。Ar-37(放射性)の (n,α) の断面積は 10 µeV で 1.0×10⁵ b(1/v なので熱中性子で約 2000 b)。
- 直接の確認(`scratchpad/ar36_test.py`): Ar-36 だけのガス(3.3 mg/cm³、一辺 2 m)の中心から 25 meV の中性子を 20000 個。
  - G4NDL4.5: α を出すイベント 0。³⁶Ar(n,γ)³⁷Ar が 800 イベント(約 4 %、実際の捕獲断面積 約 5 b から予想される程度)。
  - G4NDL4.7.1: **19305 / 20000** で α(残留核 ³³S)、ほかに (n,p)³⁶Cl。ほぼ全部の中性子が 10 cm 程度で吸収される。
- 実際の ³⁶Ar(n,α) の熱中性子の断面積は mb 程度。G4NDL4.7.1 だと天然 Ar(Ar-36 0.334 %)で 1 原子あたり約 0.00334 × 2000 ≈ **6.7 b の偽の (n,α)**
  (Ar-40 の捕獲 0.66 b の約 10 倍)→ **Ar ガスの TPC にとっては誤った物理**。熱中性子のバックグラウンド(α のイベント、ガスでの中性子の吸収)に効く。
- 対処(小野田が選択): **Ar-36 / Ar-38 のファイルだけ G4NDL4.5 から補う**。
  `scripts/sim_env/make_g4ndl_ar36fix.sh` が `~/opt/geant4-data-extra/G4NDL4.7.1_Ar36fix` を作る(G4NDL4.7.1 のハードリンクの写し + G4NDL4.5 の Ar-36/38 の 30 ファイル)。
  `setup_sim.sh` で `G4NEUTRONHPDATA` をそこに向ける(既定)。同じ Ar-36 の試験で G4NDL4.5 と同じ結果(α 0、捕獲 800 / 20000)。

### 確認 4: リファレンス相当(G4NDL4.5 + Urban)を 32M

| 設定(32M) | 率の比 | TPC のエネルギー損失の帯ごとの比 | χ² p | KS p |
| --- | --- | --- | --- | --- |
| G4NDL4.7.1 + GS | 0.994 ± 0.012 | 0.91 0.96 1.00 0.97 1.00 1.08 1.13 1.63 | 1e-6 | 1e-5 |
| G4NDL4.5 + Urban | 0.999 ± 0.012 | 1.08 0.90 0.94 0.98 1.01 1.02 1.05 0.93 | 0.03 | 0.23 |

→ 中性子データ(Ar-36)と e± の多重散乱をリファレンスに合わせると、30 keV 以上の TPC のエネルギー損失はリファレンスと合う。
残りは 10 keV 以下の小さいエネルギー損失(0–1 keV 1.08、1–10 keV 0.90)。低エネルギーの EM(G4EMLOW のバージョン、Livermore のモデルの更新、
`LowestElectronEnergy` など)の違いと思われる。

## 2026-10-13 設定ファイル(config/simulation)と、カットの表の単位のバグ

- 検出器・物理モデル・線源・乱数・記録を 1 つの JSON にまとめる形にした(小野田の希望: モデルの選択も検出器の設定と一緒に config から)。
  `config/simulation/*.json` + `run_script/run_sim.py`(`ericanext_config.load_sim()`、`{sim_data}`, `{home}` を置き換え)。
  節: `environment`(`G4NEUTRONHPDATA` など。Geant4 の初期化の前に設定)、`detector`, `physics`, `source`, `geant4`, `pickup`。
  `--set 節.キー=値` で上書き。説明は `config/README.md`(当時は `config/simulation/README.md`)。
- **バグ**: `SMILEPhysicsList` のコンストラクタの既定値 `lowEnergyLimit_(0.25)` に keV を掛けていなかった
  → Geant4 の内部単位(MeV)で 0.25 MeV = **250 keV** になっていた(リファレンスは 250 eV)。
  config から値を渡すと単位が掛かるので正しく 0.25 keV になり、古い試験スクリプトと結果が違ったことで発覚。
  `cutValue_(0.001 * mm), lowEnergyLimit_(0.25 * keV)` に修正。修正後、`run_sim.py` と `run_smile2p_parma_cr.py` で同じ seed のイベントが完全に一致。
- 影響: これまでの run(確認 2〜4、Ar-36 の試験以外)はすべて カットの表の下限 250 keV で、250 keV より下の二次粒子(δ線、制動放射)を
  別の粒子として作らずにその場に落としていた。→ 確認 3, 4 の TPC のエネルギー損失の比較をやり直す(A: 既定 = Ar36fix + GS、B: G4NDL4.5 + Urban、各 32M)。
  Gd の試験(GSO の立方体)と Ar-36 の試験は、エネルギーの合計・反応の種類を見ているので結論は変わらない。

### 確認 5: カットの表を直したあとのやり直し(各 32M、2026-10-13)

`run_script/run_sim.py smile2p_flight_parma_thermal_neutron_d10`(カットの表の下限 250 eV、リファレンスと同じ)。

| 設定 | 率の比(TPC && シンチ / トリガ) | TPC のエネルギー損失の帯ごとの比 | TPC: χ² p / KS p | シンチ KS p | 一次粒子 KS p |
| --- | --- | --- | --- | --- | --- |
| A: G4NDL4.7.1_Ar36fix + GS(既定) | 1.018 ± 0.012 / 1.007 ± 0.013 | 1.00 1.05 1.05 1.03 1.02 1.00 0.94 0.69 | 0.13 / 0.36 | 0.83 | 0.10 |
| B: G4NDL4.5 + Urban(リファレンス相当) | 0.979 ± 0.012 / 0.974 ± 0.013 | 0.97 1.05 0.91 0.99 0.98 0.97 1.00 1.07 | 0.25 / 0.56 | 0.65 | 0.24 |

- **どちらもリファレンスと統計的に一致。** 確認 3, 4 で見えた TPC のエネルギー損失の高い側の超過は、
  **カットの表の単位のバグ(250 keV)と ³⁶Ar の偽の α** によるもので、e± の多重散乱のモデル(GS / Urban)の差ではなかった
  (確認 4 の「Urban にすると合う」は、バグがある状態での比較だったので取り消し)。
- A と B の率の差 3.9 ± 1.7 %(2.3σ)は中性子のデータ(JEFF-3.3 / ENDF/B-VII.1)の差の可能性。どちらもリファレンスから 1.5σ 程度。
- → 既定は **Ar-36 を補った G4NDL4.7.1 + Geant4 11 の GS** のままにする。

## 2026-10-14 検出器の応答と raw ファイル(TPC_emulator, Emu_HA, EmuWrite2p)の移植

SMILE_tool_box_v2.20 の 3 モジュール(ローカルの写し `~/SMILE_tool_box_v2.20/Erica/`)を、EricaSim の ANLNext モジュールとして移植。
チェーンは `... → EdepPickUp → SimTrigger → TPCEmulator → HAEmulator → RawWriter2p`(config の新しい節 `response`、`run_sim.py --raw-dir`)。

| SMILE_tool_box | EricaSim | 中身 |
| --- | --- | --- |
| SMILE_2p の DAQ_Mode / Collect_Number | `SimTrigger` | 1 TPC、2 シンチ、3 TPC && シンチ、4 + anti なし、5 TPC だけ、6 シンチだけ。満たさないイベントは AS_SKIP。規定数に達したら次のイベントの頭で終了 |
| TPC_emulator(ver 0.41) | `TPCEmulator` | 電離(cluster.txt の分布 / Poisson × Fano)、拡散、FiducialModify の解析的な関数での位置と時刻、GEM の隙間、GEM(幾何分布)× µ-PIC(Polya、ゲインマップから)、岩城の応答のテンプレート、ストリップ × クロックの地図 → ボードごとのヒットの記録と 4 × 512 の FADC |
| Emu_HA | `HAEmulator` | 分解能(底面 11.5 %、側面 10.5 %)、周りへのもれ、電荷分割(charge.div)、PMT の一様性(PMT_Map、無ければ 1)、3.1 ch/keV、ペデスタル 0.8、閾値 33 / 4095 → 0x2000。`Cal_HA*.dat` も書く |
| EmuWrite2p | `RawWriter2p` | `TPC_FileManager`, `HA_FileManager` をそのまま使って raw を書く。真の値の `sim` ツリー(`sim_truth.root`) |

- 入力: SMILE_tool_box はバンク(`TPC_Hit_E/X/Y/Z` = Geant4 の各ステップのエネルギー損失 [keV] と位置 [mm]、`ScintiHit_*` = ピクセルごとの合計)。
  EricaNext は `EdepPickUp` から同じもの(シンチはピクセルの体積の名前ごとに、最初のエネルギー損失の順で合計)。
- そのまま残したリファレンスの癖: FADC の積分が 33 ストリップ分(隣の ASIC の 1 本を含む)、FiducialModify::RealToDetector_3D でカソードの時刻にもアノードの座標を使う、
  `fidmod.Init()` を呼ばないので fiducial_modify.root は使われず解析的な関数、ゲインマップの読み込みの eof の扱い、
  TPC の時刻は実行の経過時間(壁時計)× 100、`r_gem` は引くが使わない(乱数の順番を保つため残した)。
- 違い: 乱数は random_seed から作る TRandom3(リファレンスは既定の種 4357 の TRandom3 と gRandom)→ 統計的に同じでビット単位では一致しない。診断用のヒストグラムは作らない。
- vendor: `tpc_emu/`(CalcCluster: 種の指定と SetDirectory(nullptr) のみ追加、FiducialModify: そのまま)、`emu_write/`(TPC_FileManager, HA_FileManager: そのまま)。
- リファレンスの run のパラメータ(HabocStatus.scl)で `config/simulation/smile2p_flight_parma_thermal_neutron_d10_raw.json` を作成。
  ゲインマップは参照の `parameters/20190124/Calibration20190130.dat` と同じ名前の `param/20190528/Calibration20190130.dat`。
- 確認: 20 万イベントで 55 イベントを書き出し、EricaNext の ReadData でそのまま読めた。リファレンスの raw(d10/v1、10000 イベント)も ReadData で読めた。
  ReadData → RawTreeWriter の生データツリーで比べると(`scratchpad/compare_simraw.py`)、55 イベントでも
  ヒットの記録の数、ストリップのヒットの数、FADC の電荷、HA の ADC の数と合計の KS p = 0.16〜0.85。
- 8 × 1M(seed 1〜8)で 1927 イベント(リファレンスの d10/v1 は 10000):

  | 量(1 イベントあたり) | 中央値 EricaNext / リファレンス | KS p |
  | --- | --- | --- |
  | アノードのヒットの記録 | 82 / 95 | 0.079 |
  | カソードのヒットの記録 | 80 / 94 | 0.089 |
  | アノードのストリップのヒット | 613 / 655 | 0.16 |
  | カソードのストリップのヒット | 605 / 636 | 0.056 |
  | アノードの FADC の電荷(閾値 20) [pC] | 1777 / 1734 | 0.21 |
  | カソードの FADC の電荷 [pC] | 1731 / 1735 | 0.18 |
  | HA の有効な ADC の数 | 8 / 8 | 0.76 |
  | HA の ADC の合計 | 4355 / 4398 | 0.95 |

  ヒットのクロックの分布の 0.5 % / 99.5 % 点: 103 / 905(リファレンス 103 / 903)。分布の形もすべて重なる → 検出器の応答と raw の書き出しはリファレンスと同等。

## 2026-10-14 シミュレーションの raw の再構成(EricaNext のチェーン)

- リファレンスの再構成の設定(`BGsim/thermal_neutron_g10/reconstruct_skew/d10/v1/HabocStatus.scl`): Cal_Modify 1.27512、
  HA_Center_Gravity の閾値 33、HA_Calc_s2 `Emu_Mode 1`・`Parameter_Data_Dir parameters/emulator`・`Flag_Modify_PMT_Gain 0`、
  シミュレーション用の別の drift_velocity.dat(`/home/nis_home/ikeda_sim/simulation4/CR_BG_data4/drift_velocity.dat`、手元に無い)。
- `Emu_Mode`(SMILE_tool_box `HA_Emu_ID_Modification`): エミュレータの PMT / ピクセルの番号を位置の計算の前に付け直す
  (PMT 0 → 1 で x, y 反転、1 → 5 で反転、3 は反転だけ、5 → 0)。エネルギーの較正は元の番号。EricaNext の HA_Calc に `Emu_Mode`(既定 0)として追加。
- パラメータのフォルダ `~/smile2p_sim_data/emulator_param/`(README.txt):
  XMDF/YMDF_HA*.dat = `XMDF.dat` / `YMDF.dat`(エミュレータの電荷分割用の MDF、全 HA 共通と推定)、Cal_HA*.dat = HAEmulator の出力(run によらず同じ)、
  HAs.pos = param/20190528(データと同じ)、drift_velocity_sim.dat = シミュレーションのヒットのクロックから(offset 93、0.3770 mm/clock。
  端は微分の最大の ±20 クロックでガウス関数をフィット。この方法を Per02 に当てると DriftVelocity.ipynb の 10 分ごとのフィットより 0.7 % 速い)。
- 設定: `config/smile2p_sim_d10_recon.json`(`--set ReadData.data_dir=<raw>`)。`ericanext_config.load()` でも `{sim_data}` を使えるようにした。
- 結果(1927 イベント、リファレンスは d10 の 20 run = 200000 イベント、`Reconstruct/s2t`):

  | | EricaNext | リファレンス | KS p |
  | --- | --- | --- | --- |
  | err_flag == 0 の割合 | 21.0 %(404) | 19.9 % | |
  | Eg の中央値 [keV] | 950 | 923 | 0.49 |
  | Ke(Ee)の中央値 [keV] | 74.3 | 74.3 | 0.32 |
  | Eg + Ke | 1050 | 1030 | 0.84 |
  | Len_TRA [mm] | 137 | 152 | **0.029** |
  | Pe_TRA の z [mm] | 212 | 220 | **0.0075** |

  エネルギーは一致。z に関わる量がずれる: Pe_TRA の z の分布のドリフト面側の山がリファレンス 330〜340 mm、EricaNext 320〜330 mm。
  → ドリフト速度のファイル(offset と速度)がリファレンスと違うため。
- 小野田がリファレンスの再構成のファイルを `hikaru3_analysis2/par_for_sim_reconstruct/` に置いた: drift_velocity.dat は
  **offset 85、0.38 mm/clock で一定**(エミュレータの入力の `DriftVel 38`、`Offset_for_ETCC 85` そのもの)、Calibration20190130.dat は param/20190528 と同じ、
  setting.mac は上の設定と同じ。→ `emulator_param/drift_velocity.dat` にコピーし、設定をこれに変更。
- やり直し(同じ 1927 イベント): Pe_TRA の z は 216 / 220 mm、KS p = **0.099**(一致)。Eg, Ke は変わらず一致。
  Len_TRA は 137 / 152 mm、p = 0.030 のまま(404 行、5 つの量を比べた中での p なので判断できない)。
- 8 × 4M(seed 11〜18、7428 イベント): 再構成は Len_TRA 152.2 / 152.1 mm(p = 0.08)、Pe_TRA の z p = 0.82、err_flag == 0 の割合 19.3 / 19.9 % で一致
  (Len_TRA の差は統計のゆらぎだった)。ただし raw の段階で TPC の信号が大きい: 全 9355 イベント対リファレンス 5 run(50000 イベント)で
  FADC の電荷の中央値 ×1.11、ストリップのヒット ×1.10(KS p ≈ 1e-9)。ヒットの記録の数(時間の長さ)は同じ。
- 原因の切り分け: TPC の真のエネルギー損失は同じ(中央値 73.2 / 73.6 keV)、電荷 ÷ エネルギー損失が 5〜13 % 大きい。
  移植したコードは hikaru3 側の SMILE_tool_box_v2.20(リファレンスの run で動いた版)と全ファイル同じ。ポリヤ分布の乱数の平均は 1001(理論 1000)。
  IwakiTemp_mod.root は IwakiTemp.root のちょうど半分の大きさ(原因ではない)。
- **原因: Geant4 のステップの細かさ。** TPCEmulator はステップごとに CalcCluster で種電子の数を決める(15 eV 未満のステップは必ず 1 個など)ので、
  ステップが細かいほど 1 keV あたりの電子が増える。e± のステップ(eIoni の finalRange、msc)を Geant4 10.4 と同じにした `em_msc = urban` で 8 × 1M:

  | 設定 | TPC のステップ / keV(中央値) | 1 ステップのエネルギー(中央値) | FADC の電荷の比(対リファレンス) | KS p |
  | --- | --- | --- | --- | --- |
  | gs(Geant4 11 の既定) | 14.3 | 21.5 eV | 1.113 | 2e-9 |
  | urban(リファレンス相当) | 9.3 | 47.9 eV | 0.95 | 0.41 |

  → urban でリファレンスと一致。エミュレータの電離の計算が Geant4 のステップの細かさに依存しているのが根本の問題
  (どちらの物理設定が正しいかではなく、エミュレータの結果が Geant4 の版・設定で 1 割変わる)。
- 2026-10-14 決定(小野田): **このまま(既定は gs)でよい。** 細かいステップで 1 割多いのも物理的におかしくはない。
  リファレンスと合わせたいときは `--set physics.em_msc=urban`。

## 2026-10-15 simulation のモジュールの置き場所、終了コード

- 小野田の希望で、`simulation_source/src`, `simulation_source/include` を `simulation_source/modules/<モジュール名>/`(データ解析の `source/modules` と同じ形)に変更
  (`git mv`)。CMakeLists.txt は `modules/*` のフォルダを集める。並べ替えの前後で同じ種の結果がバイト単位で一致
  (Geant4 の部分 187 イベント、真の値 55 イベント、HA の raw 18 ファイル)。
- `anlnext.AnalysisChain.run()` は初期化に失敗しても何も返さないので、`run_script/anl_run.py` の `run_checked()`(同じ手順で状態を確かめる)
  を `run_chain.py` と `run_sim.py` で使い、失敗したら終了コード 1。データの終わり(ReadData の AS_QUIT_ALL)と -n で止めた場合は 0 のまま。
- 2026-10-14〜15、`hikaru3_analysis2` のマウントが NFS から SMB に変わった(小野田)。切り替えの間、ファイルのアクセス権が `----------` に見えたり、
  読んでいたプロセス(Ba-133 の Per03, 04, 06)が終了処理なしで止まったりした。流し直し中。

## 次にやること

1. ~~SMILE の物理リスト~~(済み。Gd の中性子捕獲は未。Geant4 11 で消えた `G4HadronCaptureProcess` を使っているので書き直しが要る)
2. ~~`.mqo` から SMILE-2+ の形状を組み立て、GDML に書き出すモジュール~~(済み。上の要確認 2 点が残る)
3. ~~エネルギー損失を記録するモジュール~~(済み。リファレンスの真の値(コンプトン散乱の情報など)の記録は未)
4. ~~Cs-137 の点線源を SMILE-2+ の形状で動かす~~(済み)
5. リファレンスのシミュレーションとの比較(同じ設定で SMILE_tool_box を回した結果が必要)
6. TPC_emulator / Emu_HA / EmuWrite2p(検出器の応答と raw ファイルの書き出し)

## 2026-10-14 フォルダ名、シミュレーションの DataIO チュートリアル

- `simulation/` を `simulation_source/` に改名(`git mv`、build と install は作り直し)。`run_sim.py` の import の場所と `.gitignore` も変更。
- `tutorials_simulation/1_DataIO/DataIO_example.ipynb` を追加(既存の準備中のフォルダは 2_〜5_ に番号を付け直し)。
  d10 の熱中性子を 100 万個(seed 1): エネルギーを落としたイベント 9193、トリガー(daq_mode 3)270、
  再構成の err_flag == 0 は 48(17.8 %)。出力は `tutorials_simulation/cache/`(リポジトリに入れない)。
- 再構成用のパラメータ `{sim_data}/emulator_param` をノートブックの中で作る手順にした(XMDF/YMDF の写し、HAEmulator の Cal_HA、
  データの HAs.pos、TPCEmulator の DriftVel と Offset_for_ETCC から drift_velocity.dat)。既存のものとファイルの中身が一致。
- 観察: err_flag == 0 のイベントで `Ke / E_tpc` の中央値が約 0.8。エネルギーはすべて x, y が ±150 mm の中(読み出し範囲の外へのもれではない)。
  再構成の `Ke` の分布は参照と一致するので、SMILE_tool_box と同じ振る舞いとしてこのまま使う(小野田の判断)。

## SMILE-3 に向けた改善事項

SMILE_tool_box と同じ振る舞いなので今は直さないが、SMILE-3 では見直すべき点。

| 項目 | 中身 | 見つけた日 |
| --- | --- | --- |
| 再構成の `Ke` が真の値より小さい | シミュレーションの raw を再構成すると `Ke / E_tpc` の中央値が約 0.8(d10 の熱中性子、err_flag == 0 の 48 イベント)。分解能による広がりではなく偏り。読み出し範囲の外へのもれではない。TPC の応答(電離、増幅、閾値)と較正(`TPC_Calibration`)のどこで小さくなるかを調べる | 2026-10-14 |

## 2026-10-15 データの置き場所を NAS にまとめた

- 小野田が EricaNext_try に要るデータを NAS の `/mnt/nas_eureka/onoda/EricaNext_try_data/SMILE2+` にまとめた
  (`calibration_data/sm2+_cal_rawdata/20190701_Per0x`, `calibration_data/param/20190528`, `TPC_caldata/`, `simulation_data/parameters/smile2p_sim_data/`)。
- `config/ericanext_config.py`: 根元を `DATA_DIR`(NAS)、環境変数 `SMILE2P_DATA` で変えられるようにし、`{data_top}` = `<根元>/calibration_data`、
  `{tpc_caldata}` = `<根元>/TPC_caldata`、`{sim_data}` = `<根元>/simulation_data/parameters/smile2p_sim_data` にした。
  前の `~/smile2p_data`、`~/smile2p_sim_data`、研究室サーバーへの切り替えと、`SMILE2P_TPC_CALDATA`, `SMILE2P_SIM_DATA` はやめた
  (`setup_sim.sh` の `export SMILE2P_SIM_DATA` も削除)。
- 設定ファイルの生データの場所を `{data_top}/sm2+_cal_rawdata/20190701_Per02` などに変更。全設定ファイルのパスが NAS にあることを確認。
- 確認: NAS から Per02 の先頭 2000 イベントを再構成した `single_hit` は、前の出力と同じ(Eg, Ke, Len_TRA, err_flag)。
  NAS のデータで回したシミュレーション(seed 1、2 万個)の `edep` は前の出力と同じ。
- PARMA のフラックス(`source.flux_file`)は NAS に無いので、まだ `hikaru3_analysis2/BGsim` を指している。
- ついでに見つけた問題: `EdepPickUp` の出力先のディレクトリが無いと、`edep.root` が作られないまま終了コード 0 で終わっていた。
  ディレクトリを作り、開けなければ `AS_QUIT_ERROR`(終了コード 1)にした。
- 小野田が PARMA のフラックス(熱中性子、深さ 3〜100 g/cm²)を NAS の `simulation_data/parma_flux/` に置いた(d10 は元と md5 一致)。
  置き換え文字 `{parma_flux}`(= `<根元>/simulation_data/parma_flux`)を足し、`source.flux_file` を `{parma_flux}/parma_flux_d10_g10_thermal.root` にした。
  NAS のフラックスで回したシミュレーション(seed 1、2 万個)の `edep` は前の出力と同じ。中性子(`neutron_g10`)のフラックスは NAS に無い。

## 2026-10-16 ANLNext を 1 つにまとめた

- 実データの EricaNext(`source/`)を `~/opt/anlnext-g4`(`ANLNEXT_USE_G4VECTOR=ON`)に対してビルドすると、Per02 の先頭 5000 イベントの `single_hit` は
  `/usr/local` の ANLNext のときと 28 列すべてビット単位で同じ(NaN もビットで比較)。→ ANLNext を 1 つにした(小野田の判断)。
- `scripts/sim_env/` → `scripts/env/`。`setup_sim.sh` → `setup_ericanext.sh`(実データとシミュレーションで共通、`~/opt` に置く)。
  `install_all.sh`(Geant4 → G4NDL の修正 → ANLNext と ComptonSoft → 環境 → EricaNext)と `build_ericanext.sh`(EricaNext の 2 つのビルド)を追加。
  `build_geant4.sh` はデータが無ければダウンロード、`build_anlnext_comptonsoft.sh` はソースが無ければ GitHub から取る(c22eb3e / bbe3486)。
- `~/.bashrc` の `/usr/local` の ANLNext の 4 行(`ANLNEXT_INSTALL`, `RUBYLIB`, `LD_LIBRARY_PATH`, `PYTHONPATH`)を `source ~/opt/setup_ericanext.sh` に置き換え。
  `~/opt/setup_sim.sh` は新しいファイルを読むだけの互換用に残した。`/usr/local` の ANLNext は消していない。
- ANLNext は `import anlnext` だけで `libG4clhep.so` を使い、RPATH に Geant4 の場所が無いので、実データの解析にも `setup_ericanext.sh` の環境が要る。
- 新しい環境で、実データ(Per02)とシミュレーション(seed 1、2 万個)が前と同じ出力。全チュートリアル(24 冊)を実行し直してすべて完了。
- 別件: 10 月 12 日に作ったチュートリアル用の `cache/Per02_50000.root` と今のビルドの出力が `Ve_TRA`, `Ve_TRA_fit_status`, `Ve_TRA_grad_err` で
  わずかに違う(grad_err の相対差 10⁻⁷ 程度、fit_status 4 → 0 が 6 行)。ANLNext とは無関係(`/usr/local` のビルドでも同じ)。原因は未調査。
- 同日: ほかの Geant4 / ANLNext を使っている人の環境を壊さない形にした(小野田と相談、仮想環境は使わない)。
  - `setup_ericanext.sh` は `~/.bashrc` に書かず使うときだけ読み込むのを標準にし、README に注意を書いた(小野田の環境は `~/.bashrc` で読み込むまま)。
    `G4NEUTRONHPDATA` は設定しない(シミュレーションの設定ファイルの `environment` で `run_sim.py` が設定。試験のスクリプト 2 つは自分で設定)。
  - Jupyter のカーネル「EricaNext」(`make_jupyter_kernel.sh`、kernel.json の env に環境変数)。環境変数の無いシェルから、このカーネルで
    DataIO(実データ・シミュレーション)と GainMap_Board が完了し、カーネルの環境だけで `run_chain.py` と `run_sim.py` が動くことを確認。
  - 既にある Geant4 を `GEANT4_INSTALL` で使えるようにした: `geant4_paths.sh`(場所の決定)、`check_geant4.sh`(GDML、TLS のモデル、物理データ、版)。
    `build_anlnext_comptonsoft.sh`, `make_g4ndl_ar36fix.sh`(その Geant4 の G4NDL を使う)、`setup_ericanext.sh` は `~/opt/geant4-11.4.3` の決め打ちをやめた。
    既にある Geant4 での実際の通しの試験はしていない(この環境には別の Geant4 が無い)。
