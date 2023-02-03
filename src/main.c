
/*
 * MEEC - AS: SISTR 20/21
 * Trabalho de desenvolvimento 2: ADC + DMA com FreeRTOS
 * Autores: António Santos 1171178
 * 	    David Pinto	1200474
 *
 * Ficha 7 incluída
 *
 * Alterações feitas em FreeRTOSConfig.h
 *
 */


/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"
#include "semphr.h"

#define ADC_buffer 3

//Variaveis Globais
volatile uint16_t ADC_values[ADC_buffer];
typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
}ADC_Struct;

// HANDLRES GLOBAIS
TaskHandle_t HandleTask1;
TaskHandle_t HandleTask2;
TaskHandle_t HandleTask3;
TaskHandle_t HandleTask4;

QueueHandle_t xQueue;
QueueHandle_t xUSART_Send;
QueueHandle_t xQueueADC;
QueueHandle_t xQueueJoystick;


/* Task priorities. */
#define mainFLASH_TASK_PRIORITY	( tskIDLE_PRIORITY + 1)
#define mainLCD_TASK_PRIORITY	( tskIDLE_PRIORITY + 1)
/*Tarefas prioritarias*/
#define mainADC_TASK_PRIORITY	( tskIDLE_PRIORITY + 2)
#define mainMensagem_TASK_PRIORITY	( tskIDLE_PRIORITY + 2)
/*********************/
/* The rate at which the flash task toggles the LED. */
#define mainFLASH_DELAY			( ( TickType_t ) 1000 / portTICK_PERIOD_MS )//1s
/* The rate at LCD is refreshed. */
#define mainLCD_DELAY			( ( TickType_t ) 2000 / portTICK_PERIOD_MS )//2s
/* The rate at which the message is sent to the USART. */
#define mainUSART_DELAY			( ( TickType_t ) 1000 / portTICK_PERIOD_MS )//1s

/* Configure RCC clock at 72 MHz */
static void prvSetupRCC( void );

/* Configure GPIO. */
static void prvSetupGPIO( void );

/* Simple LED toggle task. */
static void prvLED( void *pvParameters );

/* LCD activity task. */
static void prvLcdTask( void *pvParameters );

static void prvADC_read(void *pvParameters);//Tarefa de Leitura ADC

static void prvMensagem(void *pvParameters);//Tarefa de enviar mensagens para a USART

/***************************************/
void NVIC_config(void);
/* Drivers Joystick */
char joystick_read(void);

void joystick_setup(void);
/******************/
/* Drivers USART */
void USART_init (uint32_t baudrate, uint16_t buffer_depth);

char prvUSART_GetChar();

void prvUSART_PutChar(char put_char);

void USART_PutString(char *put_string, uint8_t string_length);
/****************/
void ADC_config(void);

void DMA_config(void);

/*-----------------------------------------------------------*/
//INTERRUPÇOES EXTERNAS(JOYSTICK) E USART2
void EXTI0_IRQHandler(void) // Left (L)
{
	EXTI_ClearITPendingBit(EXTI_Line0);
	//static para guardar valor entre ciclos
	static	TickType_t xnew, xold=0;
	xnew = xTaskGetTickCountFromISR();

	if((xnew-xold)>100)
	{
		char c='L';

		static BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR( xQueueJoystick, &c, &pxHigherPriorityTaskWoken);
		if( pxHigherPriorityTaskWoken == pdTRUE )
			taskYIELD(); /* Forces a context switch before exit the ISR. */

		xold = xnew;//atualiza variável
	}
}

void EXTI2_IRQHandler(void) // Enter (E)
{
	EXTI_ClearITPendingBit(EXTI_Line2);
	static	TickType_t xnew, xold=0;
	xnew = xTaskGetTickCountFromISR();

	if((xnew-xold)>100)
	{
		char c='E';

		static BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR( xQueueJoystick, &c, &pxHigherPriorityTaskWoken);
		if( pxHigherPriorityTaskWoken == pdTRUE )
			taskYIELD(); /* Forces a context switch before exit the ISR. */

		xold = xnew;
	}
}

void EXTI4_IRQHandler(void) // Right(R)
{
	EXTI_ClearITPendingBit(EXTI_Line4);
	static	TickType_t xnew, xold=0;
	xnew = xTaskGetTickCountFromISR();

	if((xnew-xold)>100)
	{
		char c='R';

		static BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR( xQueueJoystick, &c, &pxHigherPriorityTaskWoken);
		if( pxHigherPriorityTaskWoken == pdTRUE )
			taskYIELD(); /* Forces a context switch before exit the ISR. */
		xold = xnew;
	}
}


void USART2_IRQHandler(void)
{
/*//ficha 7
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == pdTRUE)
	{
		char RxData=USART_ReceiveData(USART2);

		static BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR( xQueue, &RxData, &pxHigherPriorityTaskWoken);
		if( pxHigherPriorityTaskWoken == pdTRUE )
			taskYIELD(); //Forces a context switch before exit the ISR.

		USART_ClearITPendingBit(USART2, USART_IT_RXNE); //Limpar a flag da interrupção
		USART_ClearFlag(USART2,USART_FLAG_RXNE); //Limpar flag de receção
	}
*/
	if(USART_GetITStatus(USART2, USART_IT_TXE) == pdTRUE)
	{
		char TXData;
		BaseType_t xTaskWokenByReceive = pdFALSE;

		if(xQueueReceiveFromISR( xUSART_Send, &TXData,  &xTaskWokenByReceive )==pdFALSE)
			//desativa a interrupção quando acabam caracteres da fila de mensagens
			USART_ITConfig( USART2, USART_IT_TXE, DISABLE );

		else USART_SendData(USART2,TXData);	//envia por USART

		if( xTaskWokenByReceive != pdFALSE )
			taskYIELD ();

		//USART_ClearITPendingBit(USART2, USART_IT_TXE); //Limpar a flag da interrupção; ficha 7
		//USART_ClearFlag(USART2,USART_FLAG_TXE); //Limpar flag de receção; ficha 7
	}
}

/*-----------------------------------------------------------*/
int main( void )
{
	/*Setup the hardware, RCC, GPIO, etc...*/
	prvSetupRCC();
	lcd_init();
	prvSetupGPIO();
	USART_init(115200,25);	//driver USART: baudrate,buffer_depth
	joystick_setup();
	ADC_config();
	DMA_config();
	NVIC_config();

	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);

	//xQueue = xQueueCreate( 10, sizeof( char ) ); //Fila de Mensagens USART_GetChar para ficha 7
	xQueueADC=xQueueCreate( 5, sizeof( ADC_Struct ) );//Fila de mensagens da estrutura ADC
	xQueueJoystick=xQueueCreate( 10, sizeof( char ) );//Fila de mensagens Joystick
	/*Tamanho de stack configuradas para cada tarefa com base no returno do API uxTaskGetStackHighWaterMark()
	 * uxTaskGetStackHighWaterMark() returna o valor de stack livre numa dada tarefa
	 */
	//Create the tasks
	//tarefa de piscar LED
	xTaskCreate( prvLED, "LED", configMINIMAL_STACK_SIZE, NULL, mainFLASH_TASK_PRIORITY, &HandleTask1 );
	//tarefa do display LCD
	xTaskCreate( prvLcdTask, "LCD", configMINIMAL_STACK_SIZE * 2, NULL, mainLCD_TASK_PRIORITY, &HandleTask2 );
	//Tarefa da Mensagem para USART
	xTaskCreate( prvMensagem, "Mensagem", configMINIMAL_STACK_SIZE * 2, NULL, mainMensagem_TASK_PRIORITY, &HandleTask3 );
	//Tarefa leitura do ADC
	xTaskCreate( prvADC_read, "ADC", configMINIMAL_STACK_SIZE, NULL, mainADC_TASK_PRIORITY, &HandleTask4 );

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was not enough heap space to create the idle task. */
	return 0;
}

/*-----------------------------------------------------------*/

static void prvLED( void *pvParameters )
{
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();

	while(1)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_6, (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_6)));
		vTaskDelayUntil( &xLastExecutionTime, mainFLASH_DELAY );//1s
	}
}
/*-----------------------------------------------------------*/

/* Example task to present characteres in ther display. */
static void prvLcdTask( void *pvParameters )
{
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
	char bufferA[50],bufferB[50],bufferC[50],bufferD[50]; //buffer para linhas do display
	char menu;

	lcd_draw_fillrect(0,0,127,63,BLACK);
	lcd_draw_string( 47, 0 , "MENU", WHITE , 1 );
	lcd_draw_string( 0, 20 , "Left-> Estado Sistema", WHITE , 1 );
	lcd_draw_string( 0, 30 , "Right-> Stack tarefa", WHITE , 1 );
	lcd_draw_string( 0, 40 , "Enter-> Menu", WHITE , 1 );

	while(1)
	{
		menu=joystick_read();
		switch(menu)
		{
			case 'L':
				lcd_draw_fillrect(0,0,127,63,BLACK);
				lcd_draw_string(30, 0,"Estado Sistema", WHITE, 1);
				sprintf(bufferA, "Nr ticks = %ld", xTaskGetTickCount());
				lcd_draw_string(0, 16, bufferA, WHITE, 1);
				sprintf(bufferB, "Nr tarefas = %ld", uxTaskGetNumberOfTasks());
				lcd_draw_string(0, 25, bufferB, WHITE, 1);
				break;
			case 'E':
				lcd_draw_fillrect(0,0,127,63,BLACK);
				lcd_draw_string( 47, 0 , "MENU", WHITE , 1 );
				lcd_draw_string( 0, 20 , "Left-> Estado Sistema", WHITE , 1 );
				lcd_draw_string( 0, 30 , "Right-> Stack tarefa", WHITE , 1 );
				lcd_draw_string( 0, 40 , "Enter-> Menu", WHITE , 1 );
				break;
			case 'R':
				lcd_draw_fillrect(0,0,127,63,BLACK);
				lcd_draw_string(30, 0,"Stack por tarefa", WHITE, 1);
				sprintf(bufferA, "1: %s: %ld", pcTaskGetName(HandleTask1),uxTaskGetStackHighWaterMark(HandleTask1));
				lcd_draw_string(0, 16, bufferA, WHITE, 1);
				sprintf(bufferB, "2: %s: %ld", pcTaskGetName(HandleTask2),uxTaskGetStackHighWaterMark(HandleTask2));
				lcd_draw_string(0, 25, bufferB, WHITE, 1);
				sprintf(bufferC, "3: %s: %ld", pcTaskGetName(HandleTask3),uxTaskGetStackHighWaterMark(HandleTask3));
				lcd_draw_string(0, 34, bufferC, WHITE, 1);
				sprintf(bufferD, "4: %s: %ld", pcTaskGetName(HandleTask4),uxTaskGetStackHighWaterMark(HandleTask4));
				lcd_draw_string(0, 43, bufferD, WHITE, 1);
				break;
		}
		display();
		vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) 400 / portTICK_PERIOD_MS );//400ms refresh
	}
}

static void prvADC_read(void *pvParameters)//Tarefa de leitura do ADC
{
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
	ADC_Struct Structsend;

	while(1)
	{
		Structsend.x = ADC_values[0];
		Structsend.y = ADC_values[1];
		Structsend.z = ADC_values[2];
		xQueueSendToBack( xQueueADC, ( void * ) &Structsend, ( TickType_t ) portMAX_DELAY );
		vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) 200 / portTICK_PERIOD_MS );	//200ms
	}
}

static void prvMensagem(void *pvParameters)
{
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
	ADC_Struct Structreceive;
	char buffer[60];
	float xg, yg, zg;

	while(1)
	{
		xQueueReceive( xQueueADC, &Structreceive, ( TickType_t ) portMAX_DELAY );
		/*
		 * pelo datasheet: 3.3 V -> 330 mV/g
		 * 12 bit resolução -> 3.3 V / (2^12) = 0.0008 V = 0.8 mV
		 * 0.8 mV / 330 mV/g = 0.0024 g por cada step do ADC
		*/
		xg = (float) (Structreceive.x * 0.0024) - 5;	//5: offset de calibração;
		yg = (float) (Structreceive.y * 0.0024) - 5;	//em USART apresenta valores
		zg = (float) (Structreceive.z * 0.0024) - 5;	//entre -1 e 1

		sprintf(buffer,"%.4f %.4f %.4f\r\n",xg,yg,zg);//.4 : arredondado a 4 casas decimais
		USART_PutString(buffer,strlen(buffer));
		vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) 200 / portTICK_PERIOD_MS ); //200ms
	}
}
/*-----------------------------------------------------------*/
static void prvSetupRCC( void )
{
	/* RCC configuration - 72 MHz */
	ErrorStatus HSEStartUpStatus;

	RCC_DeInit();
	/*Enable the HSE*/
	RCC_HSEConfig(RCC_HSE_ON);
	/* Wait untill HSE is ready or time out */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable The Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		/* 72 MHZ - 2 wait states */
		FLASH_SetLatency(FLASH_Latency_2);

		/* No division HCLK = SYSCLK = 72 MHz*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		/* PCLK1 = HCLK/2 (36MHz) */
		RCC_PCLK1Config(RCC_HCLK_Div2);
		/* PCLK2 = HCLK (72MHz)*/
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* Use PLL with HSE = 12 MHz (12 MHz * 6 = 72 MHz) */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
		/* Enable the PLL */
		RCC_PLLCmd(ENABLE);
		/* Wait for PLL ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET );

		/* Select the PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		/* Wait until PLL is used as system clock */
		while( RCC_GetSYSCLKSource() != 0x08 );
	}
	else
	{
		/* HSE error? No further action */
		while(1);
	}
}
/*-----------------------------------------------------------*/

static void prvSetupGPIO( void )
{
	/* GPIO configuration */
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB  | RCC_APB2Periph_GPIOC| RCC_APB2Periph_ADC1, ENABLE );
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//LED
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*GPIO ADC
	* X -> PC4 -> ADC1_14
	* Y -> PB1 -> ADC1_9
	* Z -> PC5 -> ADC1_15
	*/
	GPIO_InitTypeDef ADC_B;
	ADC_B.GPIO_Pin = GPIO_Pin_1;
	ADC_B.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &ADC_B);

	GPIO_InitTypeDef ADC_C;
	ADC_C.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	ADC_C.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &ADC_C);
}

void ADC_config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	//ADC1 independente de ADC2
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//conversão multi-canal
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	//conversão contínua
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	//sem trigger externo
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	//12 bits alinhados á direita
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//conversão de três canais
	ADC_InitStructure.ADC_NbrOfChannel = 3;
	ADC_Init(ADC1, &ADC_InitStructure);
	//configurar cada canal
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_41Cycles5);	//41.5 ciclos
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 3, ADC_SampleTime_41Cycles5);
	//Ativa ADC1
	ADC_Cmd(ADC1, ENABLE);
	//ativa DMA para ADC
	ADC_DMACmd(ADC1, ENABLE);

	//calibrar ADC
	//reset registo de calibração
	ADC_ResetCalibration(ADC1);
	//espera fim do reset da calibração
	while(ADC_GetResetCalibrationStatus(ADC1));
	//começa calibração
	ADC_StartCalibration(ADC1);
	//espera fim da calibração
	while(ADC_GetCalibrationStatus(ADC1));
}

void DMA_config(void)
{
	//reset
	DMA_DeInit(DMA1_Channel1);
	DMA_InitTypeDef DMA_InitStructure;
	//endereço perfiférico ADC1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) (&(ADC1->DR));
	//Endereço memória
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) ADC_values;
	//Origem dos dados: ADC1
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	//tamanho do buffer a transferir
	DMA_InitStructure.DMA_BufferSize = ADC_buffer;
	//incremento do endereço da fonte desativado
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//incremento automático da memória do destino ativado
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//tamanho dos dados da fonte: HalfWord -> 16 bits
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	//tamanho dos dados da origem: HalfWord -> 16 bits
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	//modo circular
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	//prioridade média
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	//Transferência de dados entre regiões de memória desativada
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
}

/*-----------------------------------------------------------*/
void USART_init (uint32_t baudrate, uint16_t buffer_depth)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOA clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE );

	/* USART Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;//Pino TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;//Pino RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	/* Configure the USART2 */
	USART_Init(USART2, &USART_InitStructure);
	/* Enable the USART2 */
	USART_Cmd(USART2, ENABLE);

	xUSART_Send = xQueueCreate( buffer_depth, sizeof( char ) ); //Fila de Mensagens USART_Send
}

void NVIC_config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// INTERRUPÇOES EXTERNAS JOYSTICK
	EXTI_InitTypeDef EXTI_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);//LEFT
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	/*
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);// UP
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
 	 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);//ENTER
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	/*
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3);// DOWN
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);// RIGHT
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// INTERRUPÇAO USART2
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable da interrupção de receção da USART2 */
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE );//Flag de receber dados; ficha 7
}

/*-----------------------------------------------------------*/
void joystick_setup(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB  | RCC_APB2Periph_GPIOC, ENABLE );

	//CONFIG JOYSTICK; necessitam de ser todos configurados

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;  // UP
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4; // RIGHT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3; // DOWN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0; //LEFT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2; //ENTER
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

char joystick_read(void)//leitura do JOYSTICK
{
	static char c;

		if(xQueueReceive( xQueueJoystick, &c, ( TickType_t ) 20 / portTICK_PERIOD_MS )==pdTRUE);
		return c;
}

/*-----------------------------------------------------------*/
char prvUSART_GetChar()	//exclusivo ficha 7
{
	char c;

	if( xQueue != 0 )
		xQueueReceive( xQueue, &c, ( TickType_t ) portMAX_DELAY );
	return c;
}


/*-----------------------------------------------------------*/

void prvUSART_PutChar(char put_char) //exclusivo ficha 7
{
	if( xUSART_Send != 0 )
	{
		xQueueSendToBack( xUSART_Send, ( void * ) &put_char, ( TickType_t ) portMAX_DELAY );

		/* Failed to post the message within 10 ticks. The message queue is full. */

		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	}
}

/*-----------------------------------------------------------*/
void USART_PutString(char *put_string, uint8_t string_length)
{
	uint8_t i=0;
	for(i=0; i<string_length; i++)
	{
		xQueueSendToBack( xUSART_Send, ( void * ) &put_string[i], ( TickType_t ) portMAX_DELAY );
		//prvUSART_PutChar(put_string[i]); //ficha 7
	}
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

/*-----------------------------------------------------------*/
