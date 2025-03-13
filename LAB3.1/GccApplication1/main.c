/*
 * GccApplication1.c
 *
 * Created: 2/27/2025 13:53:48
 * Author : Student
 */ 

/************************************************************************/
/* INCLUDE                                                              */
/************************************************************************/
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>  // To use snprintf for formatting

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define BAUD 38400
#define MYUBRR ((F_CPU / (16 * BAUD)) - 1)
#define PRESCALE (1 << CS12) | (1 << CS10)  // Set prescaler to 1024
#define SCL_CLOCK 100000L
#define I2C_ADDRESS 0x48

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

uint8_t textt = 0x41;  // 'A'
char *string = "Hello";
volatile uint8_t menu_level = 0;  // Track which menu level we're in
volatile uint8_t char_to_process = 0;  // Flag to process character input
volatile char recv = 0;  // Store received character
volatile uint8_t pwm_duty_cycle = 128;  // Start with 50% duty cycle for PWM

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void board_init();
void USART_init(uint16_t _ubrr);
void USART_Transmit(uint8_t data);
uint8_t USART_GetChar(void);
void UART_SendChar(uint8_t data);
void UART_SendString(char *text);
void showMenu();
void handleMenuOption(uint8_t option);
void handleMainMenu(uint8_t option); // Correct declaration
void handleSubMenu();
void handleSubMenuOption(uint8_t option);

// Timer1 functions
void Timer1_cmp_start(uint16_t value);
void Timer1_stop();  // Function to stop Timer1
ISR(TIMER1_COMPA_vect);

// Timer2 functions for PWM control
void Timer2_fastpwm_start(uint8_t strida);
void adjust_pwm_duty_cycle(int8_t change);  // Function to increase or decrease duty cycle

// I2C functions
void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_write(uint8_t data);
uint8_t i2c_read_ack();
uint8_t i2c_read_nack();
uint16_t read_temperature();

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

// I2C Functions
void i2c_init() {
	// Set SCL frequency to the defined value
	TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2;
	TWSR = 0;  // Prescaler value of 1
}

void i2c_start() {
    TWCR = (1 << TWSTA) | (1 << TWINT) | (1 << TWEN);  // Send START condition
    while (!(TWCR & (1 << TWINT)));  // Wait for TWINT flag
}

void i2c_stop() {
	// Send a stop condition
	TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);
	while (TWCR & (1 << TWSTO));  // Wait until stop condition is executed
}

void i2c_write(uint8_t data) {
    TWDR = data;  // Load data into the data register
    TWCR = (1 << TWINT) | (1 << TWEN);  // Start transmission
    while (!(TWCR & (1 << TWINT)));  // Wait for TWINT flag
}

uint8_t i2c_read_ack() {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);  // Send ACK after data byte
    while (!(TWCR & (1 << TWINT)));  // Wait for TWINT flag
    return TWDR;  // Return the received byte
}

uint8_t i2c_read_nack() {
    TWCR = (1 << TWINT) | (1 << TWEN);  // Send NACK after data byte
    while (!(TWCR & (1 << TWINT)));  // Wait for TWINT flag
    return TWDR;  // Return the received byte
}

// AT30TSE758 Functions
#define AT30TSE758_ADDR 0x48  // I2C address of the AT30TSE758

uint16_t read_temperature() {
	uint8_t highByte, lowByte;
	uint16_t temperature;

	i2c_start();  // Start I2C communication
	i2c_write(I2C_ADDRESS << 1);  // Send device address (write mode)
	i2c_write(0x00);  // Send register address to read temperature (Assuming 0x00 is the temperature register)
	
	i2c_start();  // Restart I2C communication (repeated start)
	i2c_write((I2C_ADDRESS << 1) | 1);  // Send device address (read mode)
	
	highByte = i2c_read_ack();  // Read high byte
	lowByte = i2c_read_nack();   // Read low byte (send NACK at the end)
	
	i2c_stop();  // Stop I2C communication

	// Combine the high and low bytes to form the full 16-bit temperature data
	temperature = ((uint16_t)highByte << 8) | lowByte;
	return temperature;
}

// USART Functions (existing)

void USART_init(uint16_t _ubrr) {
    UBRR1H = (uint8_t)(_ubrr >> 8);
    UBRR1L = (uint8_t)(_ubrr);
    UCSR1B |= (1 << RXEN1) | (1 << TXEN1);
    UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);
}

void USART_Transmit(uint8_t data) {
    while (!(UCSR1A & (1 << UDRE1)));
    UDR1 = data;
}

uint8_t USART_GetChar(void) {
    while (!(UCSR1A & (1 << RXC1)));
    return UDR1;
}

void UART_SendChar(uint8_t data) {
    while (!(UCSR1A & (1 << UDRE1)));
    UDR1 = data;
}

void UART_SendString(char *text) {
    while (*text != 0x00) {
        UART_SendChar(*text);
        text++;
    }
}

void board_init() {
    USART_init(MYUBRR);  // Initialize UART with baud rate
    i2c_init();
    // Set PB4 as output (for the PWM LED)
    DDRB |= (1 << PB4);  // Set PB4 as output for OC2A (PWM output)
    DDRB |= (1 << PB5);  // Set PB5 as output for Timer1 compare output (OC1A)
}

// Timer1 functions
void Timer1_cmp_start(uint16_t value) {
	cli();  // Disable interrupts
	
	// Set Timer1 to CTC mode (Clear Timer on Compare Match)
	TCCR1A = 0;  // Clear control register A
	TCCR1B = 0;  // Clear control register B
	TIMSK1 = 0;   // Disable interrupts for Timer1

	OCR1A = value;  // Set the compare value (e.g., 7812 for 2Hz)

	// Configure Timer1 in CTC mode
	TCCR1B |= (1 << WGM12);  // CTC mode

	// Set the prescaler to 1024
	TCCR1B |= PRESCALE;

	// Enable toggle output on OC1A (PORTB5)
	TCCR1A |= (1 << COM1A0);

	// Enable compare match interrupt (optional, if needed)
	TIMSK1 |= (1 << OCIE1A);

	sei();  // Enable global interrupts
}

// Function to stop Timer1
void Timer1_stop() {
	cli();  // Disable interrupts
	TCCR1B = 0;  // Disable Timer1
	TIMSK1 = 0;  // Disable Timer1 interrupts
	TCCR1A &= ~(1 << COM1A0);  // Disable output toggle on OC1A (PB5)
	sei();  // Enable global interrupts
}

// Timer1 Compare Match ISR
ISR(TIMER1_COMPA_vect) {
	PORTB ^= (1 << PB5);  // Toggle PORTB5 (LED) on compare match (e.g., for a 2Hz frequency)
}

// Function to start Timer2 in Fast PWM mode
void Timer2_fastpwm_start(uint8_t strida) {
	cli();  // Disable interrupts to prevent timing issues
	TCCR2A = 0;  // Clear the control register A
	TCCR2B = 0;  // Clear the control register B
	TIMSK2 = 0;   // Disable interrupts for Timer2

	// Set PWM duty cycle based on input parameter 'strida' (0 to 100)
	OCR2A = (255 * strida) / 100;  // Set the compare value based on percentage

	// Fast PWM mode
	TCCR2A |= (1 << WGM21) | (1 << WGM20);  // Select Fast PWM mode

	// Set prescaler to 1024
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);

	// Enable output on OC2A (PB4) pin
	TCCR2A |= (1 << COM2A1);

	sei();  // Re-enable interrupts
}

// Function to adjust PWM duty cycle
void adjust_pwm_duty_cycle(int8_t change) {
	if (pwm_duty_cycle + change <= 255 && pwm_duty_cycle + change >= 0) {
		pwm_duty_cycle += change;  // Update the duty cycle
	}
	Timer2_fastpwm_start(pwm_duty_cycle);  // Restart Timer2 with the new duty cycle
}

void showMenu() {
	UART_SendString("\033[2J\033[H");  // Clear screen and move cursor to home position
	UART_SendString("\nSelect an option:\r\n");
	UART_SendString("1. Show A (Pismeno)\r\n");
	UART_SendString("2. Show ABCDEFG\r\n");
	UART_SendString("3. Enter Submenu\r\n");
	UART_SendString("4. Option 4 (Blank)\r\n");
	UART_SendString("5. Generate 2Hz Frequency (Timer1)\r\n");
	UART_SendString("6. Increase PWM Brightness (+)\r\n");
	UART_SendString("7. Decrease PWM Brightness (-)\r\n");
	UART_SendString("8. Stop 2Hz Frequency\r\n");  // Option to stop frequency generation
	UART_SendString("9. Read Temperature (T)\r\n");  // Option to read temperature
	UART_SendString("0. Exit\r\n");

	// Delay to prevent instant refresh
	_delay_ms(500);  // Adjust the delay as per your requirement
}


void handleMainMenu(uint8_t option) {
	switch (option) {
		case '1':
		UART_SendString("You selected Option 1: Show A (Pismeno)\n");
		UART_SendChar(textt);  // Show character 'A'
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '2':
		UART_SendString("You selected Option 2: Show ABCDEFG\n");
		UART_SendString("ABCDEFG\n");
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '3':
		UART_SendString("You selected Option 3: Enter Submenu\n");
		// You can add submenu handling here
		// For now, it simply prints a message
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '4':
		UART_SendString("You selected Option 4: (Blank)\n");
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '5':
		UART_SendString("Generating 2Hz frequency...\n");
		Timer1_cmp_start(7812);  // Start the timer with the calculated value for 2Hz
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '6':
		UART_SendString("Increasing PWM Brightness\n");
		adjust_pwm_duty_cycle(10);  // Increase brightness by 10
		break;
		case '7':
		UART_SendString("Decreasing PWM Brightness\n");
		adjust_pwm_duty_cycle(-10);  // Decrease brightness by 10
		break;
		case '8':  // New option to stop the 2Hz frequency generation
		UART_SendString("Stopping 2Hz frequency generation.\n");
		Timer1_stop();  // Stop Timer1 and the 2Hz signal
		_delay_ms(1000);  // Delay for 1 second
		break;
		case 'T':
		UART_SendString("You selected Option T: Read Temperature\n");
		uint16_t temperature = read_temperature();
		char temp_str[20];
		snprintf(temp_str, sizeof(temp_str), "Temperature: %.2f °C\n", temperature);
		UART_SendString(temp_str);  // Display the temperature
		_delay_ms(1000);  // Delay for 1 second
		break;
		case '0':
		UART_SendString("Exiting program.\n");
		menu_level = 0;  // Return to main menu
		_delay_ms(1000);  // Delay for 1 second before exiting
		break;
		default:
		UART_SendString("Invalid option! Please select again.\n");
		_delay_ms(1000);  // Delay for 1 second before displaying the menu again
		break;
	}
}


int main(void) {
    board_init();
    i2c_init();  // Initialize I2C interface
    sei();  // Enable global interrupts

    while (1) {
        showMenu();  // Show the menu

        // Wait for user input and handle it
        recv = USART_GetChar();
        handleMainMenu(recv);
    }

    return 0;
}
