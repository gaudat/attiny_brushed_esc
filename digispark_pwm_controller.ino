// P5: Reset pin
// P4: Servo input (disconnect when using USB)
// P3: USB
// P2: IN2
// P1: OC0B - PWM output, LED
// P0: IN1

#include <core_timers.h>

// OCRA = 0xff -> 16500000 / 8 / 256 = 8056 Hz
// 12 kHz = 172, 16 kHz = 129, 20 kHz = 103, 24 kHz = 86

uint8_t pwm_max = 255;

void pwm_setup() {
  Timer0_SetToPowerup();
  Timer0_SetWaveformGenerationMode(Timer0_Phase_Correct_PWM_OCR);
  Timer0_SetOutputCompareMatchA(pwm_max);
  pwm_write(0);
  Timer0_SetCompareOutputModeB(Timer0_Clear);
  Timer0_ClockSelect(Timer0_Prescale_Value_1);
}

void pwm_write(uint8_t pwm_val) {
  if (pwm_val > pwm_max) {
    return;
  }
  Timer0_SetOutputCompareMatchB(pwm_val);
}

// millis uses Timer1 so Timer0 can be freely used

void setup() {
  pinMode(4, INPUT);
  digitalWrite(2, LOW);
  pinMode(2, OUTPUT);
  digitalWrite(1, LOW);
  pinMode(1, OUTPUT);
  digitalWrite(0, LOW);
  pinMode(0, OUTPUT);
  pwm_setup();
  center_setup();
}

void brake() {
  pwm_write(0);
  digitalWrite(0, LOW);
  digitalWrite(2, LOW);
}

void coast() {
  pwm_write(0);
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
}

void forward() {
  // Write high first to coast
  digitalWrite(0, HIGH);
  digitalWrite(2, LOW);
}

void backward() {
  // Write high first to coast
  digitalWrite(2, HIGH);
  digitalWrite(0, LOW);
}

uint32_t reverse_start = 0;
bool last_direction = true;
bool reverse_waiting = false;
static const uint16_t reverse_timeout = 100;

bool check_reverse(bool forward) {
    if (!reverse_waiting && !(forward ^ last_direction)) {
        // Stay false
        return reverse_waiting;
    }
    else if (!reverse_waiting && (forward ^ last_direction)) {
        // Set waiting = true, set reverse_start to time
        reverse_waiting = true;
        reverse_start = millis();
        return reverse_waiting;
    } else if (reverse_waiting && (millis() - reverse_start < reverse_timeout)) {
        // Stay true
        return reverse_waiting;
    } else if (reverse_waiting && (millis() - reverse_start >= reverse_timeout)) {
        // Set last_direction, clear reverse_start, reverse_waiting = false
        reverse_waiting = false;
        last_direction = forward;
        reverse_start = 0;
        return reverse_waiting;
    }
}

uint16_t last_servo_input = 0;
static const uint16_t servo_input_timeout = 100;

uint32_t pw_center = 0;
uint16_t pw_max = 2000;
uint16_t pw_min = 1000;
uint16_t pw_deadzone = 10;

void center_setup() {
  uint16_t loops = 128;
  const uint8_t loops_right_shift = 7;
  pw_center = 0;
  while (loops > 0) {
    uint32_t pw = pulseIn(4, HIGH, servo_input_timeout * 1000);
    if (pw >= pw_min && pw <= pw_max) {
      pw_center += pw;
      loops --;
    }
  }
  pw_center >>= loops_right_shift;
  brake_print_pw(pw_center);
  // Set pw_max and pw_min
  pw_max = pw_center * 19 / 15;
  pw_min = pw_center * 11 / 15;
}

void loop_pwm_test() {
  for (uint8_t i = 0; i < pwm_max; i++) {
    pwm_write(i);
    delay(10);
  }
}

void brake_print_pw(uint16_t pw) {
  uint16_t digit_separate_delay = 500;
  uint16_t digit_show_delay = 100;
  uint8_t digits[5];
  digits[0] = (pw / 10000) % 10;
  digits[1] = (pw / 1000) % 10;
  digits[2] = (pw / 100) % 10;
  digits[3] = (pw / 10) % 10;
  digits[4] = pw % 10;
  for (uint8_t di = 0; di < 5; di++) {
    if (digits[di] == 0) {
      digitalWrite(1, HIGH);
      delay(10);
      digitalWrite(1, LOW);
      delay(digit_separate_delay);
      continue;
    }
    uint8_t si = digits[di];
    while (si -- > 0) {
      digitalWrite(1, HIGH);
      delay(digit_show_delay);
      digitalWrite(1, LOW);
      delay(digit_show_delay);
    }
    delay(digit_separate_delay);
  }
  pwm_setup();
}


void loop() {
  int32_t pw = pulseIn(4, HIGH, servo_input_timeout * 1000);
  if (pw > 0) {
    last_servo_input = millis();
    // Check if within deadzone
    if ((pw > pw_center - pw_deadzone) && (pw < pw_center + pw_deadzone)) {
      // Within deadzone, brake
      pwm_write(0);
      brake();
    } else if (pw > pw_center + pw_deadzone) {
      // Forward
      // Check switch direction
      if (check_reverse(true)) {
        // Wait in brake
        pwm_write(0);
        brake();
      } else {
        // Calculate output pwm
        int32_t out = (pw - (pw_center + pw_deadzone));
        out *= pwm_max;
        out /= (pw_max - (pw_center + pw_deadzone));
        if (out >= 255) {
            out = 255;
        }
        if (out <= 0) {
          out = 0;
        }
        uint8_t pwm_out = out;
        pwm_write(pwm_out);
        forward();
      }
    } else if (pw < pw_center - pw_deadzone) {
      // Backward
      // Check switch direction
      if (check_reverse(false)) {
        // Wait in brake
        pwm_write(0);
        brake();
      } else {
        // Calculate output pwm
        int32_t out = ((pw_center - pw_deadzone) - pw);
        out *= pwm_max;
        out /= ((pw_center - pw_deadzone) - pw_min);
        if (out >= 255) {
            out = 255;
        }
        if (out <= 0) {
          out = 0;
        }
        uint8_t pwm_out = out;
        pwm_write(pwm_out);
        backward();
      }
    }
  } else {
    // Check servo timeout
    if ((millis() - last_servo_input) > servo_input_timeout) {
      pwm_write(0);
      brake();
    } else {
    // Keep going
    }
  }
}
