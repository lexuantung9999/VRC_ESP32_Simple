#include <Arduino.h>
#include <EEB.h>
#include <Adafruit_PWMServoDriver.h>
#include <main.h>
#include <PS2X_lib.h>
#include <malloc.h>
#include <math.h>


DCMotor         VRC_Motor;
Servo_Motor     VRC_Servo;
PS2X            VRC_PS2;

char PS2_text[100];
int16_t pwm_left, pwm_right;
bool dir_left, dir_right;


#ifndef MAX_PWM
  uint16_t MAX_PWM = 700;
#endif

/*!
  *  @brief  Config IO pin, endstop pin, another pin, ...
*/
void GPIO_config(){
  pinMode(MAX_END_STOP, INPUT_PULLUP); pinMode(MIN_END_STOP, INPUT_PULLUP); 
  pinMode(ANOTHER1, OUTPUT); pinMode(ANOTHER2, OUTPUT); pinMode(ANOTHER3, OUTPUT); 
}


TimerHandle_t xTimers[1]; // using 1 timer
void vTimerCallback(TimerHandle_t xTimer){
    configASSERT(xTimer);
    int ulCount = (uint32_t) pvTimerGetTimerID(xTimer);

    //timer 0 reading gamepad
    if(ulCount==0){
       // Task 1
       VRC_PS2.read_gamepad(0, 0); // khong co PS2 thi ham nay khong chay thanh cong, bi treo

    }
}


void VRC_Control(){
 
  // **** CONTROL MOTOR ***** //
  int16_t val_RY, val_RX;

  val_RY = VRC_PS2.Analog(PSS_RY);
  val_RX = VRC_PS2.Analog(PSS_RX);
    
    // loc nhieu
    if(val_RY>=NOISE_J_UP || val_RY<=NOISE_J_DOWN){
      val_RY = map(val_RY,0,255,MAX_PWM,-MAX_PWM);
    }
    else val_RY = 0;
    if(val_RX>=NOISE_J_UP || val_RX<=NOISE_J_DOWN){
      val_RX = map(val_RX,0,255,MAX_PWM,-MAX_PWM);
    }
    else val_RX=0;

    // tinh toan

    if(val_RY>=0){
        pwm_left = val_RY - val_RX;
        pwm_right = val_RY + val_RX;
    }

    else{
       pwm_left = val_RY + val_RX;
       pwm_right = val_RY - val_RX; 
    }


    if(abs(pwm_left)<=MIN_PWM) pwm_left = 0;
    if(abs(pwm_right)<=MIN_PWM) pwm_right = 0;

    if(pwm_right>MAX_PWM ) pwm_right = MAX_PWM ;
    if(pwm_right<-MAX_PWM ) pwm_right = -MAX_PWM ;

    if(pwm_left>MAX_PWM ) pwm_left = MAX_PWM ;
    if(pwm_left<-MAX_PWM ) pwm_left = -MAX_PWM ;

    if(pwm_left >=0) dir_left =0;
    else {
      dir_left =1; 
      pwm_left = -pwm_left;
    }
    if(pwm_right >=0) dir_right =0;
    else {
      dir_right =1; 
      pwm_right = -pwm_right;
    }
  // ********************** END AGORITHM **************** //

  
  #if GAMEPAD_LOG_INFO
    sprintf(PS2_text,"pwm_left: %d, dir_left: %d  pwm_right: %d, dir_right: %d \n",pwm_left,dir_left,pwm_right,dir_right);
    Serial.println(PS2_text);
    // sprintf(PS2_text,"RX: %d, LX: %d  RY: %d, LY: %d \n",VRC_PS2.Analog(PSS_RX),VRC_PS2.Analog(PSS_LX),VRC_PS2.Analog(PSS_RY),VRC_PS2.Analog(PSS_LY));
    // Serial.println(PS2_text);
    // sprintf(PS2_text,"tri: %d, cros: %d, sqr: %d, circ: %d \n", VRC_PS2.Button(PSB_TRIANGLE),VRC_PS2.Button(PSB_CROSS),VRC_PS2.Button(PSB_SQUARE),VRC_PS2.Button(PSB_CIRCLE));
    // Serial.println(PS2_text);
  #endif 

  // Servo đóng mở kẹp
  if(VRC_PS2.ButtonPressed(PSB_L2)){
    // **** Your code Here ***** //
    VRC_Servo.Angle(90, Servo2);
  }

  if(VRC_PS2.ButtonPressed(PSB_R2)){
    // **** Your code Here ***** //
    VRC_Servo.Angle(180, Servo2);
  }

  //Servo xoay 
  if(VRC_PS2.ButtonPressed(PSB_L1)){
    // **** Your code Here ***** //
    VRC_Servo.Angle(0, Servo1);
  }


  if(VRC_PS2.ButtonPressed(PSB_R1)){
    // **** Your code Here ***** //
    VRC_Servo.Stop(Servo1);
  }


  // ************* Cơ cấu nâng hạ, 2 nút lên xuống, động cơ thứ 4 ****************** //
  // ******** nhấn giữ nút up-> động cơ lên, nhấn giữ nút down, động cơ xuống **** //
  // ************ Nâng lên **************** //
  if(VRC_PS2.Button(PSB_PAD_UP)){
    // **** Your code Here ***** //
    VRC_Motor.Lift(LIFT_MOTOR, LIFT_UP, 3000);
  }
  else if(VRC_PS2.ButtonReleased(PSB_PAD_UP)){
    VRC_Motor.Lift(LIFT_MOTOR, LIFT_STOP,0);
  }

  // ************ Hạ xuống ******************** //
  if(VRC_PS2.Button(PSB_PAD_DOWN)){
    // **** Your code Here ***** //
    VRC_Motor.Lift(LIFT_MOTOR, LIFT_DOWN, 1000);
  }
  else if(VRC_PS2.ButtonReleased(PSB_PAD_DOWN)){
    VRC_Motor.Lift(LIFT_MOTOR, LIFT_STOP,0);
  }

  // ****************** Hết hàm nâng hạ ************************************ //


    // ************* Cơ cấu tịnh tiến trục x, 2 nút trái phải, động cơ thứ 3 ****************** //
  // ******** tưong tự như động cơ nâng hạ **** //
  // ************ Ra **************** //
  if(VRC_PS2.Button(PSB_PAD_LEFT)){
    // **** Your code Here ***** //
    VRC_Motor.Run(MOTOR4,1000,1);
  }
  else if(VRC_PS2.ButtonReleased(PSB_PAD_LEFT)){
    VRC_Motor.Stop(MOTOR4);
  }

  // ************ Vào ******************** //
  if(VRC_PS2.Button(PSB_PAD_RIGHT)){
    // **** Your code Here ***** //
    VRC_Motor.Run(MOTOR4,1000,1);
  }
  else if(VRC_PS2.ButtonReleased(PSB_PAD_RIGHT)){
    VRC_Motor.Stop(MOTOR4);
  }

  // ****************** Hết hàm tịnh tiến ************************************ //


  if(VRC_PS2.ButtonPressed(PSB_SQUARE)){
    // **** Your code Here ***** //
  }

  if(VRC_PS2.ButtonPressed(PSB_PAD_LEFT)){
    // **** Your code Here ***** //
  }

  if(VRC_PS2.ButtonPressed(PSB_CIRCLE)){
     // **** Your code Here ***** //
     // chế độ auto
     // chạy thẳng vào line 1 đoạn
     VRC_Motor.Run(LEFT_MOTOR,700,1);
     VRC_Motor.Run(RIGHT_MOTOR,700,1);
     delay(4000);
     //dừng robot
     VRC_Motor.Stop(LEFT_MOTOR); VRC_Motor.Stop(RIGHT_MOTOR);
     //nâng lên
     VRC_Motor.Lift(LIFT_MOTOR,LIFT_UP,4000);
     delay(3000);
     VRC_Motor.Lift(LIFT_MOTOR,LIFT_STOP,0);
     //nhả kẹp;
     VRC_Servo.Angle(180,Servo2);
     delay(1000);
     // hạ xuống
     VRC_Motor.Lift(LIFT_MOTOR,LIFT_DOWN,4000);
     delay(3000);
     VRC_Motor.Lift(LIFT_MOTOR,LIFT_STOP,0);
     //lùi ra
     VRC_Motor.Run(LEFT_MOTOR,700,0);
     VRC_Motor.Run(RIGHT_MOTOR,700,0);
     delay(4000);
     VRC_Motor.Stop(LEFT_MOTOR); VRC_Motor.Stop(RIGHT_MOTOR);
  }

  if(VRC_PS2.ButtonPressed(PSB_TRIANGLE)){
    // **** Your code Here ***** //
  }

  if(VRC_PS2.ButtonPressed(PSB_CROSS)){
    // **** Your code Here ***** //
  }

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  GPIO_config();
  VRC_Motor.Init();
  VRC_Servo.Init();

  //config ps2:
  int err = -1;

  for(int i=0; i<10; i++){
    delay(100);
    err = VRC_PS2.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    Serial.print(".");
    if(!err){
      Serial.println("Sucsessfully Connect PS2 Controller!");
      break;
    }
  }
    // Timer config
  xTimers[ 0 ] = xTimerCreate("Timer PS2",pdMS_TO_TICKS(100),pdTRUE,( void * ) 0,vTimerCallback);
  xTimerStart(xTimers[0],0);
} 



void loop() {
  // put your main code here, to run repeatedly:

// ******************* CONTROL *************** //
  VRC_Control();
  VRC_Motor.Run(LEFT_MOTOR,pwm_left,dir_left);
  VRC_Motor.Run(RIGHT_MOTOR,pwm_right,dir_right);
  
// ********************** END CONTROL************* //
}



