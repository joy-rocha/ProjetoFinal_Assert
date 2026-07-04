/******************************************************************************/
/**
 * @file Bsp.c
 * @addtogroup Bsp
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
#define dTIM_SAMPLING_PRESCALER                                                83
#define dTIM_SAMPLING_PERIOD                                                   4999
#define dTIM_DEBOUNCE_PRESCALER                                                839
#define dTIM_DEBOUNCE_PERIOD                                                   2999
#define dTIM_PWM_PRESCALER                                                     83
#define dTIM_PWM_PERIOD                                                        99
#define dUART_BAUDRATE                                                         115200
#define dADC_TIMEOUT_VAL                                                       10
#define dPWM_MAX_DUTY                                                          100
#define dADC_CHANNEL_CONFIG                                                    ADC_CHANNEL_0

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura contendo todas as variaveis locais do modulo Bsp
static struct
{
    ADC_HandleTypeDef hadc1;
    TIM_HandleTypeDef htimSampling;
    TIM_HandleTypeDef htimDebounce;
    TIM_HandleTypeDef htimPwm;
    UART_HandleTypeDef huart3;
    bool samplingFlag;
    bool buttonExtiFlag;
    bool debounceTimeoutFlag;
    bool uartRxFlag;
    u8 uartRxByte;
} bsp;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
static void Bsp_GpioInit(void);
static void Bsp_AdcInit(void);
static void Bsp_TimSamplingInit(void);
static void Bsp_TimDebounceInit(void);
static void Bsp_TimPwmInit(void);
static void Bsp_Uart3Init(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializacao geral de todo o hardware do sistema.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void)
{
    Bsp_GpioInit();
    Bsp_AdcInit();
    Bsp_TimSamplingInit();
    Bsp_TimDebounceInit();
    Bsp_TimPwmInit();
    Bsp_Uart3Init();

    HAL_TIM_Base_Start_IT(&bsp.htimSampling);
    HAL_UART_Receive_IT(&bsp.huart3, &bsp.uartRxByte, 1);
}

/******************************************************************************/
/** @brief Retorna se a flag de amostragem de 5 milissegundos ocorreu.
 * @param Nenhum.
 * @retval Estado da flag de amostragem (true ou false).
 ******************************************************************************/
bool Bsp_GetSamplingFlag(void)
{
    return bsp.samplingFlag;
}

/******************************************************************************/
/** @brief Limpa a flag de amostragem de 5 milissegundos.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearSamplingFlag(void)
{
    bsp.samplingFlag = false;
}

/******************************************************************************/
/** @brief Realiza a leitura do conversor analogico digital por polling.
 * @param Nenhum.
 * @retval Valor bruto lido do ADC de 12 bits.
 ******************************************************************************/
u16 Bsp_AdcRead(void)
{
    u16 adcVal = 0;

    HAL_ADC_Start(&bsp.hadc1);

    if((HAL_ADC_PollForConversion(&bsp.hadc1, dADC_TIMEOUT_VAL) == HAL_OK))
    {
        adcVal = (u16)HAL_ADC_GetValue(&bsp.hadc1);
    }

    HAL_ADC_Stop(&bsp.hadc1);

    return adcVal;
}

/******************************************************************************/
/** @brief Altera a intensidade do LED selecionado via PWM.
 * @param channel: Canal do LED (dBSP_LED_CHANNEL_1, 2 ou 3).
 * @param duty: Ciclo de trabalho de 0 a 100.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_PwmSetDuty(u8 channel, u8 duty)
{
    u8 dutyApplied = duty;

    if((dutyApplied > dPWM_MAX_DUTY))
    {
        dutyApplied = dPWM_MAX_DUTY;
    }

    if((channel == dBSP_LED_CHANNEL_1))
    {
        __HAL_TIM_SET_COMPARE(&bsp.htimPwm, TIM_CHANNEL_1, dutyApplied);
    }
    else if((channel == dBSP_LED_CHANNEL_2))
    {
        __HAL_TIM_SET_COMPARE(&bsp.htimPwm, TIM_CHANNEL_2, dutyApplied);
    }
    else if((channel == dBSP_LED_CHANNEL_3))
    {
        __HAL_TIM_SET_COMPARE(&bsp.htimPwm, TIM_CHANNEL_3, dutyApplied);
    }
    else
    {
        /* Canal invalido, nenhuma acao realizada */
    }
}

/******************************************************************************/
/** @brief Retorna se a flag de interrupcao externa do botao ocorreu.
 * @param Nenhum.
 * @retval Estado da flag do botao (true ou false).
 ******************************************************************************/
bool Bsp_GetButtonExtiFlag(void)
{
    return bsp.buttonExtiFlag;
}

/******************************************************************************/
/** @brief Limpa a flag de interrupcao externa do botao.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearButtonExtiFlag(void)
{
    bsp.buttonExtiFlag = false;
}

/******************************************************************************/
/** @brief Inicia o temporizador dedicado para o debounce nao bloqueante.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_StartDebounceTimer(void)
{
    __HAL_TIM_SET_COUNTER(&bsp.htimDebounce, 0);
    HAL_TIM_Base_Start_IT(&bsp.htimDebounce);
}

/******************************************************************************/
/** @brief Retorna se o temporizador de debounce finalizou a contagem.
 * @param Nenhum.
 * @retval Estado da flag de timeout do debounce (true ou false).
 ******************************************************************************/
bool Bsp_GetDebounceTimeoutFlag(void)
{
    return bsp.debounceTimeoutFlag;
}

/******************************************************************************/
/** @brief Limpa a flag de timeout do temporizador de debounce.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearDebounceTimeoutFlag(void)
{
    bsp.debounceTimeoutFlag = false;
}

/******************************************************************************/
/** @brief Verifica se existem dados recebidos na USART3.
 * @param Nenhum.
 * @retval Status indicando se ha novos dados (true ou false).
 ******************************************************************************/
bool Bsp_UartRxHasData(void)
{
    return bsp.uartRxFlag;
}

/******************************************************************************/
/** @brief Le o byte recebido pela USART3 e limpa a flag de recepcao.
 * @param Nenhum.
 * @retval Byte recebido da serial.
 ******************************************************************************/
u8 Bsp_UartRxReadByte(void)
{
    u8 receivedByte = bsp.uartRxByte;
    bsp.uartRxFlag = false;
    return receivedByte;
}

/******************************************************************************/
/** @brief Redirecionamento do printf para a USART3 da HAL.
 * @param file: Descriptor do arquivo.
 * @param ptr: Ponteiro para o buffer de caracteres.
 * @param len: Quantidade de caracteres a transmitir.
 * @retval Quantidade de caracteres transmitidos.
 ******************************************************************************/
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&bsp.huart3, (uint8_t *)ptr, len, dADC_TIMEOUT_VAL);
    return len;
}

/******************************************************************************/
/** @brief Callback global do estouro de periodo dos temporizadores HAL.
 * @param htim: Ponteiro para o manipulador do temporizador.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if((htim->Instance == TIM6))
    {
        bsp.samplingFlag = true;
    }
    else if((htim->Instance == TIM7))
    {
        HAL_TIM_Base_Stop_IT(&bsp.htimDebounce);
        bsp.debounceTimeoutFlag = true;
    }
    else
    {
        /* Outros temporizadores */
    }
}

/******************************************************************************/
/** @brief Callback global de interrupcao externa dos pinos GPIO.
 * @param gpioPin: Pino que gerou a interrupcao externa.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t gpioPin)
{
    if((gpioPin == GPIO_PIN_0))
    {
        bsp.buttonExtiFlag = true;
    }
}

/******************************************************************************/
/** @brief Callback global de finalizacao de recepcao da UART.
 * @param huart: Ponteiro para o manipulador da UART.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if((huart->Instance == USART3))
    {
        bsp.uartRxFlag = true;
        HAL_UART_Receive_IT(&bsp.huart3, &bsp.uartRxByte, 1);
    }
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializacao dos pinos de entrada e saida digital (GPIO).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_GpioInit(void)
{
    GPIO_InitTypeDef gpioConfig = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configuracao do botao da interrupcao externa EXTI0 no pino PC0 */
    gpioConfig.Pin = GPIO_PIN_0;
    gpioConfig.Mode = GPIO_MODE_IT_RISING;
    gpioConfig.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &gpioConfig);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/******************************************************************************/
/** @brief Inicializacao do periferico Conversor Analogico Digital ADC1.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_AdcInit(void)
{
    ADC_ChannelConfTypeDef channelConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    bsp.hadc1.Instance = ADC1;
    bsp.hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    bsp.hadc1.Init.ScanConvMode = DISABLE;
    bsp.hadc1.Init.ContinuousConvMode = DISABLE;
    bsp.hadc1.Init.DiscontinuousConvMode = DISABLE;
    bsp.hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    bsp.hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    bsp.hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    bsp.hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&bsp.hadc1);

    channelConfig.Channel = dADC_CHANNEL_CONFIG;
    channelConfig.Rank = 1;
    channelConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    HAL_ADC_ConfigChannel(&bsp.hadc1, &channelConfig);
}

/******************************************************************************/
/** @brief Inicializacao do temporizador de amostragem de 5 milissegundos.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_TimSamplingInit(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();

    bsp.htimSampling.Instance = TIM6;
    bsp.htimSampling.Init.Prescaler = dTIM_SAMPLING_PRESCALER;
    bsp.htimSampling.Init.CounterMode = TIM_COUNTERMODE_UP;
    bsp.htimSampling.Init.Period = dTIM_SAMPLING_PERIOD;
    bsp.htimSampling.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&bsp.htimSampling);

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

/******************************************************************************/
/** @brief Inicializacao do temporizador dedicado de debounce do botao.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_TimDebounceInit(void)
{
    __HAL_RCC_TIM7_CLK_ENABLE();

    bsp.htimDebounce.Instance = TIM7;
    bsp.htimDebounce.Init.Prescaler = dTIM_DEBOUNCE_PRESCALER;
    bsp.htimDebounce.Init.CounterMode = TIM_COUNTERMODE_UP;
    bsp.htimDebounce.Init.Period = dTIM_DEBOUNCE_PERIOD;
    bsp.htimDebounce.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&bsp.htimDebounce);

    HAL_NVIC_SetPriority(TIM7_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

/******************************************************************************/
/** @brief Inicializacao do temporizador de modulacao por largura de pulso.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_TimPwmInit(void)
{
    TIM_OC_InitTypeDef pwmConfig = {0};
    GPIO_InitTypeDef gpioConfig = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configuracao dos pinos PA6, PA7 para canais de PWM do TIM3 */
    gpioConfig.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpioConfig.Mode = GPIO_MODE_AF_PP;
    gpioConfig.Pull = GPIO_NOPULL;
    gpioConfig.Speed = GPIO_SPEED_FREQ_LOW;
    gpioConfig.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &gpioConfig);

    bsp.htimPwm.Instance = TIM3;
    bsp.htimPwm.Init.Prescaler = dTIM_PWM_PRESCALER;
    bsp.htimPwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    bsp.htimPwm.Init.Period = dTIM_PWM_PERIOD;
    bsp.htimPwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&bsp.htimPwm);

    pwmConfig.OCMode = TIM_OCMODE_PWM1;
    pwmConfig.Pulse = 0;
    pwmConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    pwmConfig.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&bsp.htimPwm, &pwmConfig, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&bsp.htimPwm, &pwmConfig, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&bsp.htimPwm, &pwmConfig, TIM_CHANNEL_3);

    HAL_TIM_PWM_Start(&bsp.htimPwm, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&bsp.htimPwm, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&bsp.htimPwm, TIM_CHANNEL_3);
}

/******************************************************************************/
/** @brief Inicializacao da interface de comunicacao serial USART3.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_Uart3Init(void)
{
    GPIO_InitTypeDef gpioConfig = {0};

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configuracao dos pinos PC10 (TX) e PC11 (RX) para USART3 */
    gpioConfig.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpioConfig.Mode = GPIO_MODE_AF_PP;
    gpioConfig.Pull = GPIO_PULLUP;
    gpioConfig.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioConfig.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &gpioConfig);

    bsp.huart3.Instance = USART3;
    bsp.huart3.Init.BaudRate = dUART_BAUDRATE;
    bsp.huart3.Init.WordLength = UART_WORDLENGTH_8B;
    bsp.huart3.Init.StopBits = UART_STOPBITS_1;
    bsp.huart3.Init.Parity = UART_PARITY_NONE;
    bsp.huart3.Init.Mode = UART_MODE_TX_RX;
    bsp.huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    bsp.huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&bsp.huart3);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

/** @} */
