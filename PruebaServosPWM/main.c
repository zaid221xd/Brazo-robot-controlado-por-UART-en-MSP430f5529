#include "intrinsics.h"
#include <msp430.h>

#define INPUT_SIZE 8

void configura_puertos (void);
void configura_timers (void);
void DCO_setup(void);
void UART_setup(void);
void UART_send(unsigned char *str);
void set_servo(unsigned char servo, unsigned char angulo);
int convert_ASCII(unsigned char *str);
void show_position(unsigned char posicion);
void num_to_str(unsigned int num, unsigned char *numstr);

unsigned int duty_cycle[4] = {500, 500, 500, 500};
unsigned char secuencia[4][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
unsigned char counter = 0;
unsigned char counter_converter = 0;
unsigned char flag = 0;
unsigned char input[INPUT_SIZE];
unsigned char index = 0;
unsigned char servo;
unsigned char angulo;
unsigned char posicion = 1;
unsigned int i=0;
unsigned int j=0;
unsigned char numstr[5];

void main (void) {
    WDTCTL = WDT_ADLY_250; // Configurar WDT a 250ms, usando el ACLK
    SFRIE1 |= WDTIE; // Activa la interrupción del WDT

    configura_puertos();
    configura_timers();        // PWM
    DCO_setup(); //Configurar el DCO
    UART_setup(); //Configurar el UART

    __enable_interrupt();

    while(1){
        UART_send("Control de robot.\r\nSeleccione 1 para modo manual o 2 para modo captura: \r\n\0");
        flag |= BIT2;
        while(flag & BIT2);
        if(input[0] == '1'){
            flag |= BIT0;
        }else if(input[0] == '2'){
            flag |= BIT1;
        }else{
            UART_send("Comando desconocido.\r\n\0");
        }
        while(flag & BIT0) { //Modo manual
            UART_send("Seleccionar servo (1,2,3,4), 0 para salir: \r\n\0");
            flag |= BIT2;
            while(flag & BIT2);
            if(input[0] == '0'){
                flag &= ~BIT0;
            }else if(input[0] == '1'||input[0] == '2'||input[0] == '3'||input[0] == '4'){
                servo = input[0];
                UART_send("Seleccionar angulo (0-180): \r\n\0");
                flag |= BIT2;
                while(flag & BIT2);
                angulo = convert_ASCII(input);
                set_servo(servo, angulo);
            }else{
                UART_send("Ingresa una opción válida.\r\n\0");
            }
        }
        while(flag & BIT1) { //Modo captura
            UART_send("Seleccionar posicion (1-10)\r\n11 para ejecutar, 12 para borrar, 0 para salir: \r\n\0");
            flag |= BIT2;
            while(flag & BIT2);
            flag |= BIT4;
            switch(input[0]){
            case '0':
                flag &= ~BIT1;
                flag &= ~BIT4;
                break;
            case '1':
                switch(input[1]){
                case '0':
                    UART_send("Posicion 10 - seleccionar servo: \r\n\0");
                    posicion = 10;
                    break;
                case '1':
                    UART_send("Ejecutando...\r\n\0");
                    for(i = 0; i < 10; i++){
                        flag |= BIT3;
                        while(flag & BIT3);
                        for(j = 0; j < 4; j++){
                            set_servo(j, secuencia[j][i]);
                        }
                    }
                    flag &= ~BIT4;
                    UART_send("Secuencia termidada.\r\n\0");
                    break;
                case '2':
                    UART_send("De verdad quieres borrar la secuencia? 1 para si: \r\n\0");
                    flag |= BIT2;
                    while(flag & BIT2);
                    if(input[0] == '1'){
                        UART_send("Secuencia borrada.\r\n\0");
                        for(i = 0; i < 10; i++){
                            for(j = 0; j < 4; j++){
                                secuencia[j][i] = 0;
                            }
                        }
                    }
                    flag &= ~BIT4;
                    break;
                default:
                    UART_send("Posicion 1 - seleccionar servo: \r\n\0");
                    posicion = 1;
                    break;
                }
                break;
            case '2':
                UART_send("Posicion 2 - seleccionar servo: \r\n\0");
                posicion = 2;
                break;
            case '3':
                UART_send("Posicion 3 - seleccionar servo: \r\n\0");
                posicion = 3;
                break;
            case '4':
                UART_send("Posicion 4 - seleccionar servo: \r\n\0");
                posicion = 4;
                break;
            case '5':
                UART_send("Posicion 5 - seleccionar servo: \r\n\0");
                posicion = 5;
                break;
            case '6':
                UART_send("Posicion 6 - seleccionar servo: \r\n\0");
                posicion = 6;
                break;
            case '7':
                UART_send("Posicion 7 - seleccionar servo: \r\n\0");
                posicion = 7;
                break;
            case '8':
                UART_send("Posicion 8 - seleccionar servo: \r\n\0");
                posicion = 8;
                break;
            case '9':
                UART_send("Posicion 9 - seleccionar servo: \r\n\0");
                posicion = 9;
                break;
            default:
                UART_send("Comando desconocido.\r\n\0");
                flag &= ~BIT4;
                break;
            }
            while(flag & BIT4){
                UART_send("Angulos guardados: \r\n\0");
                show_position(posicion);
                UART_send("Seleccionar servo (1,2,3,4)\r\n0 para borrar, 5 para guardar y salir: \r\n\0");
                flag |= BIT2;
                while(flag & BIT2);
                switch(input[0]){
                case '1':
                    servo = 1;
                    break;
                case '2':
                    servo = 2;
                    break;
                case '3':
                    servo = 3;
                    break;
                case '4':
                    servo = 4;
                    break;
                case '0':
                    UART_send("De verdad quieres borrar la secuencia? 1 para si\r\n\0");
                    flag |= BIT2;
                    while(flag & BIT2);
                    if(input[0] == '1'){
                        for(j = 0; j<4; j++){
                            secuencia[j][posicion-1] = 0;
                        }
                        UART_send("Secuencia borrada.\r\n\0");
                        flag |= BIT5;
                    }
                    break;
                case '5':
                    UART_send("Guardado.\r\n\0");
                    flag |= BIT5;
                    flag &= ~BIT4;
                }
                if(!(flag & BIT5)){
                    UART_send("Seleccionar angulo (0-180): \r\n\0");
                    flag |= BIT2;
                    while(flag & BIT2);
                    secuencia[servo][posicion-1] = convert_ASCII(input);
                }
            }
        }
    }
}

void configura_puertos (void) {
    // P1.2 y P1.3
    P1DIR |= 0x0C;
    P1SEL |= 0x0C;
    // P4.1 y 4.2
    P4DIR |= 0x06;
    P4SEL |= 0x06;
}

void configura_timers (void) {
    //Timer A0
    TA0CCR0 = 20000 - 1;         // Frecuencia del PWM
                                // ~1,000,000 / 20000 = 50Hz
    //P1.2
    TA0CCTL1 = OUTMOD_7;        // Configuración SET/RESET (user's guide p.470)
    TA0CCR1 = 500;              // Duty cycle de 0.5ms - Posicion 0º
    //P1.3
    TA0CCTL2 = OUTMOD_7;
    TA0CCR2 = 500;

    TA0CTL = TASSEL_2 | MC_1 | TACLR;   // SMCK | modo UP | limpia TAR

    //Timer B0
    TB0CCR0 = 20000 - 1;

    //P4.1
    TB0CCTL1 = OUTMOD_7;
    TB0CCR1 = 500;
    //P4.2
    TB0CCTL2 = OUTMOD_7;
    TB0CCR2 = 500;

    TB0CTL = TBSSEL_2 | MC_1 | TBCLR;
}

void DCO_setup(void){
    // Configuración del DCO a 16MHz
    UCSCTL3 = SELREF_2;                       // Referencia del DCO FLL = REFO
    UCSCTL4 |= SELA_2;                        // Configurar ACLK = REFO
    UCSCTL0 = 0x0000;                         // Configurar el DCOx, MODx más bajo posible

    // Mientras XT1,XT2 y el DCO se estabilizan, permanece en el loop
    // Como no estamos usando cristales externos, solo el DCO se debe estabilizar
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); // Limpia las banderas de falla de cristal 1 y 2 y del DCO
        SFRIFG1 &= ~OFIFG;                          // Limpia la bandera de falla de los osciladores
    } while (SFRIFG1 & OFIFG);                      // Revisa si no hay falla en los osciladores

    __bis_SR_register(SCG0);                // Desactiva el ciclo de control del FLL (Frequency-Locked Loop)
    UCSCTL1 = DCORSEL_5;                    // Selecciona el rango de operación de 16MHz
    UCSCTL2 = 487;                         // Configura el multiplicador del DCO para 8MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (487 + 1) * 32768 = 16MHz
    __bic_SR_register(SCG0);                // Activa el ciclo de control del FLL

    // De acuerdo con la User's Guide, el tiempo más grande para que se inicie el DCO está dado por
    // n x 32 x 32 x f_MCLK / f_FLL_reference
    // 16MHz x 32 x 32 / 32,768 = 500,000 ciclos
    __delay_cycles(500000);
}

void UART_setup(void) {
    P4SEL |= 0x30;                            // P4.4,5 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                      // Resetea la máquina de estados
    UCA1CTL1 |= UCSSEL_2 ;                     // SMCLK | Oversampling???
    UCA1BR0 = 138;                            // 16MHz 115200 (User's Guide)
    UCA1BR1 = 0;                              // 16MHz 115200
    UCA1MCTL |= UCBRS_7 + UCBRF_0;            // Modulación UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // Inicializa la máquina de estados
    UCA1IE |= UCRXIE;                         // Activa la interrupción de RX
}

void UART_send(unsigned char *str){
    while(*str != '\0') { //Mientras haya texto en el arreglo
        while(!(UCA1IFG & UCTXIFG)); // Está listo el buffer de TX?
        UCA1TXBUF = *str++; //Mandar siguiente caracter
    }
}

void set_servo(unsigned char servo, unsigned char angulo){
    if(angulo<=180){
        duty_cycle[servo-1] = angulo*11+500; //2000/180º=11, 500 = 0º
    }else{
        UART_send("Error. Angulo fuera del rango. \r\n\0");
    }
    TA0CCR1 = duty_cycle[0];
    TA0CCR2 = duty_cycle[1];
    TB0CCR1 = duty_cycle[2];
    TB0CCR2 = duty_cycle[3];
}

int convert_ASCII(unsigned char *str){
    int angulo=0;
    int digit=0;
    counter_converter=0;
    while(*str != '\0' && counter_converter<3){
        switch(*str){
                case '0':
                    digit = 0;
                    break;
                case '1':
                    digit = 1;
                    break;
                case '2':
                    digit = 2;
                    break;
                case '3':
                    digit = 3;
                    break;
                case '4':
                    digit = 4;
                    break;
                case '5':
                    digit = 5;
                    break;
                case '6':
                    digit = 6;
                    break;
                case '7':
                    digit = 7;
                    break;
                case '8':
                    digit = 8;
                    break;
                case '9':
                    digit = 9;
                    break;
                default:
                    digit = 0;
                    break;
                }
                angulo = angulo*10 + digit;
                *str++;
        counter_converter++;
    }
    return angulo;
}

void show_position(unsigned char posicion){
    UART_send("Posiciones: ");
    //for(i = 0; i<4; i++){
        num_to_str(secuencia[1][posicion-1]/100, numstr);
        UART_send(numstr);
        num_to_str((secuencia[1][posicion-1]%100)/10, numstr);
        UART_send(numstr);
        num_to_str(secuencia[1][posicion-1]%10, numstr);
        UART_send(numstr);
        UART_send(" ");

        num_to_str(secuencia[2][posicion-1]/100, numstr);
        UART_send(numstr);
        num_to_str((secuencia[2][posicion-1]%100)/10, numstr);
        UART_send(numstr);
        num_to_str(secuencia[2][posicion-1]%10, numstr);
        UART_send(numstr);
        UART_send(" ");

        num_to_str(secuencia[3][posicion-1]/100, numstr);
        UART_send(numstr);
        num_to_str((secuencia[3][posicion-1]%100)/10, numstr);
        UART_send(numstr);
        num_to_str(secuencia[3][posicion-1]%10, numstr);
        UART_send(numstr);
        UART_send(" ");

        num_to_str(secuencia[4][posicion-1]/100, numstr);
        UART_send(numstr);
        num_to_str((secuencia[4][posicion-1]%100)/10, numstr);
        UART_send(numstr);
        num_to_str(secuencia[4][posicion-1]%10, numstr);
        UART_send(numstr);
        UART_send("\r\n\0");
    //}
}

void num_to_str(unsigned int num, unsigned char *numstr){
    i=0;
    switch (num) {
            case 0:
                numstr[i++] = '0';
                break;
            case 1:
                numstr[i++] = '1';
                break;
            case 2:
                numstr[i++] = '2';
                break;
            case 3:
                numstr[i++] = '3';
                break;
            case 4:
                numstr[i++] = '4';
                break;
            case 5:
                numstr[i++] = '5';
                break;
            case 6:
                numstr[i++] = '6';
                break;
            case 7:
                numstr[i++] = '7';
                break;
            case 8:
                numstr[i++] = '8';
                break;
            case 9:
                numstr[i++] = '9';
                break;
        }
    numstr[i] = '\0';
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void) {
    counter++;
    if(counter >= 12){
        counter=0;
        flag &= ~BIT3;
    }
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    switch(__even_in_range(UCA1IV,4)) {
        case 0:break;                             // Vector 0 - no interrupt
        case 2:                                   // Vector 2 - RXIFG
            while (!(UCA1IFG & UCTXIFG));         // Está listo el buffer de TX?
            UCA1TXBUF = UCA1RXBUF;                // Envía de vuelta el caracter recibido
            /*if(UCA1RXBUF == "\b"){ //backspace?
                input[index]--;
                input[index] = "\0";
            }else{*/
            if(UCA1RXBUF != '\n' && UCA1RXBUF != '\r' && index < INPUT_SIZE){
                input[index++] = UCA1RXBUF;
            }else{
                input[index] = '\0';
                index = 0;
                flag &= ~BIT2;
            }
            //}
            break;
        case 4:break;                             // Vector 4 - TXIFG
        default: break;
    }
}
