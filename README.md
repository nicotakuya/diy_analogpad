# diy_analogpad

## overview

XE-1APの互換品を作る試みです。

メガドライブ版「アフターバーナーII」で動作を確認しています。

![diy_analogpad](https://user-images.githubusercontent.com/5597377/174466063-c6dd810d-6be7-47ac-a3cb-714a4f06b8cd.jpg)

## 回路図

"diy_analogpad_schematics.png"を参照。

## parts list

U1:Arduino Pro mini互換モジュール(ATmega328、5V動作、16MHz)
https://akizukidenshi.com/catalog/g/gK-10347/

U2:シリアルUSB変換モジュール(プログラム書き込み時のみ使用)
https://akizukidenshi.com/catalog/g/gM-11007/

CN1:ジョイスティックコネクタ（メス）
https://akizukidenshi.com/catalog/g/gC-00645/

JOY1/2:ジョイスティック（アナログ型、10k ohm可変抵抗）
https://akizukidenshi.com/catalog/g/gK-10263/

SW1-8:スイッチ x8個

PS1:リセッタブルヒューズ(0.2A程度で切断を推奨)
https://akizukidenshi.com/catalog/g/gP-12911/

## 作り方

Arduino IDEを使ってマイコンにプログラム(.ino)を書き込みます。

メガドライブとアナログパッドを接続して電源を入れます。「アフターバーナーII」の場合、タイトル画面で「CONTROL ANALOG-JOY」と表示されたら、認識成功です。

＊Arduinoのブートローダで時間が取り過ぎて、認識に失敗している可能性があります。その場合、メガドライブのリセットボタンを押すと改善します。

## タイミングチャート

"diy_analogpad_timing.png"を参照。

・データの仕様はPCモードとMDモードでそれぞれ違います。ジョイスティックポートのピンアサインも違います。

・1秒間に約60回のペースでMD→PADにREQが送信されます。PADはREQのH→Lを合図に通信を開始します。

・REQ１回あたり、4bit×11回のデータを送信します。8bitデータは4bitずつ2回に分けて送信します。

・REQがL→Hに戻る時間によって、データの転送速度をMDから指定することが可能です。

・D3～D0の4bitのデータをPAD→MDに送信します。送信時、ACK信号で同期を取ります。ACKの1周期ごとにLHは反転します。LHはLから始まります。

・スティック/スロットルの位置は0～255（8bit）の値で表わします。


## movie
https://sites.google.com/site/yugenkaisyanico/diy-analogpad
