/******************************************************************************/
/**
 * @file Sampler.c
 * @brief Modulo responsavel pela aquisicao, filtragem e conversao dos dados.
 * @addtogroup Sampler
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Sampler.h"
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Quantidade de amostras para o calculo da media movel
#define dSAMPLER_MAX_SAMPLES                                                   100

/// Valor maximo do conversor AD de 12 bits
#define dSAMPLER_ADC_MAX_VALUE                                                 4095

/// Valor maximo em porcentagem
#define dSAMPLER_MAX_PERCENTAGE                                                100

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estrutura local de controle do amostrador
static struct
{
    u32 accumulator;
    u16 sampleCount;
    u8 currentPercentage;
} sampler;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa as variaveis do modulo de amostragem.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Sampler_Init(void)
{
    sampler.accumulator = 0;
    sampler.sampleCount = 0;
    sampler.currentPercentage = 0;
}

/******************************************************************************/
/** @brief Executa a rotina de verificacao e aquisicao de amostras.
 * Deve ser chamada de forma continua no laco principal (while 1).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
bool Sampler_Update(void)
{
    u32 average = 0;
    bool newAverageReady = false; /* Criamos a flag local */

    if(Bsp_GetSamplingFlag() == true)
    {
        Bsp_ClearSamplingFlag();

        sampler.accumulator += (u32)Bsp_AdcRead();
        sampler.sampleCount++;

        if(sampler.sampleCount >= dSAMPLER_MAX_SAMPLES)
        {
            average = sampler.accumulator / dSAMPLER_MAX_SAMPLES;
            sampler.currentPercentage = (u8)((average * dSAMPLER_MAX_PERCENTAGE) / dSAMPLER_ADC_MAX_VALUE);

            sampler.accumulator = 0;
            sampler.sampleCount = 0;

            newAverageReady = true; /* Avisa que a media esta pronta! */
        }
    }

    return newAverageReady; /* Retorna a flag para a main */
}

/******************************************************************************/
/** @brief Retorna o valor filtrado mais recente na escala de porcentagem.
 * @param Nenhum.
 * @retval Valor de 0 a 100 representando a leitura do ADC.
 ******************************************************************************/
u8 Sampler_GetFilteredPercentage(void)
{
    return sampler.currentPercentage;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
