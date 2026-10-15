# vendor: SMILE_tool_box_v2.20 のソースをそのまま取り込んだもの

SMILE-2+ の形状(`.mqo` + コードで書いた HA)を組み立てるために、元のコードを変更最小限で使っています。
変更した箇所は各ファイルの `// EricaSim:` のコメント、および `docs/simulation_port_log.md` を参照。

| フォルダ | 元 |
| --- | --- |
| `ggg/` | `SMLib/libs/GGG`(`.mqo` を読み、`G4TessellatedSolid` を作る) |
| `haboc_g4/` | `haboc/src/haboc_g4/SMILE_Material.cxx`, `haboc/include/SMILE_Material.h`(物質の定義) |
| `ha_common/` | `SMLib/libs/HA_common`(HA の番号・位置。`SM_Log.h` は SMLib の代わりの最小限のもの) |
| `smile2p_geo/` | `Erica/SMILE_2p/SMILE_2p_Geo.{h,cxx}`(SMILE-2+ の形状の組み立て) |
| `gd_capture/` | `haboc/src/haboc_g4/` の `GdCaptureGammas_ggarnet`, `GdParticleHPCaptureFS`, `GdParticleHPCapture`, `SKNeutronCapturePhysics`(→ `GdNeutronCapturePhysics`)。Gd の中性子捕獲(`GdPhysics 1`)。Geant4 11 向けに書き直し(物理は同じ。各ファイルの先頭のコメント) |
