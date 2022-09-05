# 今ある全てのベスト提出に対して、切るところを摂動させたり色を調整したりするのを、限界まで試すやつ

```sh
cargo run --release --bin climbers-high -- <出力先のディレクトリ (../output/hoge みたいなやつ)>
```

# 塗ってある色を調整するやつ

以下のコマンドで標準出力に色を調整した isl が出る

```sh
cargo run --release --bin color-free-lunch -- <problem id> <出力 isl> <strict 0|1>
```
strict: strictly backtrack all is colors. 0 by default.

# 切るところを摂動させるやつ
```sh
cargo run --release --bin cut-free-dinner -- <problem id> <出力 isl> <出力先ディレクトリ>
```
