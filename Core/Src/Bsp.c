/******************************************************************************/
/**
 * @file Bsp.c
 * @brief Abstracao de hardware com PWM manual e UART por polling.
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
/// Tempo limite para a leitura do ADC em milissegundos
#define dBSP_ADC_TIMEOUT_MS                                                    10

/// Valor maximo de porcentagem para o duty cycle do PWM
#define dBSP_PWM_MAX_DUTY                                                      100

/// Tempo em milissegundos para o debounce manual
#define dBSP_DEBOUNCE_TIME_MS                                                  50

/// Tempo limite zero para leitura serial nao bloqueante
#define dBSP_UART_TIMEOUT_ZERO                                                 0

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura local com flags de hardware
static struct
{
    bool samplingFlag;
    bool buttonExtiFlag;

    /* Variaveis para o debounce manual via SysTick */
    bool isDebounceRunning;
    u32 debounceStartTime;
} bsp;

/* Importacao dos handles gerados automaticamente pelo STM32CubeMX no main.c */
extern TIM_HandleTypeDef htim6;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart3;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicia os perifericos utilizando as configuracoes do CubeMX.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void)
{
    bsp.samplingFlag = false;
    bsp.buttonExtiFlag = false;
    bsp.isDebounceRunning = false;

    /* 1. Inicia o Timer 6 por interrupcao (Gatilho da Amostragem) */
    HAL_TIM_Base_Start_IT(&htim6);

    /* 2. Inicia o Hardware PWM de forma manual (Sem interrupcao) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    /* Nota: A UART por polling nao necessita de inicializacao adicional alem
       do que ja e feito no main.c pelo STM32CubeMX. */
}

/******************************************************************************/
/** @brief Retorna se a flag de amostragem do TIM6 ocorreu.
 * @param Nenhum.
 * @retval true ou false.
 ******************************************************************************/
bool Bsp_GetSamplingFlag(void)
{
    return bsp.samplingFlag;
}

/******************************************************************************/
/** @brief Limpa a flag de amostragem.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearSamplingFlag(void)
{
    bsp.samplingFlag = false;
}

/******************************************************************************/
/** @brief Realiza a leitura do conversor AD por polling (bloqueio rapido).
 * @param Nenhum.
 * @retval Valor bruto lido do ADC de 12 bits.
 ******************************************************************************/
u16 Bsp_AdcRead(void)
{
    u16 adcVal = 0;

    HAL_ADC_Start(&hadc1);

    if(HAL_ADC_PollForConversion(&hadc1, dBSP_ADC_TIMEOUT_MS) == HAL_OK)
    {
        adcVal = (u16)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    return adcVal;
}

/******************************************************************************/
/** @brief Altera a intensidade do LED selecionado manipulando o registrador.
 * @param channel: Canal do LED (dBSP_LED_CHANNEL_1, 2 ou 3).
 * @param duty: Ciclo de trabalho de 0 a 100%.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_PwmSetDuty(u8 channel, u8 duty)
{
    u8 dutyApplied = duty;

    if(dutyApplied > dBSP_PWM_MAX_DUTY)
    {
        dutyApplied = dBSP_PWM_MAX_DUTY;
    }

    if(channel == dBSP_LED_CHANNEL_1)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, dutyApplied);
    }
    else if(channel == dBSP_LED_CHANNEL_2)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, dutyApplied);
    }
    else if(channel == dBSP_LED_CHANNEL_3)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, dutyApplied);
    }
    else
    {
        /* Canal invalido */
    }
}

/******************************************************************************/
/** @brief Retorna se a interrupcao externa do botao ocorreu.
 * @param Nenhum.
 * @retval true ou false.
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
/** @brief Inicia o debounce manual baseado em SysTick.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_StartDebounceTimer(void)
{
    bsp.isDebounceRunning = true;
    bsp.debounceStartTime = HAL_GetTick();
}

/******************************************************************************/
/** @brief Verifica se o tempo de debounce ja foi atingido.
 * @param Nenhum.
 * @retval true se o tempo acabou, false caso contrario.
 ******************************************************************************/
bool Bsp_GetDebounceTimeoutFlag(void)
{
    if(bsp.isDebounceRunning == true)
    {
        if((HAL_GetTick() - bsp.debounceStartTime) >= dBSP_DEBOUNCE_TIME_MS)
        {
            bsp.isDebounceRunning = false;
            return true;
        }
    }
    return false;
}

/******************************************************************************/
/** @brief Limpa a flag de timeout de debounce.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearDebounceTimeoutFlag(void)
{
    bsp.isDebounceRunning = false;
}

/******************************************************************************/
/** @brief Verifica via polling se ha dados na USART3 sem bloquear a CPU.
 * Consulta diretamente a flag RXNE no hardware.
 * @param Nenhum.
 * @retval true se ha dados, false caso contrario.
 ******************************************************************************/
bool Bsp_UartRxHasData(void)
{
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) == SET)
    {
        return true;
    }
    return false;
}

/******************************************************************************/
/** @brief Realiza a leitura do byte recebido de forma nao bloqueante.
 * @param Nenhum.
 * @retval Byte recebido do terminal.
 ******************************************************************************/
u8 Bsp_UartRxReadByte(void)
{
    u8 receivedByte = 0;

    /* Usa a HAL com timeout zero (0) para ler instantaneamente o registrador */
    HAL_UART_Receive(&huart3, &receivedByte, 1, dBSP_UART_TIMEOUT_ZERO);

    return receivedByte;
}

/******************************************************************************/
/** @brief Redirecionamento padrao do printf para a USART3.
 * @param file, ptr, len: parametros padrao de syscall.
 * @retval Int com quantidade de caracteres transmitidos.
 ******************************************************************************/
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, dBSP_ADC_TIMEOUT_MS);
    return len;
}

/******************************************************************************/
/** @brief Callback global do CubeMX para estouro de Temporizador.
 * @param htim: Ponteiro da HAL.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        bsp.samplingFlag = true;
    }
}

/******************************************************************************/
/** @brief Callback global do CubeMX para Interrupcao Externa (EXTI).
 * @param gpioPin: Pino da interrupcao.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t gpioPin)
{
    /* Ajuste o numero do pino de acordo com o botao na sua placa */
    if(gpioPin == GPIO_PIN_13)
    {
        bsp.buttonExtiFlag = true;
    }
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
