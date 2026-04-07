//src/srclib/max30102_i2c.c

#include "max30102_i2c.h"

static float bpm_sum = 0, spo2_sum = 0;
static float bpm_avg_buffer[AVG_SIZE] = {0};
static float spo2_avg_buffer[AVG_SIZE] = {0};

volatile dc_t dc_red = {0.90f, 0, 0, 0};
volatile dc_t dc_ir = {0.90f, 0, 0, 0};
volatile pulse_time_t p_time = {0, 0, 0};

void timer0_init(void) {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 |= (1 << OCIE0A);
    sei();
}


ISR(TIMER0_COMPA_vect) {
    p_time.current_ms++;
}


uint8_t max30102_init() {
    timer0_init();
    i2c_init();
    max30102_reset();
    _delay_ms(1000);

    return max30102_write_reg(MAX30102_REG_INTR_ENABLE_1, 0xc0) &&
           max30102_write_reg(MAX30102_REG_INTR_ENABLE_2, 0x00) &&
           max30102_write_reg(MAX30102_REG_FIFO_WR_PTR, 0x00) &&
           max30102_write_reg(MAX30102_REG_OVF_COUNTER, 0x00) &&
           max30102_write_reg(MAX30102_REG_FIFO_RD_PTR, 0x00) &&
           max30102_write_reg(MAX30102_REG_FIFO_CONFIG, 0x4f) &&
           max30102_write_reg(MAX30102_REG_MODE_CONFIG, 0x03) &&
           max30102_write_reg(MAX30102_REG_SPO2_CONFIG, 0x27) &&
           max30102_write_reg(MAX30102_REG_LED1_PA, 0x24) &&
           max30102_write_reg(MAX30102_REG_LED2_PA, 0x24) &&
           max30102_write_reg(MAX30102_REG_PILOT_PA, 0x7f);
}


uint8_t max30102_write_reg(uint8_t reg_addr, uint8_t data) {
    i2c_start();
    i2c_write(MAX30102_I2C_WRITE_ADDR);
    i2c_write(reg_addr);
    i2c_write(data);
    i2c_stop();

    return 1;
}


uint8_t max30102_read_reg(uint8_t reg_addr, uint8_t* data) {
    i2c_start();
    i2c_write(MAX30102_I2C_WRITE_ADDR);
    i2c_write(reg_addr);
    i2c_stop();

    i2c_start();
    i2c_write(MAX30102_I2C_READ_ADDR);
    *data = i2c_read_nack();
    i2c_stop();

    return 1;
}


uint8_t max30102_read_fifo(uint32_t* red_led, uint32_t* ir_led) {
    uint8_t uch_temp = 0;

    *red_led = 0;
    *ir_led = 0;

    max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &uch_temp);
    max30102_read_reg(MAX30102_REG_INTR_STATUS_2, &uch_temp);

    i2c_start();
    i2c_write(MAX30102_I2C_WRITE_ADDR);
    i2c_write(MAX30102_REG_FIFO_DATA);
    i2c_stop();

    i2c_start();
    i2c_write(MAX30102_I2C_READ_ADDR);

    *red_led |= ((uint32_t)i2c_read_ack() << 16);
    *red_led |= ((uint32_t)i2c_read_ack() << 8);
    *red_led |= (uint32_t)i2c_read_ack();

    *ir_led |= ((uint32_t)i2c_read_ack() << 16);
    *ir_led |= ((uint32_t)i2c_read_ack() << 8);
    *ir_led |= (uint32_t)i2c_read_nack();

    i2c_stop();

    *red_led &= 0x0003FFFF;
    *ir_led &= 0x0003FFFF;

    return 1;
}


void max30102_calculate_heart_and_spo2(uint32_t red_raw, uint32_t ir_raw, float* heart_bps, float* spo2_data) {
    static uint8_t bpm_avg_idx = 0;
    static uint8_t spo2_avg_idx = 0;
    static float prev_ac_red = 0;
    static float prev_prev_ac_red = 0;
    static uint8_t detect_state = 0;
    static float red_ac_avg = 0;
    static float ir_ac_avg = 0;
    static uint8_t bpm_initialized = 0;
    static uint8_t spo2_initialized = 0;

    if ((red_raw + ir_raw) < FINGER_THRESHOLD) {
        dc_red.dc_val = 0;
        dc_ir.dc_val = 0;
        red_ac_avg = 0;
        ir_ac_avg = 0;
        prev_ac_red = 0;
        prev_prev_ac_red = 0;
        *heart_bps = 0;
        *spo2_data = 0;
        detect_state = 0;
        bpm_sum = 0;
        spo2_sum = 0;
        bpm_initialized = 0;
        spo2_initialized = 0;
        p_time.last_peak_ms = 0;
        for (uint8_t i = 0; i < AVG_SIZE; i++) {
            bpm_avg_buffer[i] = 0;
            spo2_avg_buffer[i] = 0;
        }
        return;
    }

    if (dc_red.dc_val == 0) {
        dc_red.dc_val = (float)red_raw;
        dc_ir.dc_val = (float)ir_raw;
    }

    dc_red.dc_val += (1.0f - dc_red.alpha) * ((float)red_raw - dc_red.dc_val);
    dc_ir.dc_val += (1.0f - dc_ir.alpha) * ((float)ir_raw - dc_ir.dc_val);

    dc_red.ac_val = dc_red.dc_val - (float)red_raw;
    dc_ir.ac_val = dc_ir.dc_val - (float)ir_raw;

    if (p_time.last_peak_ms != 0 && (p_time.current_ms - p_time.last_peak_ms) > 2500) {
        detect_state = 0;
        p_time.last_peak_ms = 0;
    }

    if (detect_state == 0) {
        if (prev_ac_red > prev_prev_ac_red && prev_ac_red > dc_red.ac_val && prev_ac_red > PEAK_THRESHOLD) {
            uint32_t interval = p_time.current_ms - p_time.last_peak_ms;
            if (p_time.last_peak_ms != 0 && interval > 300 && interval < 2000) {
                float raw_bpm = 60000.0f / (float)interval;
                if (!bpm_initialized) {
                    for (uint8_t i = 0; i < AVG_SIZE; i++) {
                        bpm_avg_buffer[i] = raw_bpm;
                    }
                    bpm_sum = raw_bpm * (float)AVG_SIZE;
                    bpm_initialized = 1;
                }
                bpm_sum -= bpm_avg_buffer[bpm_avg_idx];
                bpm_avg_buffer[bpm_avg_idx] = raw_bpm;
                bpm_sum += raw_bpm;
                *heart_bps = bpm_sum / (float)AVG_SIZE;
                bpm_avg_idx = (bpm_avg_idx + 1) % AVG_SIZE;
            }
            p_time.last_peak_ms = p_time.current_ms;
            detect_state = 1;
        }
    } else {
        if (dc_red.ac_val < 0) {
            detect_state = 0;
        }
    }

    prev_prev_ac_red = prev_ac_red;
    prev_ac_red = dc_red.ac_val;

    red_ac_avg = red_ac_avg * 0.90f + fabsf(dc_red.ac_val) * 0.10f;
    ir_ac_avg  = ir_ac_avg  * 0.90f + fabsf(dc_ir.ac_val)  * 0.10f;

    if (dc_ir.dc_val > 0 && dc_red.dc_val > 0 && ir_ac_avg > 1.5f) {
        float R = (red_ac_avg * dc_ir.dc_val) / (ir_ac_avg * dc_red.dc_val);
        float raw_spo2 = -45.060f * R * R + 30.354f * R + 94.845f;
        if (raw_spo2 > 100.0f) raw_spo2 = 100.0f;
        if (raw_spo2 < 70.0f)  raw_spo2 = 70.0f;

        if (!spo2_initialized) {
            for (uint8_t i = 0; i < AVG_SIZE; i++) {
                spo2_avg_buffer[i] = raw_spo2;
            }
            spo2_sum = raw_spo2 * (float)AVG_SIZE;
            spo2_initialized = 1;
        }
        spo2_sum -= spo2_avg_buffer[spo2_avg_idx];
        spo2_avg_buffer[spo2_avg_idx] = raw_spo2;
        spo2_sum += raw_spo2;
        *spo2_data = spo2_sum / (float)AVG_SIZE;
        spo2_avg_idx = (spo2_avg_idx + 1) % AVG_SIZE;
    }
}

uint8_t max30102_read_temperature(int8_t* integer_part, uint8_t* fractional_part) {
    max30102_write_reg(MAX30102_REG_TEMP_CONFIG, 0x01); // enable temp_en
    _delay_ms(1);

    uint8_t temp = 0;
    max30102_read_reg(MAX30102_REG_TEMP_INTR, &temp);
    *integer_part = temp;
    max30102_read_reg(MAX30102_REG_TEMP_FRAC, fractional_part);

    return 1;
}


uint8_t max30102_reset() {
    return max30102_write_reg(MAX30102_REG_MODE_CONFIG, MAX30102_CONFIG_RESET);
}