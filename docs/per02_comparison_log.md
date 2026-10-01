# 20190701_Per02 突き合わせログ

参照: `/home/onoda/hikaru3_analysis2/sm2+_cal_rootdata/20190701_Per02/haboc_tmp_0.root`
(`Reconstruct/s2t`, 815630 entries, setting.mac: `Cal_Modify:1.4530463`, `Flag_Modify_PMT_Gain:1` 等)

比較用スクリプト: `run_script/run_per02.py`
突き合わせキー: `event_id`(=anode_id、重複あり) ではなく `sevt_id`(0始まり通し番号、`ReadData::Get_SEvtID()`)。
`sevt_id` と参照の `SEvt_ID` は100%一致することを確認済み(ReadDataのマージ処理が参照と完全に同じ順序で処理できている証拠)。

## 見つけて直したバグ

1. **TOT_Skewness `Get_TPC_XYZ` のクアドラント分岐が誤り**
   SMILE-2+は単一クアドラント(384 strip, 1board)。参照コードは
   `center_strip = 128*3/2-0.5 = 191.5` の単純な1本式のみ。
   コミット済みHEADにあったクアドラント分岐(val/384で場合分け)は誤り、修正済み。

2. **TOT_Skewness `CalcSkewness`/`Ve`フィットが3D一致後の点群(caq0/caq1)を使っていた**
   参照は `TOTs_ax`/`TOTs_cx`(全ヒットのraw位置、3D一致フィルタ前)を使う。修正済み。

3. **TPC_Calibration: Cal_Modifyパラメータが存在しなかった**
   `cal_a *= Cal_Modify; cal_b *= Cal_Modify;` を読み込み直後に追加。

4. **HA_Calc: XMDF/YMDFファイル形式が未対応 + PMTオフセット項が抜けていた**
   - `MDF::SetParametersFlat()` を追加(110値/PMTのTMultiDimFitフラットダンプに対応)
   - `ha_x = 54*(pmt%2-0.5) + 6*(x_id-3.5)`, `ha_y = 54*(1-pmt/2) + 6*(y_id-3.5)` のPMTオフセット追加
   - `Flag_Modify_PMT_Gain` + `Mean_Sigma_HA*_137Cs_662keV.dat` による cal_c 上書き追加
   → **Eg, Pgは完全一致(84.6%、残りはそもそもEgがinvalidなイベント)**

5. **CalcFADC: `charge *= ADC_to_mV * Clock_to_nsec / 50.` の単位変換が抜けていた**
   (ADC_to_mV=2, Clock_to_nsec=20 がsetting.mac + コード既定値)
   これが抜けていたため、Keが参照の1.25倍(=1/0.8)になっていた。追加後、大幅改善。
   ※ユーザー確認: Clock_to_nsecは10では?という質問があったが、参照コードのデフォルト(20ns/50MHz)
     とsetting.macの不在(上書きなし)から20が正しいと実験でも確認(10を使うとKeがちょうど半分になり悪化)。

6. **TPC_Calibration: ROOT histogramのbin番号とストリップ値のズレ(off-by-one)**
   `Track_A`/`Track_C` は `Fill(n_strip, clk)` という値ベースなので `GetBinContent` には `+1` が必要なのに、
   生のストリップ値をそのままbin番号として渡していた。さらにループが `ai<max_a` と排他的で、
   各イベントの一番上のストリップ/クロックを常にスキップしていた。
   `GetBinContent(ai+1,clk+1)` + ループを `<=max_a` に修正。
   → **Ke完全一致 1265/2839 (44.6%) → 2259/2839 (79.6%)** に改善。

## 棚上げ(影響なしと確認済み)

- Slackログにあった「`Reverse`フラグを無視して常に`31-bit`で抽出していた」バグ(`RD_Encoder.cxx`)は、
  念のため `Reverse` 条件分岐に直したが、`TPC_Encoder.cxx` 側の `rev_flag` 処理が
  既にビットペア単位の並べ替えを行っているため、現在の設定(`reverse_TPC:True`)では
  **挙動に影響なし**(実験で確認: Reverse=True/False、混在パターン全て試した)。
  修正自体は無害なので残してある。

## 現在未解決: Ke残り20%の不一致パターン

典型例: `sevt_id=59` (`event_id=6867`)。参照 `Ee=376.79 keV`。

- 生のcoincidence数(`n_coinci`) は1472件あり、どこかに本物の大きなトラックがある
  (anode charge[board1,ch0]=1720.53、大きい)。
- しかし `min_a=128, max_a=131` ( = anode strip 128-131の **わずか4ストリップ** )しか
  Track_Aにヒットが記録されていない。本来これだけ大きなエネルギーならもっと広い範囲に
  ヒットがあるはず。
- このストリップ範囲(128-131)はちょうど32ストリップ境界(board1のch0の先頭4ストリップ)に
  ぴったり一致し、エッジ除外(`<4 || >=28`)で**全部**消えてしまい、`Energy=0`になる。
- **エッジ除外を無効化して試しても `Energy=47.2 keV`にしかならず、参照の376.8 keVには遠く届かない**
  (cathode側もstrip 94が32境界のedge(local=30)に近く一部除外されている)。
- → エッジ除外だけの問題ではなく、**そもそも anode の有効ヒット範囲自体が狭すぎる
  (本来はもっと広いはず)** という、別の・より大きい問題がありそう。
  次に疑うべきは: Track_A へのヒット登録条件(RD_Encoder.cxxのbit閾値)と
  CalcFADCの電荷積分条件(threshold=20)が食い違っていて、大振幅パルスの一部strip/clockが
  Track_Aに記録されていない可能性。

## 追加調査 (このラウンド)

7. **CalcFADC: `Range_min`/`Range_max`が抜けていた**
   参照は電荷積分を`[Range_min,Range_max)=[30,500)`で行う(setting.macに上書きなし→デフォルト)。
   こちらは`[base_window, Nclk)=[30,512)`を使っていた。パラメータとして追加し修正。
   → **Ke完全一致 2259/2839 (79.6%) → 2322/2839 (81.8%)** に改善。

### event_id=6867 (Ee=376.8keV) の深掘り結果

- CalcFADCの生波形を直接ダンプしたところ、board1 ch0に実在するパルスを発見
  (clk=168near190、ピーク268、閾値超えサンプル20個、積算charge=1720.53)。
  これは `Track_A` が記録しているヒット範囲(anode strip 128-131, clock 321-390)とは
  **全く違うクロック領域**にある(TRACK側clockは0-1023, FADC側clockは0-511で、
  比較すると概ね2倍のスケール差があるように見えたが、CalcFADCの電荷積分は
  TRACK側のclock範囲と無関係にFADC波形全体を独立に積分するので、これ自体は
  本来Energyの数値には影響しないはずと判明)。
- `TPC_Encoder.cxx`のGetEventID()で、**FADC用とTRACK用でrev_flagによる
  チャンネル入れ替えの並び替え式が異なる**ことを発見し、「FADCのch⇔TRACKのch
  の対応は全rev_flag値について(3-ch)の反転になっているはず」という仮説を立てて
  実装・テストしたが、**全体の一致率をむしろ悪化させ、誤りと判明(即ロールバック済み)**。
- そこで手当たり次第ではなく、**系統的な総当たり検証**を実施:
  20000イベントの `Q_anode[12]`, `Q_cathode[12]`, `HitPanel[12][12]` を
  Reconstructツリーにダンプし(デバッグ用branch、後で消す想定)、Calibration20190130.dat
  から直接 cal_a/cal_b を読み込んでPython側で全24通り(4!)のアノードchannel入れ替え、
  全24通りのカソードchannel入れ替え、さらに全6通り(3!)×6通りのボード入れ替えを
  オフラインで総当たりし、どの並べ替えが参照Eeに一番近いか計測。
  **→ 結果: 現状の並び(identity, 入れ替えなし)が既に最良(9385/11309=83.0%)**。
  つまり **channel/board のインデックス対応は正しい** ことが統計的に確認できた。
  (= event 6867のような個別の謎は、全体に効くインデックスバグではなく、
   もっと局所的・イベント固有の要因と考えられる)

## 現状

20000イベントサンプルで Ke完全一致 **9385/11309 (83.0%)**。
Eg・Pgは引き続き約84.6%(そもそもEgが無効なイベントを除くとほぼ100%)。

## モジュール別レビュー (ユーザー指示: 一つづつ確認)

### HA_CenterGravity (参照: HA_Center_Gravity)

8. **ADC閾値判定が`>`になっていた(参照は`>=`)**
   参照: `under = (adc[j][k] < ADC_Threshold)`、つまり良品条件は`adc>=ADC_Threshold`。
   こちらは`adc[...] > threshold`だった。`>=`に修正。

9. **HA_RAW_E(生のADC和)がエラービットを考慮せず破壊されていた**
   `Get_ADC_HA2p()`はエラーフラグが立ったチャンネルの値を`-1`に差し替えて返す実装だったが、
   参照の`HA_RAW_E`は「エラービットの有無に関わらず、マスク後の12bit値を常に加算する」。
   `HA_Hit_Flag`(=有効な重心ヒットか)の判定にはエラービットを使うが、`HA_RAW_E`自体は
   エラーの有無によらず計算される、という2つの別概念だった。
   → `Get_ADC_HA2p_Raw()`(値とエラーフラグを別々に返す)を新設し、`raw_e`(常時計算)を
     `HA_CenterGravity::Get_RawE()`として公開。

### HA_Calc (参照: HA_Calc_s2::Count_Hit_Num()/Set_HA_Pixel())

10. **SingleHit_Eth / SingleHit_ADCthの純度カットが未実装だった**
    参照は「ジオメトリ的に有効な重心ヒットが1つ」なだけでは不十分で、
    (a) その1ピクセルの較正後エネルギーが`SingleHit_Eth`(=50keV)を超える
    (b) **他の**どのピクセルも生ADC和が`SingleHit_ADCth`(=1000ADC)を超えていない(純度)
    の両方を要求する。こちらはこの2条件を一切チェックしていなかった。
    `HA_Calc::mod_analyze()`末尾に追加実装。

**→ 2万イベントサンプルでの効果: Eg/Pg完全一致 83.0% → 91.3%、Ke完全一致 → 89.2%**
(比較対象行数は11309→9977に減少。これは不純/低エネルギーのイベントを正しく
除外するようになったため=参照とより近い選択基準になった結果)

## 続きのモジュール別レビュー

### CalcFADC / TPC_Calibration: `TPC_Enable`(ボード無効化)

参照は`Nhit_A[j]>0`でボード単位にFADC処理を有効/無効化しているが、無効ボードの
strip一致探索(`AnodeCathodeHit`)もどのみちそのボード範囲で0件になるため、
**最終的なEnergy計算には実質影響しない(パフォーマンス最適化であり正しさには無関係)**
と確認。実装不要と判断。

### TPC_Calibration: `Flag_Strip_Cut`

setting.macで未設定→デフォルトでOFF(`Strip_data=0`)。該当コードは全て
`if(Strip_data)`配下にあり完全に不活性。実装不要と確認。

### Reconstruct: チェーン順序の問題(重要)

11. **`TOT_Skewness`が`Reconstruct`より前にあるため、TOTが反応しないイベント
    (`NUM[0]==0`等でAS_SKIP)が丸ごとReconstructに届いていなかった**
    ANLNextは、あるモジュールがAS_SKIPを返すとそのイベントの残りのモジュールは
    実行されない。参照コードのReconstruct.cxxを読むと、`Eg`/`Pg`/`Ee`は
    HA_Calc_s2・TPC_Calibrationが立てた`Eg`/`Ee`バンクから**無条件に**読んでおり、
    TOT_Skewness(TRA)の成否には依存していない(`cos_alpha_kin`等の運動学量だけが
    `Ve_THA||Ve_TRA`条件で決まる、これは今回の比較対象であるEg/Ke/Pgには影響しない)。
    → `run_per02.py`で`Reconstruct`を`TOT_Skewness`より前に実行するよう並び替え。
    **行数 9977 → 10110 に増加**(TOTが失敗していたイベントのEg/Ke/Pgも拾えるように
    なった)。ただし追加された133行のうち一致したのは7行のみで、全体の一致率は
    91.3%→90.2%にわずかに低下(TOT失敗イベント群に別の不一致要因がある可能性)。

## 現状 (2万イベントサンプル)

行数 10110、Eg完全一致 9114 (90.2%)、Pg完全一致 9114 (90.2%)、Ke完全一致 8908 (88.1%)。

## 新たに拾えた133行の調査

12. **TOT_Skewnessにも同じROOT bin番号ズレのバグが残っていた**
    TPC_Calibrationだけ直して、TOT_Skewnessの`TOT_Correction()`は直す前の
    コード(`GetBinContent(ai,clk)`、`ai<max_a`排他的ループ)のままだった。
    同じ修正(`+1`、`<=`)を適用。ただし **これはTOT_Skewness自身のPe/Ve精度には
    効くが、Eg/Ke/Pgの比較には影響しない**(チェーン順序を先に直したことで、
    Reconstructの実行はもうTOT_Skewnessの成否に依存しなくなったため)。
    回帰なしを確認(修正後も rows=10110, Eg/Pg=9114, Ke=8908 で変化なし)。

13. **新規133行の不一致の原因を特定できず**: これらは`n_coinci`(生のstrip×clock一致数)が
    6〜154と極端に少なく、`max_panel_hit`が100前後(特に90台)で閾値`>100`を
    僅かに下回り`Energy=0`になるケースが多い。
    「閾値100自体が間違いでは」という仮説をオフラインで総当たり検証(0,1,5,...,200)した結果、
    **閾値100が全体の一致数を最大化する、まさにその値だった**(9385/11309でピーク)。
    つまり閾値の値自体は正しく、短いトラック固有の別の未解明要因
    (エッジ除外の境界、または取りこぼしている別パネルへの寄与)が残っている。

## 現状 (2万イベントサンプル、最新)

行数 10110、Eg完全一致 9114 (90.2%)、Pg完全一致 9114 (90.2%)、Ke完全一致 8908 (88.1%)。

## Reconstruct・chain順序をSMILE_tool_boxに合わせる (ユーザー指示)

14. **chain順序をsetting.macの[Def]と同じに変更**
    `ReadData → CalcFADC → TPC_Calibration → HA_CenterGravity → HA_Calc → TOT_Skewness → Reconstruct`

15. **「失敗したらAS_SKIP」から「センチネルを入れて続行」へ全面変更**
    参照のTPC_Calibration / HA_Calc_s2 / TOT_Skewness_Analysis / Reconstructには
    `ANL_SKIP`が一つも無い(grepで確認)。CalcFADC・HA_Center_Gravityにもあるのは
    「データパケット自体が無い」場合のみ。失敗時は`0xffffffff`センチネルを入れて
    チェーンを続行し、Reconstructは毎イベント必ず1行書く(参照s2tの815630行=全イベント)。
    → CalcFADC(`FADC_NoHit`)、TPC_Calibration(`No_TrackCoincidence`)、
      HA_CenterGravity(`HA2p_no_hit`等)、HA_Calc(`HA_OutOfMDFrange`/`HA_NoHits`/
      `HA2p_LowEnergy`/`HA2p_Impure`)、TOT_Skewness(`NUM==0`)、Reconstruct
      (`NotConformEnergy`)のAS_SKIPを全て撤去し、evsフラグは診断用として残した。
    → TPC_Calibrationは参照通り `Ee==0 → 0xffffffff`、そうでなければevs`TPC_Ee_Valid`。
    → HA_Calcは参照の`evs("Eg")`相当として evs`HA2p_EgValid` を立てる。
    → Reconstructは`HA2p_EgValid`/`TPC_Ee_Valid`を見て、無効ならEg/Ke/Pg/角度を
      センチネルにして**毎イベントFill**。
    → 5000イベントで行数5000・sevt_id 0〜4999 を確認(参照と1:1で行が対応)。
    ※ HA_Calcの`|rx|,|ry|>0.5`で棄却する処理も参照には無い(参照はクランプのみ)ので撤去。

16. **HA_Calc: Hit_Flag_Numの数え方が違っていた**
    密なツリーにしたら「参照はEg有効・こちらは無効」が大量(2万中8377行)に出た。
    event_id=634を調べると、こちらの重心有効ピクセルは3個
    (ha0/pmt1 adc≈[1485,1366,1261,1371]=本命、ha0/pmt3 adc≈[74,42,42,63]、
    ha4/pmt1 adc≈[129,141,168,145])で、参照は1個と判定していた。
    参照`Count_Hit_Num()`は `HA_Hit_Flag==1 && HA_Pixel_E > Eth` で数えるので、
    **Eth(50keV)は「重心有効な各ピクセル」に個別に掛かってから個数を数える**。
    こちらは「重心有効ピクセルがちょうど1個」を先に要求していたため、弱いサテライト
    ピクセルがあるだけで無効にしていた。
    → 全重心有効ピクセルの較正後エネルギーを計算し、`>Eth`の個数(Hit_Flag_Num)と、
      重心無効ピクセルで`raw_e>ADCth`の個数(Hit_E_Num)を参照通り数える形に修正。
    (検証の2万イベント実行中に生データのマウントが切れたため、効果は未確認)

17. **TRACK/FADCデコードを参照`RawRead2p`と1行ずつ照合 → Reverse=Trueでは完全に等価**
    - FADC: 10bit値`((w0&3)<<8)|w1`、Reverse時チャンネル`0↔1,2↔3`入替 → 一致
    - TRACK: クロック11bit(`&0x7ff`)、Reverse時ワード`0↔2,1↔3`入替+隣接ビットペア入替、
      TPC_Calibration側は常に`>>(31-bit)`で取り出し → 一致
    - 参照の非Reverse経路はワード入替なし・常に`>>(31-bit)`。こちらの`TPC_Encoder`
      非Reverse経路は`trk[3-ch]`、`RD_Encoder`は(Slack修正で)`>>bit`なので、
      **Reverse=Falseのデータを扱う場合は参照と異なる**。今回のデータ(Reverse=True)には無関係。

## 比較の基準の誤りに気づく → ほぼ完全一致へ

18. **参照s2tの無効行には「前のイベントの値」が残っている**
    参照Reconstructは `if (SingleHit||DoubleHit) { GetBank(Ee/Eg/Pg/...) ... } else { err_flag=1; 角度等=0xffffffff }`
    で、無効時はEe/Eg/Pgを更新しない。SingleHitは `Ee && Eg && (Ve_THA||Ve_TRA)`(THAはchainに無い)。
    → 比較は`err_flag==0`の行で行うべきだった。それで比べたら Eg・Pg は100%一致していた。
    → こちらのReconstructもこの挙動(古い値を残す・err_flag・角度は度数)を再現。
    ※ 参照ファイルでは「Ee&&Egは有効だがVe_TRAは無効」な2イベント(sevt 7904, 16877)で
      Ee/Eg/Pgが更新されerr_flag・角度は前のまま、という v2.20ソースでは起こらない状態だった。
      外側のif条件が`Ee&&Eg`のバイナリだったと解釈して再現。

19. **CalcFADC: 閾値超えサンプルが無いチャンネルの電荷は-100(×0.8=-80)**(こちらは0だった)
20. **CalcFADC: ベースラインは「合計してから割る」**(こちらは各サンプルを割ってから足していた)。
    mean=53.0ちょうどの時、値73のサンプルが`73-53>20`の境界で浮動小数点誤差により出入りしていた。
21. **Cal_Modifyの実効値は1.45305**。参照ツールは`setting.mac`の値をGUIの`TGNumberEntry`経由で
    受け取り有効数字6桁に丸めている(`HabocStatus.scl`に実際の値が記録されている)。
    → `run_per02.py`を`Cal_Modify: 1.45305`に変更。他のパラメータはHabocStatus.sclと全一致を確認。
22. **TPC_Calibration: Eeの和はボードごと(Ee_anode[board])に足してから合計**(和の順序でビット一致)
23. **Reconstruct: 電子質量は SMLib の 510.9989461**、角度は `angle_scatter`/`angle_recoil`/`Alpha_kin`
    と同じ式。`Alpha_kin`の範囲外は `-1000[rad]×RadToDeg = -57295.78`。
24. **TOT_Skewness: ドリフト速度は時間依存**。`drift_velocity.dat`の点を`TH1F`(float)に詰め、
    `anode_time[0]/1e4/3600`[時間]でビンを引く(最後の点はオーバーフロービン)。
    イベント時刻はTPC_Encoderで読み捨てていたヘッダ8〜11バイト目。→ Pe_TRAのzまで完全一致。
25. **TOT_Skewness: Calc_Lenは`short`に切り捨てた最小最大を`Get_TPC_XYZ`でmmに変換、Lenはピッチ倍**
    (こちらは生のストリップ番号のままだった)。3D一致点0個でもCalc_Lenは呼ばれる(参照はUBで0を読む)。
26. **TOT_Skewness: skewnessは参照と同じ式・順序(`pow`、項ごとの除算、ガード無し→0/0=NaN)**。
    3D一致点0個のときは参照バイナリはCalcSkewness前に抜けている(s2tのskewnessが前の値のまま)。
27. **TOT_Skewness: TRAはHA側Pg有効時のみ**(参照`TPC_Track_Recon`の`if(evs->Get("Pg"))`)。
28. **Ve_TRAのフィット**: 参照ファイルはROOT 6.14製で既定ミニマイザは`Minuit`(TMinuit)。
    こちら(6.36)は`Minuit2`が既定なので明示指定。初期傾きの分母ガードも外す(参照通り0/0=NaN)。
    参照s2tの`Ve_TRA_fit_status`を見ると**フィット失敗(4)の軸は必ず傾き0のフォールバック**
    → 同じ扱いにした。6.36のTMinuitはステータスに`100×HESSE`を足すので`%100`で比較。

## 現状 (2万イベント、全行・全分岐を参照s2tと突き合わせ)

| 分岐 | ビット一致 | 最下位ビット差 | その他 |
|---|---|---|---|
| err_flag, Eg, Ee, Pg, TOT_Area, Pe_TRA, Len_TRA, HA_Pixel, Hit_Flag_Num | 20000 | 0 | 0 |
| phi_kin / psi_kin / alpha_kin | 19987 / 19985 / 19997 | 13 / 15 / 3 | 0 |
| skewness_x / y | 19933 / 19947 | 67 / 53 | 0 |
| Ve_TRA_fit_status | 19995 | - | 5 (7904,16877系の古い値) |
| Track_Min/Max | 19999 | 0 | 1 (16877: 参照のUBのゴミ値) |
| Ve_TRA | 18148 | 67 | 1785 |

- 最下位ビット差は`acos`/`pow`(参照を作ったマシンのlibm)由来。同じglibcのPythonでもこちらの値になる。
- Ve_TRAの残差は相対1e-12程度(中央値)。ROOT 6.14と6.36のTMinuitの収束値の差。
  1e-3を超えるのは15行(ほぼ垂直なトラックで傾きの微小差が拡大)。同じROOTでないと一致不可能。

## 次にやること(未解決)

- 815630イベント全数で同じ比較。
- `Reverse=False`の経路は参照と異なる(項目17)。今回のデータには無関係。
- Keは全イベントに広げると両方有効な行での一致が57.6%(HAシングルヒット行に限ると88%)。
  HAシングルヒット以外のイベントでKeがずれる要因を調べる。
- エッジ除外マージンを3や5に変えて再テストしたが、**4が明確に最良**(margin=3では
  Ke完全一致が8908→4384に悪化)。マージン値自体は正しいと確定。
- 短い/低エネルギーのトラック(max_panel_hitが90台など閾値ギリギリ)の残り不一致は、
  まだ原因未特定。次の手がかりが尽きたので、一旦ここで区切りとして報告。
- `cos_alpha_kin`/`phi_kin`/`psi_kin`(Alpha_kin/angle_scatter/angle_recoil関数)を
  SMLib(Compton_common.cxx)と突き合わせて確認する(現在の比較指標には含まれないが、
  今後ARM/backprojectionの比較をする際に必要になる)。
