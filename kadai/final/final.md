# 最終課題プレゼン

## 概要

xv6にInput Methodを実装する

## OSのInput Methodとは?

- コンピュータで{日本語、中国語、韓国, etc}を入力したい!
- キーボードと文字が1:1対応できない言語もけっこうある
  - キーボードはQWERTYなのに、どうやって漢字を入力するの?
- 日本語入力:  
  1. キーボードの入力を、OSが受け取る
  2. OSのInput Methodという機構が、キー入力をInput Method Engineに渡す
  3. Input Method Engineがひらがなに変換し、Input Methodに返してあげる
  4. Input Methodは、ひらがなをプレエディット文字列として描画する
  5. ユーザーが変換キーを押したら、Input MethodからInput Method Engineに変換リクエストを送る
  6. IMEが漢字に変換を行い、候補リストをInput Methodに提示する
  7. Input Methodは変換候補を描画し、ユーザーに選択を促す
  8. ユーザーが選択した変換候補を、Input Methodからアプリケーションに渡す
  - 他の言語も一緒
- というわけで、OSにInput Methodの実装が必要!
  - 普通Input Method Engineはユーザープロセスとして実装するが、今回はカーネル側に実装

## 実装

- 仕組み
  - キー入力は、`consoleintr`という割り込みハンドラが呼ばれて、`cons.buf`に積まれる
  - アプリケーションがreadするとき: `consoleread`が呼ばれ、`cons.buf`の中身が渡される
  - consoleintrの間にInput Methodを挟んで、キー入力を盗む
    - 変換後の値を`cons.buf`に積む
  - 参考: [/kernel/console.c](/kernel/console.c)

- IM自体・IMEの実装
  - [/kernel/im.h](/kernel/im.h) / [/kernel/im.c](/kernel/im.c)
