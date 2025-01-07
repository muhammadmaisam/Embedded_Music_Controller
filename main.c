//Muhammad Maisam
#include "msp.h"
#include "BSP.h"
#include "CortexM.h"
#include "msp432p401r.h"
#define RED       (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*6)))  /* Port 2.6 Output */
#define GREEN     (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*4)))  /* Port 2.4 Output */
#define BLUE      (*((volatile uint8_t *)(0x42000000+32*0x4C42+4*6)))  /* Port 5.6 Output */
uint8_t quadrant = 0; // tracks quadrants
void quadrant_check(void) //this function checks which quadrant the joystick is in
{	
	uint16_t x,y; //declare x and y for reading coordinates of the joystick
	uint8_t select; // declare select to check if the joystick was pressed
	BSP_Joystick_Input(&x, &y, &select); //take input from joystick and store coordinate values in the address of x and y
	if(x>=341 && x<682 && y>=341 && y<682) quadrant = 0; //quadrant 0 coordinates
	else if(x<341 && y>=682 && y<=1023) quadrant = 1; //quadrant 1 coordinates
	else if(x>=341 && x<682 && y>=682 && y<=1023) quadrant = 2; //quadrant 2 coordinates
	else if(x>=682 && x<=1023 && y>=682 && y<=1023) quadrant = 3; //quadrant 3 coordinates
	else if(x<341 && y>=341 && y<682) quadrant = 4; //quadrant 4 coordinates
	else if(x>=682 && x<=1023 && y>=341 && y<682) quadrant = 5; //quadrant 5 coordinates
	else if(x<341 && y<341) quadrant = 6; //quadrant 6 coordinates
	else if(x>=341 && x<682 && y<341) quadrant = 7; //quadrant 7 coordinates
	else if(x>=682 && x<=1023 && y<341) quadrant = 8; //quadrant 8 coordinates
}
void light_set(void)//this function is used to set lights depending on the quadrants
{	
	static uint8_t countA = 0; //counter Group A (quadrant 1, 2 or 3) row 1 (lab manual position 1, 2 or 3)
	static uint8_t countB = 0; //counter Group B (quadrant 4, 0 or 5) row 2 (lab manual position 4, 5 or 6)
	static uint8_t countC = 0; //counter Group C (quadrant 6, 7 or 8) row 3 (lab manual position 7, 8 or 9)
	if(quadrant == 1 || quadrant == 2 || quadrant == 3)//Group A
	{
		countB = 0; //reset count 
		countC = 0; //reset count 
		if (countA == 5){ //every 250ms
			RED = 0x00;
			BLUE = 0x00;
			BSP_RGB_D_Toggle(0, 1, 0); //toggle green
			countA = 0; //reset count 
		}
		countA++;
	}
	else if(quadrant == 4 || quadrant == 0 || quadrant == 5) //Group B
	{
		countA = 0; //reset count 
		countC = 0; //reset count 
		if (countB == 10){ //every 500ms
			GREEN = 0x00;
			BLUE = 0x00;
			BSP_RGB_D_Toggle(1, 0, 0); //toggle red
			countB = 0; //reset count 
		}
		countB++;
	}
	else if(quadrant == 6 || quadrant == 7 || quadrant == 8) //Group C
	{
		countB = 0; //reset count 
		countA = 0; //reset count 
		if (countC == 15){ //every 750ms
			RED = 0x00;
			GREEN = 0x00;
			BSP_RGB_D_Toggle(0, 0, 1); //toggle blue
			countC = 0; //reset count 
		}
		countC++;
	}
	

}
int main(void) //main 
{	//Disable watchdog timer.
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; 
	uint16_t duty_freq[9] = {0,512,512,512,512,512,512,512,512}; //Duty cycle arranged in order of quadrants
	uint16_t frequency[9] = {880,880,988,1109,1175,1319,1480,1661,1760}; //Frequencies arranged in order of quadrants
	BSP_Joystick_Init(); //Initialize BSP_Joystick
	BSP_Buzzer_Init(duty_freq[0]); //Initialize BSP_Buzzer
	BSP_PeriodicTask_Init(quadrant_check, 50, 0); //Bonus 1 Every 20ms check which quadrant
	BSP_PeriodicTask_InitB(light_set, 20, 1); //Bonus 2 Every 50ms changes light
	BSP_RGB_D_Init(0, 0, 0); //Initialize RGB
	//Infinite loop
	while(1){
		PWMCycles = SubsystemFrequency/frequency[quadrant]; //Set PWMCycles
		BSP_Buzzer_Set(duty_freq[quadrant]); // Set Buzzer
		TA0CCR0 = PWMCycles - 1; // defines when output signals are set
	}
}
