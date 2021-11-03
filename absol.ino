#include <auto_command_util.h>

// ボタンを押してから離すまでの時間
const uint16_t BUTTON_PUSHING_MSEC = 95;
bool method_in = false;

/*
TIME_LEAP_MODE は時渡りバグを使うときの方法
1 がランクマバグを使用する。ランクマバグはいちいちランクバトルをするためすこしめんどくさい
2 が巣穴の「みんなで」から時渡りする方法。ワットがたまっていいが空を飛ぶスポットが変わるのでコードの分岐が発生する
3 がキャンプによる時渡り。
*/
#define RANKBATTLE 1
#define SUANA 2
#define CAMP 3

const int TIME_LEAP_MODE = SUANA;

long int loopcount = 0;

void myDelay(int delay_ms, unsigned long method_start_ms = -1) {
  if(method_start_ms == -1){
    method_start_ms = millis();
  }
  while(millis() - method_start_ms < delay_ms){
    // やりたいこと
    delay(1);
  }
}

bool myPush(Button b, int d, int count = 1) {
  for (int i = 0; i < count; i++){
    myPushButton(b, BUTTON_PUSHING_MSEC, d);
  }
  myDelay(BUTTON_PUSHING_MSEC);
  return true;
}

bool myPushButton(Button button, int holdtime, int delay_time_ms){
  method_in = true;
  SwitchController().pressButton(button);
  myDelay(holdtime);
  SwitchController().releaseButton(button);
  myDelay(delay_time_ms);
  method_in = false;
  return true;
}

bool myPushHatButton(Hat button, int holdtime, int delay_time_ms)
{
  method_in = true;
  SwitchController().pressHatButton(button);
  myDelay(holdtime);
  SwitchController().releaseHatButton();
  myDelay(delay_time_ms);
  method_in = false;
  return true;
}

bool myTiltJoystick(int lx_per, int ly_per, int rx_per, int ry_per, int tilt_time_msec, int delay_time_ms)
{
  method_in = true;
  SwitchController().setStickTiltRatio(lx_per, ly_per, rx_per, ry_per);
  myDelay(tilt_time_msec);
  SwitchController().setStickTiltRatio(0, 0, 0, 0);
  myDelay(delay_time_ms);
  method_in = false;
  return true;
}

// 移動パート①
void moveToInitialPlayerPosition(){
    //タウンマップ開く
    myPush(Button::X, 1000);
    myPushHatButton(Hat::LEFT_UP, 1000, BUTTON_PUSHING_MSEC);
    myPush(Button::A, 2500);
    
    //カーソルを飛ぶ場所に合わせる
    // myPushHatButton(Hat::UP, 80, BUTTON_PUSHING_MSEC);
    myPushHatButton(Hat::UP_RIGHT, 110, BUTTON_PUSHING_MSEC);
    myPushHatButton(Hat::RIGHT, 80, BUTTON_PUSHING_MSEC);
    
    myDelay(200);
    myPush(Button::A, 1100);
    // 選択画面対策として下入力2回
    myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
    myPush(Button::A, 1300);
    myPush(Button::B, 400, 5);
}

//　移動パート②～戦闘パート
void symbolEncount(){
    SwitchController().setStickTiltRatio(-55, 100, 0, 0);
    myDelay(300); // 少し進む
    myPush(Button::LCLICK, 100, 15);
    myDelay(200); // 鳴らし始める
    SwitchController().setStickTiltRatio(0, 0, 0, 0);
    // 戦闘開始～「たたかう」が表示されるまで待機。
    // 下の時間は、使用する言語、ポケモン等で多少ずれる可能性があり、delay要調整
    // 色違いのエフェクトが約2秒のため、「たたかう」が表示されて2秒以内に次の十字上↑入力がされるようにする

    // プレッシャーのアブソル用
    myPush(Button::A, 200, 2);
    myPush(Button::B, 200, 5);
    // 雨＋エレキフィールド＋プレッシャー
    // ガラルマタドガス（かがくへんか）がいるとプレッシャーが発動しないため精度が上がる。
    myDelay(7750);
    myPush(Button::B, 200, 5);
    // アブソルここまで

    // 戦闘開始、色違い光モーションなければ十字上＞逃げる
    // 色違いならば、1つめの上入力間に合わないため戦闘＞自爆技で自分側瀕死
    // Aボタンを2回以上連続して押す場合は、次のポケモン選択画面で一番上以外のポケモンを繰り出さないようにする
    myPushHatButton(Hat::UP, BUTTON_PUSHING_MSEC, 1000);
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 800);
    myPush(Button::B, 400, 2);
    myPushHatButton(Hat::UP, 1500, BUTTON_PUSHING_MSEC);
    myPush(Button::A, 800, 2);
    myPush(Button::B, 100, 5);
}


// 日付変更する（事前にランクマッチ使用した後の時渡り）
void execTimeLeep(){
  // ホーム画面へ
  if(TIME_LEAP_MODE == RANKBATTLE){
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 4000);
    myPushButton(Button::HOME, BUTTON_PUSHING_MSEC, 1500);
    changeTimeAtHome(3);
  }else if(TIME_LEAP_MODE == SUANA){
    runToSuana();

    // 一回一日進めたうえで画面に戻ってこないと固定シンボルが復活しない気がする。
    suanaTimeLeap(1); // 一日進めて画面に戻ってきて再び巣穴から募集して
    suanaTimeLeap(2); // 一日戻る（ワットもゲット！）

    initPosFromSuana();
  }
}

void changeTimeAtHome(int mode){ // mode 1 => go 1 day, mode 2 => 1 day ago, mode 3 => both
  // ホームで設定まで行く
  myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
  for(int i = 0; i < 5; i++){
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
  }
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 1500);
  
  // 設定 > 本体 > 日付と時刻
  myPushHatButton(Hat::DOWN, 1600, 100);
  myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 180);
  for(int i = 0; i < 4; i++){
    // Switchライトは4、普通のやつは9くらい？
    myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 110);
  }
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 600);

  // 日付と時刻 > 現在の日付と時刻　にて、1日進めてOK
  myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
  myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
  if(mode == 3){
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::UP, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, 1000, 100);
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
    // 1日戻してOK
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
    myPushHatButton(Hat::LEFT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::LEFT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::LEFT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, 1000, 100);
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
  }else if(mode == 1){
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::UP, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, 1000, 100);
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
  }else if(mode == 2){
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
    myPushHatButton(Hat::RIGHT, 1000, 100);
    myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
  }
    // ホーム画面 > ゲーム画面
  myPushButton(Button::HOME, BUTTON_PUSHING_MSEC, 1000);
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 3000);
}

void runToSuana(){
  // myTiltJoystick(0, 0, 20, -100, 230, BUTTON_PUSHING_MSEC);
  myTiltJoystick(25, -100, 0, 0, 2530, BUTTON_PUSHING_MSEC);
}

void initPosFromSuana(){
  myPush(Button::X, 1000);
  myPushHatButton(Hat::LEFT_UP, 1000, BUTTON_PUSHING_MSEC);
  myPush(Button::A, 2500);
  
  //カーソルを飛ぶ場所に合わせる
  myPushHatButton(Hat::RIGHT_DOWN, 180, BUTTON_PUSHING_MSEC);
  
  myDelay(200);
  myPush(Button::A, 1100);
  // 選択画面対策として下入力2回
  myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
  myPushHatButton(Hat::DOWN, BUTTON_PUSHING_MSEC, 100);
  myPush(Button::A, 1300);
  myPush(Button::B, 400, 5);
}

void suanaTimeLeap(int goorback){
  // 以下の二行は一日進めるとき（goorback == 1）には必要ない
  // ワットが手に入らないため
  // だが、ワットが仮にのこっていると面倒なので処理は残した
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 500);
  myPushButton(Button::B, BUTTON_PUSHING_MSEC, 2600);
  // 募集画面に入る
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 1500);
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 4100); // 通信待機中
  myPushButton(Button::HOME, BUTTON_PUSHING_MSEC, 1500);
  changeTimeAtHome(goorback);
  
  myPushButton(Button::B, BUTTON_PUSHING_MSEC, 600); // 募集終了
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 1000);
  myPushButton(Button::A, BUTTON_PUSHING_MSEC, 4600);
}


void setup(){
    myPush(Button::B, 500, 13); 
    // 最初の数回の入力はswitchが認識しない場合があるので、無駄打ちをしておく
    
}

void loop(){
    moveToInitialPlayerPosition();
    if(loopcount > 0){
      execTimeLeep();
    }
    symbolEncount();
    loopcount++;
}
