# run_script: チェーンの実行と比較のスクリプト

| スクリプト | 何をするか |
| --- | --- |
| [`run_chain.py`](run_chain.py) | 設定ファイル(`config/*.json`)を指定して EricaNext のチェーンを実行する。**通常はこれを使う** |
| [`run_per02.py`](run_per02.py) | Per02 の全イベントを再構成する(`run_chain.py` を決まった引数で呼ぶだけ) |
| [`compare_with_reference.py`](compare_with_reference.py) | 出力を SMILE_tool_box の参照出力とイベントごとに比較する |

どのスクリプトもこのリポジトリのビルド(`install/lib/python` の `EricaPy`)を自分で読み込むので、
`PYTHONPATH` を設定しなくても動きます(先に EricaNext_try をビルドしておく必要があります)。
データの場所は設定ファイルの `{data_top}` で決まります(リポジトリ直下の README の「自分の環境に合わせて変える場所」)。

---

## run_chain.py

設定ファイルに書かれた順番・パラメータでモジュールをつなぎ、チェーンを実行します。

```bash
cd EricaNext_try/run_script
python3 run_chain.py <設定名> -o <出力ファイル> [-n <イベント数>] [--raw] [--compression <設定>]
```

| 引数 | 意味 |
| --- | --- |
| `<設定名>` | `config/` の中の設定ファイルの名前(拡張子なし、例 `smile2p_20190701_Per02`)。`.json` のパスを直接書いてもよい |
| `-o`, `--output` | 出力する ROOT ファイル(必須) |
| `-n`, `--n-events` | 処理するイベント数。省略すると全イベント(`-1`) |
| `--raw` | `ReadData → RawTreeWriter` だけを実行し、同期した生データツリー(`RawTreeWriter/raw`)を書く。再構成はしない |
| `--compression` | ROOT の圧縮設定(100 × アルゴリズム + レベル)。`505` = zstd レベル 5(生データツリーにおすすめ)。省略すると ROOT の既定 |

### 例

```bash
# 全イベントを再構成(Per02 で約 1 時間。データを手元のディスクに置くと速い)
python3 run_chain.py smile2p_20190701_Per02 -o per02_out.root

# 先頭 1 万イベントだけ試す
python3 run_chain.py smile2p_20190701_Per02 -o test.root -n 10000

# 生データツリー(較正・診断用)。全イベントで約 5 GB
python3 run_chain.py smile2p_20190701_Per02 -o per02_raw.root --raw --compression 505

# 別の期間 / パラメータを変えた設定ファイル
python3 run_chain.py smile2p_20190701_Per03 -o per03_out.root
python3 run_chain.py ../config/my_test.json -o my_test.root -n 10000
```

### 出力

| `--raw` なし(再構成) | `--raw` あり(生データ) |
| --- | --- |
| `Reconstruct/single_hit`: 1 イベント 1 行の再構成ツリー(解析には `err_flag == 0` の行を使う) | `RawTreeWriter/raw`: 1 イベント 1 行の生データ(TPC ヒット、FADC 波形、HA の ADC、時刻) |
| `TOT_Skewness/tot_skewness`: 電子飛跡の結果だけのツリー | `RawTreeWriter/run_info`: ボード ID の一覧(1 行) |

各ブランチの意味は `tutorials/1_DataIO/` を参照してください。

### パラメータを変えて試すとき

`config/smile2p_20190701_Per02.json` をコピーして値を書き換え、そのファイルを指定します。

```bash
cp ../config/smile2p_20190701_Per02.json ../config/my_test.json
# my_test.json の "parameters" の値(例: "TPC_Calibration": {"Cal_Modify": ...})を編集
python3 run_chain.py my_test -o my_test.root -n 10000
```

### 中でしていること

```python
cfg = ericanext_config.load("smile2p_20190701_Per02")   # {data_top} を置き換えて読む
a = anlnext.AnalysisChain()
a.chain(EricaPy.SaveFile);  a.with_parameters({"filename": ..., "compression": ...})
for m in cfg["chain"]:                     # --raw のときは ["ReadData", "RawTreeWriter"]
    a.chain(getattr(EricaPy, m))
    a.with_parameters(cfg["parameters"][m])
a.run(n_events)
```

> **Jupyter から使うとき**: `CalcFADC` は `std::thread` を使うため、Jupyter のカーネル内でチェーンを直接動かすと
> カーネルごと落ちることがあります。ノートブックからは `subprocess` でこのスクリプトを呼んでください
> (チュートリアルはそうしています)。

---

## run_per02.py

Per02 の全イベントを、参照と同じパラメータで再構成します。中身は次と同じです。

```bash
python3 run_chain.py smile2p_20190701_Per02 -o per02_out.root
```

出力の `per02_out.root` は `compare_with_reference.py` で参照と比較するためのものです。
追加の引数はそのまま `run_chain.py` に渡されます(例: `python3 run_per02.py -n 10000`)。

---

## compare_with_reference.py

EricaNext の出力(`Reconstruct/single_hit`)を、SMILE_tool_box の参照出力(`Reconstruct/s2t`)とイベントごとに比較します。

```bash
python3 compare_with_reference.py per02_out.root                       # 参照は設定ファイルの reference.root
python3 compare_with_reference.py per02_out.root --ref /path/to/haboc_tmp_0.root
```

| 引数 | 意味 |
| --- | --- |
| `output` | 比較する EricaNext の出力ファイル |
| `--config` | 参照ファイルの場所を読む設定ファイル(既定 `smile2p_20190701_Per02`。`"reference": {"root": ...}`) |
| `--ref` | 参照ファイルを直接指定する(`--config` より優先) |

- 行はイベント番号で対応させます。参照の `anode_id` は 24 ビットのカウンタなので、0 に戻るのを補正しています。
  参照が読み落としているイベント(Per02 では 27 個)は、こちらの出力から除いてから比較します。
- 分岐ごとに「ビット単位で一致」「最下位ビットだけ違う」「それ以上違う」の行数と、
  「それ以上違う」のうち `err_flag == 0` の行の数を表示します。
- Per02 の全イベントでの結果と、残る差の理由は `docs/per02_comparison_log.md` の項目 30 にあります。
- 比較の基準にしているのは Per02 の参照だけです(Per03 の参照は別の版で作られていて、一部の扱いが違うため)。
