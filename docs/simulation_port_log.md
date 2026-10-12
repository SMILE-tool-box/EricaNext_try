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

## 2026-10-12 シミュレーション用パッケージの骨組み(`simulation/`)

実データ用(`source/`、`/usr/local` の ANLNext)とは使う ANLNext が違う(ベクトル有効)ので、別の CMake プロジェクトにした。

```
simulation/
├── CMakeLists.txt          libEricaSim.so と Python 拡張 EricaSimPy
├── include/, src/          モジュール
├── pyext/EricaSimPy.i
└── test/                   動作確認用(smoke_test.gdml, run_smoke_test.py)
```

ビルド: `source ~/opt/setup_sim.sh; cmake -S simulation -B simulation/build -DCMAKE_INSTALL_PREFIX=simulation/install; cmake --build simulation/build --target install`

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

## 次にやること

1. ~~SMILE の物理リスト~~(済み。Gd の中性子捕獲は未。Geant4 11 で消えた `G4HadronCaptureProcess` を使っているので書き直しが要る)
2. **`.mqo` から SMILE-2+ の形状を組み立て、GDML に書き出すモジュール**(SMLib `GGG_Model` の移植)
3. ~~エネルギー損失を記録するモジュール~~(済み。リファレンスの真の値(コンプトン散乱の情報など)の記録は未)
4. Cs-137 の点線源を SMILE-2+ の形状で動かす
