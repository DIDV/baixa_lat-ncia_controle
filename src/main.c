#include <p18f4550.h>
#include <timers.h>
#include "usb_functions.h"
#include <adc.h>
#include <pwm.h>

unsigned short contador = 0;            //Vari�vel para piscar um LED (RD2)
unsigned short sinal = 0;               //Vari�vel que indica o status do LED (RD2)

int processa_controle( char controle );
char recebe_dado_usb();
void config_pic();
unsigned short sensor_corrente_6v();
void limpa();
unsigned short sensor_temperatura_1();
unsigned short sensor_temperatura_2();
unsigned short sensor_temperatura_3();
void aciona_cooler();
void converte_corrente();
void converte_temperatura();

void high_isr(void);
#pragma code high_vector=0x08
 void interrupt_at_high_vector(void)
 {
  _asm GOTO high_isr _endasm
 }
#pragma code

#pragma interrupt high_isr
void high_isr(void)                 // Rotina para tratamento de interrup��o;
{
    sensor_corrente_6v();           //Rotina para medi��o de corrente;
    aciona_cooler();                //Rotina para medi��es de temperatura;
    if(contador == 10)              //Rotina para piscar um LED (RD2), informando que esta tudo certo;
    {
        if(sinal == 1)
        {
            PORTDbits.RD2 = 0;
            sinal = 0;
            contador = 0;
        }
        else
        {
            PORTDbits.RD2 = 1;
            sinal = 1;
            contador = 0;
        }
    }
    contador++;
    PIR1bits.TMR1IF = 0;            // Limpa flag da interrup��o;
}

/*
void low_isr(void);
#pragma code low_vector=0x18
 void interrupt_at_low_vector(void)
 {
  _asm GOTO low_isr _endasm
 }
#pragma code

#pragma interruptlow low_isr
void low_isr(void)
{
    // codigo interrupcao
}
*/

void main(void)
{
    char byte_recebido;
    config_pic();                       // Configura��es iniciais do PIC (Ports, etc.);
    usb_install();                      // Inicializa��o do USB;
    
    do
    {
        byte_recebido = recebe_dado_usb();  // Recebe os dados vindos da rasp;
        processa_controle(byte_recebido);   // Envia o dado obtido para que o controle tome uma decis�o;
    } while(1);
}


/* Fun��o no qual tem como objetivo buscar os dados no buffer de entrada do
 PIC, s� retornando dela quando alguma informa��o for recebida*/
char recebe_dado_usb()
{
    char byte_recebido;

    do
    {
        /*Maquina de estados do USB, no qual verifica se o mesmo esta configurado
         e executa a inclus�o e exclus�o de dados nos buffer de entrada e sa�da.*/
        usb_handler();
        
        /* O valor 0xFF funciona como padrao para a variavel byte_recebido,
         * e significa que nenhum byte foi recebido pelo PIC. */
        byte_recebido = 0xFF;

        /* Caso tenha algum char no buffer de entrada do PIC,
         * esse sera' transferido para a variavel byte_recebido.
         * Caso nao, o valor inicial 0xFF sera' mantido. */
        poll_getc_cdc(&byte_recebido);

        /* Se nenhum byte tiver sido recebido, esse ciclo do loop
         * sera' encerrado nesse ponto. */
    } while ( byte_recebido == 0xFF );

    return byte_recebido;                   // Retorna o dado recebido;

} 




/* Fun��o de controle, no qual toma as decis�es do que ser� feito de acordo
 com o comando recebido da Rasp;*/
int processa_controle( char controle )
{
    switch(controle)
    {
        case 0x30:              //Caracter '0' ou 0x30 - Processo de teste de comunica��o;
            putc_cdc('O');
            //putc_cdc('k');
            //putc_cdc(' ');
            break;

        case 0x5F:              //Caracter '_' ou 0x5F - Limpa a linha do display;
            limpa();
            break;

        case 0x31:              //Caracter '1' ou 0x31 - Converte a corrente em hexa para informar a raspberry;
            converte_corrente();
            break;

        case 0x32:              //Caracter '2' ou 0x32 - Converte a temperatura do sensor 1 em hexa;
            converte_temperatura('1');
            break;

        case 0x33:              //Caracter '3' ou 0x33 - Converte a temperatura do sensor 2 em hexa;
            converte_temperatura('2');
            break;

        case 0x34:              //Caracter '4' ou 0x34 - Converte a temperatura do sensor 3 em hexa;
            converte_temperatura('3');
            break;

        default:                // N�o faz nada e retorna para receber proximo controle;
            putc_cdc('N');
            //putc_cdc('o');
            //putc_cdc('p');
            //putc_cdc(' ');

    }
    return 1;
}



/*Configura��o inicial do Pic*/
void config_pic(void)
{
     /* Configura o port RA0/AN0 como port de entrada anal�gico
     * As refer�ncias de tens�o usadas s�o as internas do PIC (0V e 5V)
     * Este port ser� usado para verifica��o de expans�o, no qual
     ser� feito por um divisor de tens�o que indicar� qual expans�o
     esta conectada;*/
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_8ANA);

    // Configura��o de prioridade
    RCONbits.IPEN = 1;  //Habilita interrup��o com prioridade
    INTCONbits.GIEH=1;  //Habilita todas as interrup��es de prioridade alta
    INTCONbits.GIEL=0;  //Habilita todas as interrup��es de prioridade baixa


    // Confiru��o do timer1
    OpenTimer1( TIMER_INT_ON	// Interrup��o do timer0 habilitada (Desabilitar _OFF)
                & T1_16BIT_RW	// Tamanho do contador em 16 Bits (N�o usado por estarmos no modo temporizador)
                & T1_SOURCE_INT	// Fonte de clock interna (Fonte externa _EXT)
                & T1_PS_1_1     // PreScaler em 1:8 (Pode ser usado valores de 1, 2, 4 E 8)
                & T1_OSC1EN_OFF
                & T1_SYNC_EXT_OFF);
    //Calculo do timer: [(48Mhz)^-1]*4(padr�o)*1(Pr� Scale 1:1)*(2^16)(16 bits) = 5,46ms ou 183Hz

    PIE1bits.TMR1IE = 1;     //Habilita interrup��o por estouro de contador

    TRISA = 0x3F;       // RA0,RA1,RA2,RA3,RA5 ent. analog.; RA4 ent. dig.; RA6 e RA7 saida dig.; - 0b 0011 1111
    TRISB = 0x00;       // PortB 0 a 7 como sa�da; (n�o usados); - 0b 0000 0000
    TRISC = 0x00;       // PortC 0 a 7 como sa�da; - 0b 0000 0000
    TRISD = 0x00;       // RD1 ent. dig.; O resto do PortD como sa�da;
    TRISE = 0x07;       // RE0; RE1; RE2 ent. analog.; RE3 saida digital - 0b 0000 0111;

    PORTA = 0x00;       // Todo portA com n�vel l�gico baixo;
    PORTB = 0x00;       // Todo portB com n�vel l�gico baixo;
    PORTC = 0x00;       // Todo portC com n�vel l�gico baixo;
    PORTD = 0x00;       // Todo portD com n�vel l�gico baixo;
    PORTE = 0x00;       // Todo portE com n�vel l�gico baixo;

    SSPCON1bits.SSPEN = 1;
}

void limpa(void)
{
    unsigned short i = 0;
    while(i<80)
    {
        putc_cdc(' ');
        i++;
    }
}

//Sensor de corrente
unsigned short sensor_corrente_6v(void)       //AN_6V_
{
    unsigned short tensao;          // Armazena o valor da convers�o ADC feita;
    SetChanADC(ADC_CH2);            // Seta o canal anal�gico 2 (AN2) no qual verifica a existencia ou n�o de expans�o por um divisor de tens�o
    ConvertADC();                   //Inicia convers�o ADC
    while(BusyADC());               // Aguarda a finaliza��o da convers�o
    tensao = ReadADC();             //Guarda a informa��o obtida da convers�o
    if((tensao < 450) || (tensao > 757))    // Fora do padr�o de 0 a 25A
    {
        PORTDbits.RD4 = 1;          //Desliga totalmente o DIDV atrav�s do pino SD;
    }
    else
    {
        PORTDbits.RD4 = 0;      //Este ELSE pode ser removido ap�s os testes, pois depois que desligar todo o sistema obviamente n�o podera retornar;
    }
    return tensao;
}

void aciona_cooler(void)
{
    unsigned short liga[3];
    unsigned short contar = 0;
    liga[0] = sensor_temperatura_1();
    liga[1] = sensor_temperatura_2();
    liga[2] = sensor_temperatura_3();

    while(contar<3)
    {
        if(liga[contar] > 634)                        // Aciona para temperaturas acima de 30� (3,4V) Ap�s filtro-> 3,1V - (3,1/5)*1023 = 634
        {
             liga[contar] = 1;
        }
        else
        {
             liga[contar] = 0;
        }
        contar++;
    }

    if((liga[0] + liga[1] + liga[2]) > 0)
    {
        PORTCbits.RC2 = 1;                  //RC2 - Aciona Cooler
    }
    else
    {
        PORTCbits.RC2 = 0;                  //RC2 - Desliga cooler
    }
}

// Sensor de temperatura 1
unsigned short sensor_temperatura_1(void)             //TS1_
{
    unsigned short tensao;                  // Armazena o valor da convers�o ADC feita;
    SetChanADC(ADC_CH4);                    // Seta o canal anal�gico 4 (AN4) no qual verifica a existencia ou n�o de expans�o por um divisor de tens�o
    ConvertADC();                           //Inicia convers�o ADC
    while(BusyADC());                       // Aguarda a finaliza��o da convers�o
    tensao = ReadADC();                     //Guarda a informa��o obtida da convers�o
    return tensao;
}

// Sensor de temperatura 2
unsigned short sensor_temperatura_2(void)             //TS2_
{
    unsigned short tensao;                  // Armazena o valor da convers�o ADC feita;
    SetChanADC(ADC_CH5);                    // Seta o canal anal�gico 5 (AN5) no qual verifica a existencia ou n�o de expans�o por um divisor de tens�o
    ConvertADC();                           //Inicia convers�o ADC
    while(BusyADC());                       // Aguarda a finaliza��o da convers�o
    tensao = ReadADC();                     //Guarda a informa��o obtida da convers�o
    return tensao;
}

// Sensor de temperatura 3
unsigned short sensor_temperatura_3(void)             //TS3_
{
    unsigned short tensao;                  // Armazena o valor da convers�o ADC feita;
    SetChanADC(ADC_CH7);                    // Seta o canal anal�gico 7 (AN7) no qual verifica a existencia ou n�o de expans�o por um divisor de tens�o
    ConvertADC();                           //Inicia convers�o ADC
    while(BusyADC());                       // Aguarda a finaliza��o da convers�o
    tensao = ReadADC();                     //Guarda a informa��o obtida da convers�o
}

void converte_corrente(void)
{
    unsigned short adc;                  // Recebera um valor entre 0 e 1023, no qual indica o valor da correte;
    unsigned short i = 0;
    char resposta = 0x00;
    adc = (sensor_corrente_6v()/4);
    while(i < adc)
    {
        resposta++;
        i++;
    }
    putc_cdc(resposta);
}

void converte_temperatura(char sensor)
{
    unsigned short adc;
    unsigned short i = 0;
    char resposta = 0x00;
    if(sensor == '1')
    {
       adc = (sensor_temperatura_1()/4);
    }
    else if(sensor == '2')
    {
        adc = (sensor_temperatura_2()/4);
    }
    else if(sensor == '3')
    {
        adc = (sensor_temperatura_3()/4);
    }
    else
    {
        putc_cdc('X');          // Nunca deve acontecer
    }
    while(i < adc)
    {
        resposta++;
        i++;
    }
    putc_cdc(resposta);
}