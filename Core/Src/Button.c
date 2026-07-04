/******************************************************************************/
/**
 * @file Button.c
 * @brief Gerenciador do botao, responsavel pelo debounce e estado do sistema.
 * @addtogroup Button
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Button.h"
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura local para controle da maquina de estados do botao
static struct
{
    bool isSystemActive;
    bool isDebouncing;
} button;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa as variaveis do botao. O sistema comeca ativo (ON).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Button_Init(void)
{
    button.isSystemActive = true;
    button.isDebouncing = false;
}

/******************************************************************************/
/** @brief Atualiza a maquina de estados do botao verificando as flags da Bsp.
 * Deve ser chamada continuamente no laco principal (while 1).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Button_Update(void)
{
    /* Passo 1: Verifica se houve uma interrupcao de hardware do botao (EXTI) */
    if(Bsp_GetButtonExtiFlag() == true)
    {
        Bsp_ClearButtonExtiFlag();

        /* Se ja nao estiver no periodo de espera (debounce), inicia a contagem */
        if(button.isDebouncing == false)
        {
            button.isDebouncing = true;
            Bsp_StartDebounceTimer();
        }
    }

    /* Passo 2: Verifica se o temporizador de debounce chegou ao fim */
    if(Bsp_GetDebounceTimeoutFlag() == true)
    {
        Bsp_ClearDebounceTimeoutFlag();

        /* Valida o clique e alterna o estado global do sistema (ON/OFF) */
        button.isSystemActive = !button.isSystemActive;

        /* Libera a maquina de estados para capturar o proximo clique */
        button.isDebouncing = false;
    }
}

/******************************************************************************/
/** @brief Retorna o estado atual do sistema (se esta processando ou congelado).
 * @param Nenhum.
 * @retval true se o sistema estiver ativo (ON), false se congelado (OFF).
 ******************************************************************************/
bool Button_IsSystemActive(void)
{
    return button.isSystemActive;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
