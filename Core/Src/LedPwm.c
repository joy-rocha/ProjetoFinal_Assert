/******************************************************************************/
/**
 * @file LedPwm.c
 * @brief Gerenciador logico das intensidades dos LEDs usando PWM.
 * @addtogroup LedPwm
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "LedPwm.h"
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Valor limite maximo para o ciclo de trabalho do LED (porcentagem)
#define dLEDPWM_MAX_DUTY                                                       100

/// Valor inicial desligado
#define dLEDPWM_DUTY_ZERO                                                      0

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura local para rastrear as variaveis de estado dos LEDs
static struct
{
    u8 selectedChannel;
    u8 dutyLed1;
    u8 dutyLed2;
    u8 dutyLed3;
} ledState;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa as variaveis logicas e desliga todos os LEDs na Bsp.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_Init(void)
{
    ledState.selectedChannel = dLEDPWM_CHANNEL_1;
    ledState.dutyLed1 = dLEDPWM_DUTY_ZERO;
    ledState.dutyLed2 = dLEDPWM_DUTY_ZERO;
    ledState.dutyLed3 = dLEDPWM_DUTY_ZERO;

    Bsp_PwmSetDuty(dLEDPWM_CHANNEL_1, dLEDPWM_DUTY_ZERO);
    Bsp_PwmSetDuty(dLEDPWM_CHANNEL_2, dLEDPWM_DUTY_ZERO);
    Bsp_PwmSetDuty(dLEDPWM_CHANNEL_3, dLEDPWM_DUTY_ZERO);
}

/******************************************************************************/
/** @brief Altera qual e o LED ativamente controlado pelo potenciometro.
 * @param channel: O canal do LED (1, 2 ou 3) de acordo com os defines.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_SetSelectedLed(u8 channel)
{
    if((channel >= dLEDPWM_CHANNEL_1) && (channel <= dLEDPWM_CHANNEL_3))
    {
        ledState.selectedChannel = channel;
    }
}

/******************************************************************************/
/** @brief Retorna o canal do LED atualmente selecionado para alteracao.
 * @param Nenhum.
 * @retval O canal atual de 1 a 3.
 ******************************************************************************/
u8 LedPwm_GetSelectedLed(void)
{
    return ledState.selectedChannel;
}

/******************************************************************************/
/** @brief Atualiza a intensidade do LED selecionado se o sistema estiver ativo.
 * Esta funcao deve ser chamada apos receber o novo valor convertido do Sampler.
 * @param percentage: Valor do duty cycle desejado (0 a 100).
 * @param isSystemActive: Booleano que indica se a atuacao esta congelada ou nao.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_Update(u8 percentage, bool isSystemActive)
{
    if(isSystemActive == true)
    {
        if(percentage > dLEDPWM_MAX_DUTY)
        {
            percentage = dLEDPWM_MAX_DUTY;
        }

        if(ledState.selectedChannel == dLEDPWM_CHANNEL_1)
        {
            ledState.dutyLed1 = percentage;
            Bsp_PwmSetDuty(dLEDPWM_CHANNEL_1, ledState.dutyLed1);
        }
        else if(ledState.selectedChannel == dLEDPWM_CHANNEL_2)
        {
            ledState.dutyLed2 = percentage;
            Bsp_PwmSetDuty(dLEDPWM_CHANNEL_2, ledState.dutyLed2);
        }
        else if(ledState.selectedChannel == dLEDPWM_CHANNEL_3)
        {
            ledState.dutyLed3 = percentage;
            Bsp_PwmSetDuty(dLEDPWM_CHANNEL_3, ledState.dutyLed3);
        }
        else
        {
            /* Canal inexistente */
        }
    }
    /* Se o sistema estiver congelado (isSystemActive == false), nao faz nada.
       Os LEDs continuam com a intensidade ja enviada para a Bsp no passado. */
}

/******************************************************************************/
/** @brief Le a intensidade atual guardada na logica de um canal especifico.
 * Funcao muito util para compor o relatorio (printf) no terminal.
 * @param channel: O canal que se deseja inspecionar.
 * @retval Intensidade de 0 a 100% que o LED indicado possui no momento.
 ******************************************************************************/
u8 LedPwm_GetDuty(u8 channel)
{
    u8 currentDuty = dLEDPWM_DUTY_ZERO;

    if(channel == dLEDPWM_CHANNEL_1)
    {
        currentDuty = ledState.dutyLed1;
    }
    else if(channel == dLEDPWM_CHANNEL_2)
    {
        currentDuty = ledState.dutyLed2;
    }
    else if(channel == dLEDPWM_CHANNEL_3)
    {
        currentDuty = ledState.dutyLed3;
    }

    return currentDuty;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
