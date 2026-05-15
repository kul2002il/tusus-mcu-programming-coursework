
// # Работа с экраном.

#define CHAR_NULL 10;

char chars[] = {
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11110110, // 9
  0b00000000, // null
};

void sendBit(char data) {
  if (data & 1) {
    PORTB |= 1;
  } else {
    PORTB &= ~1;
  }
  PORTD |= 1<<7;
  PORTD &= ~(1<<7);
}

void sendByte(char data) {
  for (int i = 0; i < 8; i++) {
    sendBit(data);
    data >>= 1;
  }
}

void passNumbers() {
  PORTD |= 1<<4;
  PORTD &= ~(1<<4);
}

char currentChars[4] = {0b01100000, 0b11110110, 0b11111110, 0b01100110};
char index = 0;

void sendCurrentChar() {
  sendByte(~currentChars[index]);
  sendByte(1 << (7 - index));
  passNumbers();
  index = (index + 1) & 0b11;
}

void printNumber(int number) {
  currentChars[0] = chars[(number/1000)%10];
  currentChars[1] = chars[(number/100 )%10];
  currentChars[2] = chars[(number/10  )%10];
  currentChars[3] = chars[(number     )%10];
}

// # Работа с шаговым двигателем.
char stepSequence[] = {
  0b01100000, // 0
  0b00100010, // 1
  0b00000011, // 2
  0b01000001, // 3
};

char flagDirection = 0;
int counter = 205;

char currentStep = 0;
uint32_t lastStepAt = 0;

void sendNextStep() {
  if ((millis() - lastStepAt) < (1000 / counter)) {
    return;
  }
  
  lastStepAt += 1000 / counter;
  
  currentStep = (currentStep + 1 - flagDirection * 2) & 0b11;
  PORTD = (PORTD & ~0b01100011) | stepSequence[currentStep];
};

// # Кнопки
uint32_t lastInteraptionAt = 0;

void readButtons() {
  char newButtonState = (~PINC >> 1) & 0b111;
  
  if (newButtonState == 0) {
    return;
  }

  if (millis() - lastInteraptionAt < 100) {
    return;
  }

  lastInteraptionAt = millis();
  
  switch (newButtonState) {
    case 0b100:
      counter -= 10;
    break;
    case 0b010:
      flagDirection ^= 1;
    break;
    case 0b001:
      counter += 10;
    break;
  }
}

// Инициализация

void setup() {
  DDRB = 0b00000001;
  DDRC = 0b00000000;
  DDRD = 0b11110011;

  // TC2.
  TIMSK2 = (1<<TOIE2);
  TCCR2B = (1 << CS21) | (1 << CS21) | (0 << CS20); // Тактирование с делителем 256.
  OCR2A = 200;

  // PCINT.
  PCICR |= 1<<1;
  PCMSK1 |= 0b1110;
  
  sei();
  
  printNumber(counter);
}

void loop() {
  sendNextStep();
}

// Таймер
ISR(TIMER2_OVF_vect) {
  sendCurrentChar();
}

// Кнопки
ISR(PCINT1_vect) {
  readButtons();
  printNumber(counter);
  currentChars[0] |= flagDirection;
}
