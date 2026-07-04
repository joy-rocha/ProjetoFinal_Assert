/******************************************************************************/
/**
 * @file SerialCmd.c
 * @brief Gerenciador de comandos via USART3 e formatacao de saida no terminal.
 * @addtogroup SerialCmd
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "SerialCmd.h"
#include "Bsp.h"
#include "LedPwm.h"
#include "Sampler.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Tamanho maximo do buffer de recepcao para evitar estouro de memoria
#define dSERIALCMD_RX_BUFFER_SIZE                                              16

/// Strings exatas de comandos de selecao esperados pela especificacao
#define dSERIALCMD_CMD_LED1                                                    "LED1"
#define dSERIALCMD_CMD_LED2                                                    "LED2"
#define dSERIALCMD_CMD_LED3                                                    "LED3"

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura local de controle do interpretador de comandos seriais
static struct
{
    char rxBuffer[dSERIALCMD_RX_BUFFER_SIZE];
    u8 rxIndex;
} serialCmd;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
static void SerialCmd_ParseCommand(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa as variaveis e o buffer de recepcao da interface serial.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void SerialCmd_Init(void)
{
    serialCmd.rxIndex = 0;
    memset(serialCmd.rxBuffer, 0, dSERIALCMD_RX_BUFFER_SIZE);
}

/******************************************************************************/
/** @brief Processa dados recebidos da UART.
 * Deve ser chamada no laco principal. E responsavel por ler caractere por
 * caractere de forma nao bloqueante e armazenar ate identificar um \r ou \n.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void SerialCmd_ProcessRx(void)
{
    u8 byteReceived;

    while(Bsp_UartRxHasData() == true)
    {
        byteReceived = Bsp_UartRxReadByte();

        /* Verifica se o usuario pressionou Enter (carriage return ou newline) */
        if((byteReceived == '\r') || (byteReceived == '\n'))
        {
            if(serialCmd.rxIndex > 0)
            {
                /* Fecha a string corretamente e analisa o comando */
                serialCmd.rxBuffer[serialCmd.rxIndex] = '\0';
                SerialCmd_ParseCommand();

                /* Reinicia o indice para a proxima mensagem */
                serialCmd.rxIndex = 0;
            }
        }
        else
        {
            /* Evita buffer overflow adicionando limites na gravacao */
            if(serialCmd.rxIndex < (dSERIALCMD_RX_BUFFER_SIZE - 1))
            {
                serialCmd.rxBuffer[serialCmd.rxIndex] = (char)byteReceived;
                serialCmd.rxIndex++;
            }
        }
    }
}

/******************************************************************************/
/** @brief Realiza a impressao formatada do relatorio de sistema no terminal.
 * @param isSystemActive: Booleano para definir se o texto devera exibir ON/OFF.
 * @retval Nenhum.
 ******************************************************************************/
void SerialCmd_PrintStatus(bool isSystemActive)
{
    u8 currentPercentage = Sampler_GetFilteredPercentage();
    u8 dutyLed1 = LedPwm_GetDuty(dLEDPWM_CHANNEL_1);
    u8 dutyLed2 = LedPwm_GetDuty(dLEDPWM_CHANNEL_2);
    u8 dutyLed3 = LedPwm_GetDuty(dLEDPWM_CHANNEL_3);
    const char *stateString;

    if(isSystemActive == true)
    {
        stateString = "ON";
    }
    else
    {
        stateString = "OFF";
    }

    /* Imprime a string no formato exato solicitado pela especificacao usando o
       redirecionamento do printf criado na Bsp */
    printf("VALUE: %d%% || LED1: %d%% aceso || LED2: %d%% aceso || LED3: %d%% aceso || STATE: %s\r\n",
           currentPercentage, dutyLed1, dutyLed2, dutyLed3, stateString);
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief Interpreta o texto no buffer para alterar o comportamento do sistema.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void SerialCmd_ParseCommand(void)
{
    /* Faz a comparacao de strings para delegar a escolha do LED atuante */
    if(strncmp(serialCmd.rxBuffer, dSERIALCMD_CMD_LED1, 4) == 0)
    {
        LedPwm_SetSelectedLed(dLEDPWM_CHANNEL_1);
    }
    else if(strncmp(serialCmd.rxBuffer, dSERIALCMD_CMD_LED2, 4) == 0)
    {
        LedPwm_SetSelectedLed(dLEDPWM_CHANNEL_2);
    }
    else if(strncmp(serialCmd.rxBuffer, dSERIALCMD_CMD_LED3, 4) == 0)
    {
        LedPwm_SetSelectedLed(dLEDPWM_CHANNEL_3);
    }
    else
    {
        /* Comando nao reconhecido; e ignorado pelo sistema */
    }
}

/** @} */
