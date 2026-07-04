/******************************************************************************/
/**
 * @file main.c
 * @brief Arquivo principal de integracao do Projeto Final.
 * @addtogroup Main
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.h"
#include "Sampler.h"
#include "LedPwm.h"
#include "SerialCmd.h"
#include "Button.h"

/*******************************************************************************
 * FUNCOES PRINCIPAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief Ponto de entrada do programa e laco principal (Super Loop).
 * @param Nenhum.
 * @retval Int (padrao C).
 ******************************************************************************/
int main(void)
{
    bool systemActiveState = true;
    u8 currentPercentage = 0;

    /* Inicializacao da camada de hardware e modulos de aplicacao */
    Bsp_Init();
    Sampler_Init();
    LedPwm_Init();
    SerialCmd_Init();
    Button_Init();

    /* Laco infinito nao bloqueante */
    while(1)
    {
        /* Rotinas de atualizacao e aquisicao */
        Sampler_Update();
        Button_Update();
        SerialCmd_ProcessRx();

        /* Coleta de variaveis de estado */
        systemActiveState = Button_IsSystemActive();
        currentPercentage = Sampler_GetFilteredPercentage();

        /* Atuacao nos LEDs com base no estado do sistema */
        LedPwm_Update(currentPercentage, systemActiveState);

        /* Relatorio temporal no terminal (a cada 1 segundo) */
        if(Bsp_GetPrintTimeoutFlag() == true)
        {
            Bsp_ClearPrintTimeoutFlag();
            SerialCmd_PrintStatus(systemActiveState);
        }
    }

    return 0;
}

/** @} */
