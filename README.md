# EricaNext_try

**SMILE-2+ データで EricaNext の再構成チェーンを動かすための作業リポジトリ**

[EricaNext](https://github.com/SMILE-tool-box/EricaNext) は、SMILE プロジェクトの検出器生データ
（detector raw data）から、天体解析が可能なイベント再構成（event reconstruction）までを
一貫して処理するパイプラインソフトウェアです。ANLnext フレームワークをベースに構築されており、
解析モジュールを組み合わせてデータ処理フローを構成します。

`EricaNext_try` は、その中でもまず **SMILE-2+ の生データで再構成が通る状態**を作ることを
目的に、モジュールを 1 つずつ移植・検証しているディレクトリです。

## モジュールの由来

`SMILEsoft`（SMILE-2+ 時代の実装）と `Erica_git_dev`（`feature-TOT-skewness` ブランチ、
主に SMILE-3 向けに更新が進んでいる実装）の 2 系統から、モジュールごとに
「SMILE-2+ の生データを正しく読めるか」を基準に選んで移植しています。

| モジュール | 由来 | 理由 |
| --- | --- | --- |
| `SaveFile` | Erica_git_dev | 差分なし |
| `ReadData` | **SMILEsoft** + Erica_git_dev の Hit vector interface を移植 | Erica_git_dev 版は raw ファイル名が SMILE-3 形式（`tpc_enc_a/c??_*.raw`）で SMILE-2+ 形式（`uTPC_anode/cathode*_*.raw`）を読めないため |
| `CalcFADC` | **SMILEsoft** | Erica_git_dev 版は物理ボード番号をそのまま配列添字に使っており、12ボード/4パネル固定を前提にしている（SMILEsoft 形式のボード番号と組み合わせると範囲外アクセスの恐れ） |
| `TPC_Calibration` | Erica_git_dev | ROOT の bin 番号 off-by-one 修正・象限（384 ストリップ単位）一致チェックが入っている。CalcFADC の位置ベース配列とも整合 |
| `TOT_Skewness` | Erica_git_dev | SMILEsoft 版は未実装（空スタブ）。Erica_git_dev 版が完全実装 |
| `HA_CenterGravity` / `HA_Calc` / `Reconstruct` | Erica_git_dev | 差分なし |

検証は `SMILEsoft/src/data/Per03_ETCC` の実データ（SMILE-2+ 形式）で行い、
`ReadData → CalcFADC → TPC_Calibration → TOT_Skewness → HA_CenterGravity → HA_Calc → Reconstruct`
の全チェーンが動作し、物理的に妥当な再構成結果（飛程・skewness・コンプトン散乱角など）が
出力されることを確認済みです。

## チュートリアル

`notebooks/01_smile2p_quickstart.ipynb` に、SMILE-2+ データで一通り解析チェーンを
動かして結果を可視化するチュートリアルがあります（Fermipy / cosipy のクイックスタート相当）。

> **注意（Jupyter 実行時）**: `CalcFADC` は波形積分を `std::thread` で行い、その中から
> ROOT のヒストグラムに `Fill` します。これを Jupyter の ipykernel プロセス内でそのまま
> 実行すると、ipykernel 自身のスレッドと衝突してカーネルごと落ちることがあります
> （プレーンな python スクリプトとしての実行では問題ありません）。そのためチュートリアルでは
> チェーンの構築・実行を別プロセスのスクリプトとして書き出し、`subprocess` で実行しています。

---

## 依存関係

- [ANLnext](https://github.com/odakahirokazu/ANLNext)
- [ROOT](https://root.cern/)
- [Boost](https://www.boost.org/) 1.80.0 以降
- CMake 3.8 以降
- C++ コンパイラ（C++17 対応）
- [SWIG](https://www.swig.org/)（Python 拡張の生成に必須）
- Python 3

### 環境変数

ビルド前に、以下の環境変数を設定しておきます（各自の環境に合わせる）。

| 環境変数 | 説明 |
| --- | --- |
| `ANLNEXT_INSTALL` | ANLnext のインストール先。未設定の場合は `$HOME` が使われる。 |

---

## インストール

### 1. ANLnext のインストール

先に ANLnext をインストールしておきます。
（インストール手順は ANLnext のドキュメントを参照してください。）

### 2. EricaNext のビルド

リポジトリのルートディレクトリで、以下を実行します。

```bash
mkdir build install
cd build
cmake ../source/ \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DUSE_PYTHON=ON
make -j8
make install
```

> **メモ:** `USE_PYTHON` は `source/CMakeLists.txt` 内で既に `ON` に設定されているため、
> `-DUSE_PYTHON=ON` の指定は実際には省略可能です（明示のために残しています）。

---

## 環境依存の設定（各自の環境に合わせて変更してください）

上記コマンドのうち、以下のオプションは **実行環境ごとに変わります**。
自分の環境に合わせて書き換えてください。

| オプション | 説明 | 
| --- | --- | 
| `-DCMAKE_INSTALL_PREFIX=../install` | インストール先のディレクトリ。任意の場所に変更可。未指定時は `$HOME` にインストールされる。 | 
| `-DUSE_PYTHON=ON` | Python 拡張のビルド。ソース側で既定 `ON`。 | 
| `make -j8` | ビルドの並列数。`8` は使用するコア数に合わせて変更。 | 

### Python インタプリタを明示指定したい場合

`CreateSwigPython.cmake` は `find_package(Python3 REQUIRED COMPONENTS Interpreter Development)`
を使うため、複数の Python（conda / pyenv / system など）が混在する環境で使用する
Python を固定したい場合は、`Python3_EXECUTABLE` にパスを渡します(ANLnext install時も)。

```bash
cmake ../source/ \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DPython3_EXECUTABLE=$(which python3)
```

---

## 使い方

チュートリアルは `notebooks/01_smile2p_quickstart.ipynb` を参照してください。

ビルドした Python モジュールは `<CMAKE_INSTALL_PREFIX>/lib/python` にインストールされます。
Python から import するには、このディレクトリを `PYTHONPATH` に追加してください。

```bash
export PYTHONPATH=<CMAKE_INSTALL_PREFIX>/lib/python:$PYTHONPATH
```

> **注意**: `/usr/local/lib/python` など、他の場所に別ビルドの `EricaPy` が入っていることが
> あります。`PYTHONPATH` は必ず `EricaNext_try/install/lib/python` を先頭（優先）に
> なるよう追加してください。順序を間違えると意図しない方のビルドが import され、
> パラメータやモジュールの挙動が変わって気づきにくいバグになります。

```python
import anlnext
import EricaPy as erica

a = anlnext.AnalysisChain()
a.chain(erica.SaveFile)
a.with_parameters({"filename": "output.root"})
a.chain(erica.ReadData)
a.with_parameters({"data_dir": "/path/to/raw/data", "reverse_TPC": False, "TPC_Only": False})
# ... CalcFADC, TPC_Calibration, TOT_Skewness, HA_CenterGravity, HA_Calc, Reconstruct と続く
a.run(-1)
```
