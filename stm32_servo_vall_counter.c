#if 1
#include "led.h"
void hwInit();
void GPIO_ADC_SETTING();
void PWM_SETTING();
void Segment_setting();
unsigned int read_ADC_channel();
void Segment_0(int _cnt);
void Segment_2(int _cnt);
void Segment_3(int _cnt);
void GPIO_PE_SETTING();
void GPIO_PF_SETTING();
void GPIO_PDE_SETTING();
void A_reset();
void B_reset();
void C_reset();
void D_reset();
void A_set();
void B_set();
void C_set();
void D_set();
void number_1();
void number_2();
void number_3();
void number_4();
void number_5();
void number_6();
void number_7();
void number_8();
void number_9();
void number_0();
void A2_reset();
void B2_reset();
void C2_reset();
void D2_reset();
void A2_set();
void B2_set();
void C2_set();
void D2_set();
void number_1_2();
void number_2_2();
void number_3_2();
void number_4_2();
void number_5_2();
void number_6_2();
void number_7_2();
void number_8_2();
void number_9_2();
void number_0_2();
void A3_reset();
void B3_reset();
void C3_reset();
void D3_reset();
void A3_set();
void B3_set();
void C3_set();
void D3_set();
void number_1_3();
void number_2_3();
void number_3_3();
void number_4_3();
void number_5_3();
void number_6_3();
void number_7_3();
void number_8_3();
void number_9_3();
void number_0_3();
void Mydelay(unsigned int ms);

/*
 * TIM
 * [GPIO 선택 및 RCC,MODER,AFRL]
 * 서보모터의 pwm 기능을 사용하기 위해서는 TIM(Timer)가 필요하다. TIM은 pwm(pulse width modulation)을 생성, 입력 캡처, 출력 비교, 타이머 인터럽트 기능이 있다.
 * TIM의 종류는 1부터 14까지 있는데 고급, 일반, 기본이 있다. 사용할 타이머는 일반 타이머 TIM2~TIM5까지이다.
 * TIM2,5는 32비트 카운터, TIM3,4는 16비트 카운터이다. 서보 모터의 데이터시트를 보면 20ms 중에 1ms~2ms에 반응을 줘야하기 때문에 카운트 범위가 작기 때문에 주기가 짧은 16비트를 사용할 것이다.
 * 즉, TIM3을 사용할 것이다. TIM에는 채널이 4가지가 있다. 이 채널들은 input capture, output compare, pwm generation, simple timing 등 기능이 있다.
 * 타이머의 첫 번쨰 채널인 CH1을 사용할 것이고 정리하면, TIM3에서 CH1을 사용할 것이다.
 * TIM3에서 channel 1을 사용하기 위해서 AF2(Alternate Function 2)을 사용해야하고, 그 기능을 쓰기위해서 GPIO 핀 PA6, PB4, PC6 중에서 사용해야한다.
 * PC6을 선택했고, 관련해서 setting 해줘야한다.
 */
/*
 * 타이머를 설정하는 절차
 * 1. 시스템 클럭 설정: 시스템 클럭 소스 및 주파수를 설정한다
 * 2. 타이머 클럭 활성화: RCC 레지스터를 통해 타이머 클럭을 활성화합니다.
 * 3. 타이머 기본 설정: 프리스케일러와 자동 재로드 값을 설정합니다.
 * 4. 타이머 채널 설정: 타이머 채널의 모드와 관련 레지스터를 설정합니다.
 * 5. 인터럽트 설정: 필요한 경우 타이머 인터럽트를 설정하고 NVIC에서 활성화합니다.
 * 6. 타이머 시작: 타이머 카운터를 활성화하여 타이머를 시작합니다.
 * -------------------------------
 * ------------------------------
 *1. 타이머 클럭 활성화
 *2. 타이머 기본 설정
 *3. PWM 모드 설정
 *4. 타이머 시작

 * [TIM - PSC,ARR] - 타이머 기본 설정
 * TIM3을 사용하려면 주변 장치 클럭을 제어하는 레지스터를 써야한다. APB1 버스를 사용해 TIM3의 주소를 찾으면 => 0x4000 0400 ~ 0x4000 07FF
 * APB1ENR(advanced peripheral bus 1) 레지스터를 이용해서 1비트 자리에 위치한 TIM3EN에 1을 할당해준다.
 * 할당해줌으로써, TIM3 clock enable 시켜준다.
 * servo motor의 pwm 주기는 20ms(50Hz)이다. hwInit()함수를 보면 TIM3 에 시스템 클럭이 108MHz 인 것을 확인할 수 있다.
 * 즉, 시스템 클럭 주파수가 108MHz이고 목표로 하는 pwm 주파수가 50Hz(20ms)인것이다.
 *
 * 시스템 클럭으로 타이머 클럭에 영향을 줄 수 있기 떄문에 TIM 레지스터 중에 PSC와 ARR 에 값을 넣어서 조절해주면 된다.
 * 		PSC: 타이머의 입력 클럭 주파수를 원하는 주파수로 맞추기 위해 사용 => f_TIM=f_SYSCLK/PSC+1
 * 		ARR: 타이머가 특정 값에 도달했을 때 카운터를 재설정하는 데 사용 =>f_PWM=f_TIM/ARR+1
 * - PSC=f_SYSCLK/f_TIM -1 을 하면 된다. f_SYSCLK는 108이고 f_TIM는 1000000Hz(1MHz)을 대입한다.
 * 		1MHz인 이유는 주기와 관련된 계산이 간단하고 실제로 사용할 수 있는 유효 주파수이기 때문이다. =>이러한 이유로 대부분의 응용 프로그램에서도 많이 사용!
 * 		PSC=108MHz/1MHz-1=108-1=107 => PSC에 107 값을 넣어주면 된다.
 * - ARR=f_TIM/f_PWM -1 을 하면 된다. f_TIM은 1000000Hz(1MHz)을 넣어주고 f_PWM은 목표로 하는 pwm 주파수를 넣어서 50Hz을 대입하면 된다.
 * 		ARR=1000000Hz/50Hz -1 = 20000 -1 =19999 => ARR에 19999값을 넣어주면 된다.
 *--------------------------------------------------------------------------------------------
 * [TIM - CCMR1, CCR1, CCER] - PWM 모드 설정
 * TIM의 capture와 compare는 주기적 이벤트를 측정하고 특정 시간 간격에서 이벤트를 생성한다.
 * 		Capture: 입력 신호의 주기나 펄스 폭을 측정
 * 		Compare: 타이머가 특정 값에 도달했을 때 지정된 동작을 수행 ex) 출력 신호의 상태 변경
 * 서보모터의 시그널은 20ms(50Hz)에서 1ms~2ms 만큼 신호를 주기 때문에 입력 신호의 주기나 펄스 폭을 측정하고, 타이머가 특정 값에 도달했을 때의 동작을 수행하기 위해서
 * capture와 compare을 이용해야해서 모드를 설정해줘야한다. 서보모터의 pwm 사용이 목적이니까 PWD mode로 바꿔줘야한다.
 * 그럼으로 CCMR1 레지스터를 사용해야한다. (capture/compare mode register 1)
 * 또한 pwm에 변화를 주기 위해서 타이머에 변화를 주는데, 이 타이머의 값에 새로운 값을 줄 때마다 업데이트 될 이벤트에 발생하도록 해야한다.
 * 이 때 Preload Enable을 설정해줘야해서 CCMR1에 있는 OC1PE를 이용해야한다.
 * 또, OC1E는 TIMx_CH1이 출력을 활성화해서 신호를 생성하는 출력을 사용할 수 있게 한다.
 * - CCMR1 의 4~6비트 자리에 OC1M 이 있다. = Output Compare 1 Mode
 * 		비트 110을 넣어주면, pwd mode 1이 되고, 업카운팅에서 채널 1은 TIM_CNT< TIM_CCR1만큼 활성화된다.
 * - CCMR1 의 3비트 자리에 OC1PE가 있다. = Output Compare 1 Preload Enablep
 * 	  CCMR1레지스터의 OC1PE에 1의 값을 넣어 read/write 작업을 preload enable 을 해야한다.
 * 	=> CCMR1 의 OC1M에 110 설정, OC1PE에 1 설정
 *
 * PWM신호는 ARR과 CCR 레지스터의 조합을 통해 생성한다. ARR을 이용해서 카운터 재설정하고 CCR 통해서 출력 신호가 언제 토글될지를 결정해야한다.
 * CRR값을 이용해서 듀티 사이클을 결정해야하고, 듀티 사이클이랑 전체 주기 대비 신호가 high일 때르 유지하는 비율이다.
 * 서보 모터 듀티 사이클이 1ms~2ms(1000Hz ~ 500Hz) 이기 때문에 CCR 레지스터에 값을 넣어줘야한다.
 * 1ms때는 1000000/1000-1= 999, 1.5ms에는 1499, 2ms때는 1000000/500-1=1999을 넣어주면 된다.
 * ===> 실제로 서보모터를 돌려보면 0도~180도가 1000hz~500hz(1ms~2ms)가 아니라 45도~135도 같아 보여서 각 hz에 500씩 뺴고 더했더니 각도가 잘 맞았다.!!!
 *
 * GPIO에서 clock enable 했던것처럼 캡처와 컴페어 채널의 출력을 enable 해야한다.
 * CCER 레지스터의 0비트에 위치한 CC1E(Capture/Compare 1 ouput Enable) 을 이용하면
 *  타이머의 캡처/컴페어 1 채널 출력을 활성화 시킬 수 있다.
 *  => CCER의 CC1E에 1을 설정
 *  --------------------------------------------------------------------------------------
 *	[TIM - CR1] - 타이머 시작
 *	타이머 시작하기 위해서는 CR1(control register)을 이용해야한다.
 *	CCMR1의 OC1E에서 새로운 값을 줄 때마다 자동으로 로드하기 위해 clock 활성화 했으니까
 *	그 값을 업데이트 할지 여부를 제어하기 위해 CR1의 ARPE(Auto-Reload Preload Enable)에 값을 줘야한다.
 *	=> CR1의 ARPE에 1을 설정
 *	또한 CR1d의 CEN(Counter Enable)을 사용해서 타이머 카운터를 시작하고, 클럭 신호에 따라 증가하고, 타이머 동작을 시작시킨다.
 *	타이머 카운터의 동작을 활성화할지의 여부를 확인하고 독장을 제어하기 위해 값을 줘야한다.
 *	=> CR1의 CEN에 1을 설정

 */
//공이 떨어지는 갯수 세는 카운터
unsigned int cnt_0=0;  // ADC1_IN0 일 때의 개수
unsigned int cnt_2=0;  // ADC1_IN2 일 때의 개수
unsigned int cnt_3=0;  // ADC1_IN3 일 때의 개수

//모터의 움직임을 감지하기 위한 플래그
unsigned int motor_flag_0=0;  // ADC1_IN0 일 때의 모터 플래그
unsigned int motor_flag_2=0;  // ADC1_IN2 일 때의 모터 플래그


int main(void) {
	hwInit(); //하드웨어 초기화
	GPIO_ADC_SETTING(); //센서의 아날로그 값을 활용하기 위해 ADC 관련 GPIO 초기화 및 세팅
	PWM_SETTING(); //서보모터 사용하기 위해 PWM을 활용하기 위한 관련 세팅
	Segment_setting(); //떨어지는 공의 개수를 세기 위한 7-segment 3개 setting
	volatile unsigned int adc_value_0; //ADC1_IN0 값
	volatile unsigned int adc_value_2; //ADC1_IN2 값
	volatile unsigned int duty_cycle_0 ; //ADC1_IN0 값에 따른 PWM 주기 설정
	volatile unsigned int duty_cycle_2;//ADC1_IN2 값에 따른 PWM 주기 설정

	set_PWM_duty_cycle_0(2499); //모터 0 초기화
	set_PWM_duty_cycle_2(2499); //모터 1 초기화

	//케이스 별로 구멍에빠지는 순서 정하기
	int case_order=2; //case=0이면 굴러오는대로 1,2,3 순서 - 1이면 3,2,1 순서 - 2이면 2,1,3 - 3이면 1,3,2

	while (1) {
		int channel_flag=0; //세번째 홀에 빠지는 것을 카운트하기 위해서 두번쨰 홀을 감지하는 플래그
		// 만약 빠지는 순서가 1,2,3 일 때 1,2가 다 들어가고 그 다음에 굴러 오는 구슬이 3번째 홀보다 앞인 두번째 홀을 감지해서
		// 2번째에서 빠진 duty 값과 현재 굴러오는 구슬의 duty 값이 다를 때를 구분하기 위한 플래그

		adc_value_0 = read_ADC_channel(0);  // Read from PA0 (ADC1 channel 0)
		Mydelay(1); //너무 빠른
		adc_value_2 = read_ADC_channel(2);  // Read from PA2 (ADC1 channel 2)
		Mydelay(1);


		// ADC -> duty : QRI113 을 통해 adc 값 읽을 것을 pwm으로 바꾸기
		duty_cycle_0 = (adc_value_0 * 2000 / 4095) + 499; //ADC1_IN0 값에 따른 PWM 주기로 변환
		duty_cycle_2 = (adc_value_2 * 2000 / 4095) + 499; //ADC1_IN2값에 따른 PWM 주기로 변환
		if(case_order==0){ //케이스가 0일 때 -> 굴러오는 방향으로 1,2,3 으로 빠지기
			if (duty_cycle_0<2499 && motor_flag_0==0 && motor_flag_2==0 ) {//센서1에 반응이 있고 모터1과 모터 2에 반응이 아무것도 없는 상태일 때
				set_PWM_duty_cycle_0(duty_cycle_0); //duty_cycle_0 만큼 모터의 각도 움직이기
				motor_flag_0 = 1; //모터1를 움직이면 플래그 1
				Mydelay(100); //100ms 만큼 기다렸다가
				stop_PWM_duty_cycle_0(); //모터1 다시 닫기
			}
			if (channel_flag==0&&duty_cycle_2<2499 &&  motor_flag_0 == 1 && motor_flag_2 == 0 ) { //센서2에 반응이 있고 모터1이 지나간 상태이고 모터 2에아직 반응이 없는 상태일 때
				set_PWM_duty_cycle_2(duty_cycle_2); ////duty_cycle_2 만큼 모터의 각도 움직이기
				motor_flag_2 = 1; //모터2를 움직이면 플래그2
				Mydelay(100); 			//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_2(); //모터2 다시 닫기
				channel_flag=1; //세번째 구슬과 지금 두번째 구슬이 센서2에 지나는 값이 다른지 확인하기 위해 플래그 1
			}

			if (channel_flag==1&&duty_cycle_2<2499 && motor_flag_0 == 1 && motor_flag_2 == 1 ) { // 센서2에 반응이 있고 센서2가 반응 한 구슬이 세번째구슬이고, 첫번째, 두번째 모터가 다움직인 후일 때
				stop_PWM_duty_cycle_0(); //모터1 움직이지 않고
				stop_PWM_duty_cycle_2(); //모터2 움직이지 않고
				cnt_3+=1; //구슬 3 카운트
				//다시 while문을 돌리기 위해 다시 motor_flag와 channel_flag를 0으로 초기화
				//즉, 세 개의 구슬 이후 다음 구슬이 굴려질 때
				motor_flag_0 = 0; //
				motor_flag_2 = 0;
				channel_flag=0;
			}
		}
		else if(case_order==1){//케이스가 1일 때 -> 굴러오는 방향으로 3,2,1 으로 빠지기

			if (channel_flag==0&&duty_cycle_2<2499 && motor_flag_0 == 0 && motor_flag_2 == 0 ) {//센서2에 반응이 있고(공이 2번쨰 홀을 지나감.) 모터1,모터2에 반응이 없을 떄
				stop_PWM_duty_cycle_0(); //모터1에변화x
				stop_PWM_duty_cycle_2(); //모터2에 변화 x
				cnt_3+=1; //두번쨰 홀에 안 빠지고 지났으니까 세번째 홀에 구슬이빠지고 카운트
				channel_flag==1; //센서2를 이용해서 세번째 홀에 들어갔다는 플래그
			}

			if (channel_flag==1&&duty_cycle_2<2499 &&  motor_flag_0 == 0 && motor_flag_2 == 0 ) {//모터1,2가 아무반응이 없는 상태에서 센서2을 감지했을  때
				set_PWM_duty_cycle_2(duty_cycle_2);//duty_cycle_2 만큼 모터의 각도 움직이기
				motor_flag_2 = 1;//모터2를 움직이면 플래그2
				Mydelay(100);//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_2();//모터2 다시 닫기
				channel_flag=0;//세번째 구슬과 지금 두번째 구슬이 센서2에 지나는 값이 다른지 확인하기 위해 플래그 1
			}

			if (duty_cycle_0<2499 && motor_flag_0==0 && motor_flag_2==1 ) { //모터2가 움직였고 센서1이 반응했을떄
				set_PWM_duty_cycle_0(duty_cycle_0); //모터1를 움직이고
				motor_flag_0 = 1; //모터1 플래그
				Mydelay(100); //100ms만큼 기다렸다가
				stop_PWM_duty_cycle_0(); //모터1 닫기
			}

			if( motor_flag_0 == 1 && motor_flag_2 == 1){ //3개의 홀에 다 빠진 경우
				// 다시 motor_flag 초기화
				motor_flag_0 = 0;
				motor_flag_2 = 0;
			}
		}
		else if(case_order==2){//케이스가 2일 때 -> 굴러오는 방향으로 2,1,3 으로 빠지기
			if (duty_cycle_2<2499 &&  motor_flag_0 == 0 && motor_flag_2 == 0 ) { //센서2 감지하고 모터1,2 반응이 없었을 때
				set_PWM_duty_cycle_2(duty_cycle_2); ////duty_cycle_2 만큼 모터의 각도 움직이기
				motor_flag_2 = 1;//모터2를 움직이면 플래그
				Mydelay(100); 	//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_2();//모터2 다시 닫기
				channel_flag=1; // 구슬2 이후에 들어오는 구슬3이 센서2에 감지되어 빠지지 않게 하기 위해서 플래그

			}
			if (duty_cycle_0<2499 && motor_flag_0==0 && motor_flag_2==1 ) {//센서1 감지하고 모터2가 돌아간 상태이고 모터1이 안돌아갔을 때
				set_PWM_duty_cycle_0(duty_cycle_0); ///duty_cycle_0 만큼 모터의 각도 움직이기
				motor_flag_0 = 1;//모터1를 움직이면 플래그
				Mydelay(100);//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_0();//모터1 다시 닫기
			}
			if (channel_flag==1&& duty_cycle_2<2499 && motor_flag_0 == 1 && motor_flag_2 == 1 ) { //모터1,모터2개 돌아갔고 두번쨰홀에 구슬이 들어간 상태이고, 센서2가 세번째 구슬을 감지했을 때
				stop_PWM_duty_cycle_0(); //모터1의 변화 x
				stop_PWM_duty_cycle_2(); //모터2의 변화 x
				cnt_3+=1; //모터1,2에 반응이 없으니까 구멍에 빠지지 않고 세번째 홀에 구슬이빠졌으니까 카운트
				// 세개의 구슬이 지나가고 motor_flag와 channel_flag 다시 초기화
				motor_flag_0 = 0;
				motor_flag_2 = 0;
				channel_flag=0;
			}
		}
		else if(case_order==3){//케이스가 3일 때 -> 굴러오는 방향으로 1,3,2 으로 빠지기
			if (duty_cycle_0<2499 && motor_flag_0==0 && motor_flag_2==0 ) { //모터1,2가 안 돌아갔었고 (첫번째순서이고) 센서1을 감지했을 때
				set_PWM_duty_cycle_0(duty_cycle_0); //duty_cycle_0 만큼 모터의 각도 움직이기
				motor_flag_0 = 1;//모터1를 움직이면 플래그
				Mydelay(100);//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_0();//모터1 다시 닫기
			}
			if (channel_flag==0&& duty_cycle_2<2499 && motor_flag_0 == 1 && motor_flag_2 == 0 ) { //순서가 1,3,2 일 때 첫번째구슬이 떨어지고 센서2가 감지했을 떄
				stop_PWM_duty_cycle_0(); // 모터1 반응 x
				stop_PWM_duty_cycle_2();// 모터2 반응 x
				Mydelay(500); //센서2 지나고 바로 카운트 되는게 아니라 시간 지난 후에
				cnt_3+=1; //카운트 증가
				channel_flag=1; //2번쨰 순서인 세번째 홀에 빠졌으니 플래그
			}
			if (channel_flag==1&&duty_cycle_2<2499 &&  motor_flag_0 == 1 && motor_flag_2 == 0 ) { //모터1이 돌아갔고 세번쨰 홀에 빠졌고, 센서2가 감지했고 아직 모터2가 안 돌아갔을 떄
				set_PWM_duty_cycle_2(duty_cycle_2); ////duty_cycle_2 만큼 모터의 각도 움직이기
				motor_flag_2 = 1;//모터2를 움직이면 플래그
				Mydelay(100); 	//100ms만큼 기다렸다가
				stop_PWM_duty_cycle_2();//모터2 다시 닫기
				// 세개의 구슬이 지나가고 motor_flag와 channel_flag 다시 초기화
				channel_flag=0;
				motor_flag_0 = 0;
				motor_flag_2 = 0;
			}
		}

		//카운트한 세븐세그먼트를 개수만큼 표시
		Segment_0(cnt_0); //홀1에 빠진 공의 개수
		Segment_2(cnt_2);//홀2에 빠진 공의 개수
		Segment_3(cnt_3);//홀3에 빠진 공의 개수

		Mydelay(10); //빠른 반응을 막기 위한 딜레이
	}
}

void set_PWM_duty_cycle_0(unsigned int duty_cycle_0) { //듀티 사이클을 이용해서 모터 움직이고 모터가 열린고 닫힐 때 첫 번쨰 홀에 공 떨어지는 카운트 하는 함수
	*((volatile unsigned int*)(0x40000434U)) = duty_cycle_0; //TIM3_CH1
	if (duty_cycle_0 < 2499) { //서보모터의 pwm 최대가 2499 이하일 때 센서를 감지한 것
		motor_flag_0 =1; // 플래그를 세운다
	}
	else if (motor_flag_0 && duty_cycle_0 >= 2499) { //플래그가 1이고 서보모터가 다시 닫히면
		Mydelay(10);
		cnt_0++; // 떨어지는 공 카운트
		motor_flag_0 = 0; // 플래그 다시 0으로 설정
	}
}void set_PWM_duty_cycle_2(unsigned int duty_cycle_2) {//듀티 사이클을 이용해서 모터 움직이고 모터가 열린고 닫힐 때 두 번쨰 홀에 공 떨어지는 카운트 하는 함수
	*((volatile unsigned int*)(0x40000438U)) = duty_cycle_2; //TIM3_CH2
	if (duty_cycle_2 < 2499) {//서보모터의 pwm 최대가 2499 이하일 때 센서를 감지한 것
		motor_flag_2 =1;// 플래그를 세운다
	}
	else if (motor_flag_2 && duty_cycle_2 >= 2499) {//플래그가 1이고 서보모터가 다시 닫히면
		Mydelay(10);
		cnt_2++;// 플래그를 세운다
		motor_flag_2 = 0;// 플래그 다시 0으로 설정
	}
}

void GPIO_ADC_SETTING(void) {
	*((volatile unsigned int*)(0x40023830U)) |= 0x00000001U;   // RCC - GPIOA clock enable
	*((volatile unsigned int*)(0x40023844U)) |= 0x00000100U;   // RCC_APB2ENR - ADC1 clock enable
	// GPIOA pins 0, 2, 3 to analog mode
	*((volatile unsigned int*)(0x40020000U)) |= 0x000000F3U;    // GPIOA_MODER - PA0, PA1, PA2 to analog mode (11)
	*((volatile unsigned int*)(0x40012000U))=0; //SR 초기화
	*((volatile unsigned int*)(0x40012010U)) |= 0x0000FC7U;    // ADC_SMPR2 (sample time register 2) - 111 input -> 480 cycles for channels 0, 1, 2
	*((volatile unsigned int*)(0x40012034U)) |= 0x00000043U; //ADC의 채널 순서 정하기
	*((volatile unsigned int*)(0x40012008U)) |= 0x00000003U;   // ADC_CR2 (control register 2) - ADON (A/D converter ON/OFF) + continuous conversion -> 1 input => enable ADC
}

unsigned int read_ADC_channel(unsigned int channel) {
	*((volatile unsigned int*)(0x40012034U)) = channel; // SQR3 을 바탕으로 채널 별로 ADC 값 구하기
	*((volatile unsigned int*)(0x40012008U)) |= 0x40000000U;  // ADC_CR2 - SWSTART (start conversion of regular channels)
	while (!(*((volatile unsigned int*)(0x40012000U)) & 0x00000002U));  // ADC_SR - EOC (end of conversion) flag
	*((volatile unsigned int*)(0x40012000U)) |= 0x00000000U; //regular channel end of conversion
	return *((volatile unsigned int*)(0x4001204CU));  // ADC_DR (regular data register)
}

void PWM_SETTING(void) {
	// GPIO_PC6_ARD_D1-setting
	*((volatile unsigned int*)(0x40023830U)) |= 0x00000004U;    // RCC - GPIOC clock enable
	*((volatile unsigned int*)(0x40020800U)) |= 0x00002000U; 		//MODER - bit:10 => alternate function mode
	*((volatile unsigned int*)(0x40020820U)) |= 0x02000000U;		//AFRL(alternate function low register) - bit:0010 =>AF2
	//GPIO_PC7_ARD_D0-setting
	*((volatile unsigned int*)(0x40023830U)) |= 0x00000004U;    // RCC - GPIOC clock enable
	*((volatile unsigned int*)(0x40020800U)) |= 0x00008000U;    // MODER - bit:14 => alternate function mode
	*((volatile unsigned int*)(0x40020820U)) |= 0x20000000U;    // AFRL(alternate function low register) - bit:0010 => AF2

	//TIM3
	*((volatile unsigned int*)(0x40023840U)) |= 0x00000002U; //RCC(reset and clock control) - TIM3 clock enable
	*((volatile unsigned int*)(0x40000428U)) = 0b01101011U; //PSC(prescaler) - 107    2진수
	*((volatile unsigned int*)(0x4000042CU)) |= 0b0100111111110011U; //ARR(auto-reload register) - 19999    2진수
	//TIM3_channel 1
	*((volatile unsigned int*)(0x40000418U)) |= 0X00000060U; //CCMR1(capture/compare mode register) -pwd mode
	*((volatile unsigned int*)(0x40000418U)) |= 0X00000008U; //CCMR1(capture/compare mode register) -output compare 1 preload enable
	*((volatile unsigned int*)(0x40000420U)) |= 0X00000001U; //CCER(capture/compare enable register) -capture/compare 1 output enable
	//TIM3_channel 2
	*((volatile unsigned int*)(0x40000418U)) |= 0X00006000U; //CCMR1(capture/compare mode register) - OC1M(out compare 1 mode) -> 0110 => pwd mode
	*((volatile unsigned int*)(0x40000418U)) |= 0X00000800U; //CCMR1(capture/compare mode register) - OC1PE (output compare 1 preload enable)
	*((volatile unsigned int*)(0x40000420U)) |= 0x0010U;        // CCER(capture/compare enable register) - capture/compare 2 output enable
	*((volatile unsigned int*)(0x40000400U)) |= 0X0080U; //CR1(control register) - auto-reload preload enable
	*((volatile unsigned int*)(0x40000400U)) |= 0X0001U; //CR1(control register) - counter enable
}


/*
 * TEST_PIN7 : PE10
 * TEST_PIN8 : PE11
 * TEST_PIN9 : PE12
 * TEST_PIN10 : PE13
 * PE => 4002 1000
 *
 * TEST_PIN13 : PF0
 * TEST_PIN14 : PF1
 * TEST_PIN15 : PF2
 * TEST_PIN17 : PF4
 * PF => 4002 1400
 *
 * TEST_PIN11 : PE14
 * TEST_PIN12 : PE15
 * TEST_PIN16 : PD8
 * TEST_PIN18 : PD9
 * PE => 4002 1000
 */

void Segment_setting(){
	GPIO_PE_SETTING(); //첫번째 홀에 빠지는 공 카운트 표시하기 위한 세그먼트 함수
	GPIO_PF_SETTING();//두번째 홀에 빠지는 공 카운트 표시하기 위한 세그먼트 함수
	GPIO_PDE_SETTING();//세번째 홀에 빠지는 공 카운트 표시하기 위한 세그먼트 함수
}

void Segment_0(int _cnt){ //ADC1_IN0일 때 7세그먼트, 각 숫자 표시
	if(_cnt==1){
		number_1();
	}
	else if(_cnt==2){
		number_2();
	}
	else if(_cnt==3){
		number_3();
	}
	else if(_cnt==4){
		number_4();
	}
	else if(_cnt==5){
		number_5();
	}
	else if(_cnt==6){
		number_6();
	}
	else if(_cnt==7){
		number_7();
	}
	else if(_cnt==8){
		number_8();
	}
	else if(_cnt==9){
		number_9();
	}
	else if(_cnt==0){
		number_0();
	}
}

void Segment_2(int _cnt){ //ADC1_CH2 일 때 세그먼트, 각 숫자표현
	if(_cnt==1){
		number_1_2();
	}
	else if(_cnt==2){
		number_2_2();
	}
	else if(_cnt==3){
		number_3_2();
	}
	else if(_cnt==4){
		number_4_2();
	}
	else if(_cnt==5){
		number_5_2();
	}
	else if(_cnt==6){
		number_6_2();
	}
	else if(_cnt==7){
		number_7_2();
	}
	else if(_cnt==8){
		number_8_2();
	}
	else if(_cnt==9){
		number_9_2();
	}
	else if(_cnt==0){
		number_0_2();
	}
}

void Segment_3(int _cnt){ //ADC1_CH3 일 떄 세그먼트 함수 ,각 숫자표시
	if(_cnt==1){
		number_1_3();
	}
	else if(_cnt==2){
		number_2_3();
	}
	else if(_cnt==3){
		number_3_3();
	}
	else if(_cnt==4){
		number_4_3();
	}
	else if(_cnt==5){
		number_5_3();
	}
	else if(_cnt==6){
		number_6_3();
	}
	else if(_cnt==7){
		number_7_3();
	}
	else if(_cnt==8){
		number_8_3();
	}
	else if(_cnt==9){
		number_9_3();
	}
	else if(_cnt==0){
		number_0_3();
	}
}

void GPIO_PE_SETTING(){ // 세그먼트0를 사용하기 위한 셋팅 함수
	*(volatile unsigned int*)(0x40023830U)  |=(0x00000010U);    //RCC_AHB1ENR -> E clock enable
	//A - PE10 - TEST_PIN7
	*(volatile unsigned int*)(0x40021000U)  |=(0x00100000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0x00300000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x00100000U);   // PULL-up
	//B - PE11 - TEST_PIN8
	*(volatile unsigned int*)(0x40021000U)  |=(0x00400000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0x00C00000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x00400000U);   // PULL-up
	//C - PE12 - TEST_PIN9
	*(volatile unsigned int*)(0x40021000U)  |=(0x01000000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0x03000000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x01000000U);   // PULL-up
	//D - PE13 - TEST_PIN10
	*(volatile unsigned int*)(0x40021000U)  |=(0x04000000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0x0C000000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x04000000U);   // PULL-up
}
// 첫 번쨰 홀에 빠진 공을 카운트하기 위해 - 7447 레지스터를 활용해서 값을 주기 위한 셋,리셋 함수
//reset = Low
// set = high
void A_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x04000000U); //BSRR_reset
}
void B_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x08000000U); //BSRR_reset
}
void C_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x10000000U); //BSRR_reset
}
void D_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x20000000U); //BSRR_reset
}
void A_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00000400U); //BSRR_set
}
void B_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00000800U); //BSRR_set
}
void C_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00001000U); //BSRR_set
}
void D_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00002000U); //BSRR_set
}
// 홀 1에 빠진 공의 개수만큼 7447ic에 맞게 ABCD 셋팅
void number_1(){
	A_set();
	B_reset();
	C_reset();
	D_reset();
}
void number_2(){
	A_reset();
	B_set();
	C_reset();
	D_reset();
}
void number_3(){
	A_set();
	B_set();
	C_reset();
	D_reset();
}
void number_4(){
	A_reset();
	B_reset();
	C_set();
	D_reset();
}
void number_5(){
	A_set();
	B_reset();
	C_set();
	D_reset();
}
void number_6(){
	A_reset();
	B_set();
	C_set();
	D_reset();
}
void number_7(){
	A_set();
	B_set();
	C_set();
	D_reset();
}
void number_8(){
	A_reset();
	B_reset();
	C_reset();
	D_set();
}
void number_9(){
	A_set();
	B_reset();
	C_reset();
	D_set();
}
void number_0(){
	A_reset();
	B_reset();
	C_reset();
	D_reset();
}


void GPIO_PF_SETTING(){ // 세그먼트2를 사용하기 위한 셋팅 함수
	*(volatile unsigned int*)(0x40023830U)  |=(0x00000020U);    //RCC_AHB1ENR -> E clock enable
	//A - PF0 - TEST_PIN13
	*(volatile unsigned int*)(0x40021400U)  |=(0x00000001U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021408U)  |=(0x00000003U);   //Speed high
	*(volatile unsigned int*)(0x4002140CU)  |=(0x00000001U);   // PULL-up
	//B - PF1 - TEST_PIN14
	*(volatile unsigned int*)(0x40021400U)  |=(0x00000004U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021408U)  |=(0x0000000CU);   //Speed high
	*(volatile unsigned int*)(0x4002140CU)  |=(0x00000004U);   // PULL-up
	//C - PF2 - TEST_PIN15
	*(volatile unsigned int*)(0x40021400U)  |=(0x00000010U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021408U)  |=(0x00000030U);   //Speed high
	*(volatile unsigned int*)(0x4002140CU)  |=(0x00000010U);   // PULL-up
	//D - PF4 - TEST_PIN17
	*(volatile unsigned int*)(0x40021400U)  |=(0x00000100U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021408U)  |=(0x00000300U);   //Speed high
	*(volatile unsigned int*)(0x4002140CU)  |=(0x00000100U);   // PULL-up
}
// 두 번쨰 홀에 빠진 공을 카운트하기 위해 - 7447 레지스터를 활용해서 값을 주기 위한 셋,리셋 함수
void A2_reset(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00010000U); //BSRR_reset
}
void B2_reset(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00020000U); //BSRR_reset
}
void C2_reset(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00040000U); //BSRR_reset
}
void D2_reset(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00100000U); //BSRR_reset
}
void A2_set(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00000001U); //BSRR_set
}
void B2_set(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00000002U); //BSRR_set
}
void C2_set(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00000004U); //BSRR_set
}
void D2_set(){
	*(volatile unsigned int*)(0x40021418U)  |=(0x00000010U); //BSRR_set
}
// 홀 2에 빠진 공의 개수만큼 7447ic에 맞게 ABCD 셋팅
void number_1_2(){
	A2_set();
	B2_reset();
	C2_reset();
	D2_reset();
}

void number_2_2(){
	A2_reset();
	B2_set();
	C2_reset();
	D2_reset();
}

void number_3_2(){
	A2_set();
	B2_set();
	C2_reset();
	D2_reset();
}
void number_4_2(){
	A2_reset();
	B2_reset();
	C2_set();
	D2_reset();
}
void number_5_2(){
	A2_set();
	B2_reset();
	C2_set();
	D2_reset();
}
void number_6_2(){
	A2_reset();
	B2_set();
	C2_set();
	D2_reset();
}
void number_7_2(){
	A2_set();
	B2_set();
	C2_set();
	D2_reset();
}
void number_8_2(){
	A2_reset();
	B2_reset();
	C2_reset();
	D2_set();
}
void number_9_2(){
	A2_set();
	B2_reset();
	C2_reset();
	D2_set();
}
void number_0_2(){
	A2_reset();
	B2_reset();
	C2_reset();
	D2_reset();
}
void GPIO_PDE_SETTING(){// 세그먼트3를 사용하기 위한 셋팅 함수
	*(volatile unsigned int*)(0x40023830U)  |=(0x00000010U);    //RCC_AHB1ENR -> E clock enable
	*(volatile unsigned int*)(0x40023830U)  |=(0x00000008U);    //RCC_AHB1ENR -> D clock enable
	//A - PE14 - TEST_PIN11
	*(volatile unsigned int*)(0x40021000U)  |=(0x10000000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0x30000000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x10000000U);   // PULL-up
	//B - PE15 - TEST_PIN12
	*(volatile unsigned int*)(0x40021000U)  |=(0x40000000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40021008U)  |=(0xC0000000U);   //Speed high
	*(volatile unsigned int*)(0x4002100CU)  |=(0x40000000U);   // PULL-up
	//C - PD8 - TEST_PIN16
	*(volatile unsigned int*)(0x40020C00U)  |=(0x00010000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40020C08U)  |=(0x00030000U);   //Speed high
	*(volatile unsigned int*)(0x40020C0CU)  |=(0x00010000U);   // PULL-up
	//D - PD9 - TEST_PIN18
	*(volatile unsigned int*)(0x40020C00U)  |=(0x00040000U);    //GPIO output mode
	*(volatile unsigned int*)(0x40020C08U)  |=(0x000C0000U);   //Speed high
	*(volatile unsigned int*)(0x40020C0CU)  |=(0x00040000U);   // PULL-up
}
// 세 번쨰 홀에 빠진 공을 카운트하기 위해 - 7447 레지스터를 활용해서 값을 주기 위한 셋,리셋 함수
void A3_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x40000000U); //BSRR_reset
}
void B3_reset(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x80000000U); //BSRR_reset
}
void C3_reset(){
	*(volatile unsigned int*)(0x40020C18U)  |=(0x01000000U); //BSRR_reset
}
void D3_reset(){
	*(volatile unsigned int*)(0x40020C18U)  |=(0x02000000U); //BSRR_reset
}
void A3_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00004000U); //BSRR_set
}
void B3_set(){
	*(volatile unsigned int*)(0x40021018U)  |=(0x00008000U); //BSRR_set
}
void C3_set(){
	*(volatile unsigned int*)(0x40020C18U)  |=(0x00000100U); //BSRR_set
}
void D3_set(){
	*(volatile unsigned int*)(0x40020C18U)  |=(0x00000200U); //BSRR_set
}

// 홀 3에 빠진 공의 개수만큼 7447ic에 맞게 ABCD 셋팅
void number_1_3(){
	A3_set();
	B3_reset();
	C3_reset();
	D3_reset();
}
void number_2_3(){
	A3_reset();
	B3_set();
	C3_reset();
	D3_reset();
}

void number_3_3(){
	A3_set();
	B3_set();
	C3_reset();
	D3_reset();
}
void number_4_3(){
	A3_reset();
	B3_reset();
	C3_set();
	D3_reset();
}
void number_5_3(){
	A3_set();
	B3_reset();
	C3_set();
	D3_reset();
}
void number_6_3(){
	A3_reset();
	B3_set();
	C3_set();
	D3_reset();
}
void number_7_3(){
	A3_set();
	B3_set();
	C3_set();
	D3_reset();
}
void number_8_3(){
	A3_reset();
	B3_reset();
	C3_reset();
	D3_set();
}
void number_9_3(){
	A3_set();
	B3_reset();
	C3_reset();
	D3_set();
}
void number_0_3(){
	A3_reset();
	B3_reset();
	C3_reset();
	D3_reset();
}


void Mydelay(unsigned int ms) { // 빠르게 변화하는 것을 막기 위한 함수.
	volatile unsigned int delay;
	for (delay = 0; delay < 36000 * ms; delay++);
}
#endif
