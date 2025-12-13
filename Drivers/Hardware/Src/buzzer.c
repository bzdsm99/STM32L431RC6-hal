#include "buzzer.h"
#include "timer.h"
#include "sys.h"
#include "delay.h"

// 音符频率定义 (单位: Hz)
#define NOTE_B3  247  // 添加缺失的B3
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_F4s 370   // 升F4 (F#4)
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1046

// 音符时值定义 (毫秒)
#define WHOLE_NOTE        1000
#define HALF_NOTE         500
#define HALF_NOTE_PLUS    750    // 附点二分音符
#define DOTTED_QUARTER    375    // 附点四分音符
#define QUARTER_NOTE      250
#define EIGHTH_NOTE       125

/**
 * @brief 初始化蜂鸣器
 */
void buzzer_init(void) {
    // 初始化蜂鸣器引脚为PWM输出模式
    // 使用TIM1通道4 (PA11) 输出PWM
    // 频率为1kHz，初始占空比为0%
    atim1_npwmStart_init(1000-1, 80-1);  // ARR=999, PSC=79 => 1kHz PWM
    timx_pwmSetCompare(TIM1, TIM_CHANNEL_4, 0); // 初始占空比为0，蜂鸣器不响
}

/**
 * @brief 播放指定频率和持续时间的声音
 * @param frequency 频率(Hz)
 * @param duration 持续时间(毫秒)
 * @param volume 音量(占空比百分比 0-100)
 */
void buzzer_play_tone(uint16_t frequency, uint16_t duration, uint8_t volume) {
    if (frequency == 0) {
        // 静音
        timx_pwmSetCompare(TIM1, TIM_CHANNEL_4, 0);
        delay_ms(duration);
        return;
    }
    
    // 计算预分频器和自动重装载值
    // 频率 = 80000000 / ((psc + 1) * (arr + 1))
    uint16_t psc = 79;  // 预分频器
    uint16_t arr = 1000000/frequency - 1;  // 自动重装载值
    
    // 对于高频情况，需要调整预分频器
    if (arr < 10) {
        psc = 7;
        arr = 10000000/frequency - 1;
    }
    
    // 重新初始化定时器
    atim1_npwmStart_init(arr, psc);
    
    // 设置占空比(音量)
    uint16_t duty_cycle = (arr * volume) / 100;
    timx_pwmSetCompare(TIM1, TIM_CHANNEL_4, duty_cycle);
    
    // 持续一段时间
    delay_ms(duration);
    
    // 停止发声
    timx_pwmSetCompare(TIM1, TIM_CHANNEL_4, 0);
    delay_ms(10); // 短暂静音避免音符粘连
}

/**
 * @brief 开机音效
 */
void buzzer_play_poweron_sound(void) {
    // 科技感开机音效 - 从低频到高频的扫频效果，然后是一系列短促有力的音符
    // 扫频效果：从200Hz快速上升到1200Hz
    for (uint16_t freq = 200; freq <= 1200; freq += 100) {
        buzzer_play_tone(freq, 25, 15);
    }
    
    // 延迟一小段时间
    delay_ms(50);
    
    // 快速的数字感音符
    const uint16_t poweron_notes[] = {
        NOTE_G5, NOTE_C6, NOTE_G5, NOTE_C6, NOTE_G5
    };
    
    const uint16_t poweron_durations[] = {
        100, 100, 100, 200, 300
    };
    
    const uint8_t note_count = sizeof(poweron_notes) / sizeof(poweron_notes[0]);
    
    for (int i = 0; i < note_count; i++) {
        buzzer_play_tone(poweron_notes[i], poweron_durations[i], 20);
    }
    
    // 结束音符
    buzzer_play_tone(NOTE_C6, 500, 25);
}

/**
 * @brief 播放完整版《起风了》主题旋律
 * @note 这是歌曲的主要部分，采用舒缓的长音演奏
 */
void buzzer_play_wind_rises(void) {
    // 旋律音符序列 (完整主歌部分)
    const uint16_t melody_notes[] = {
        // 第一句
        NOTE_G4, NOTE_E4, NOTE_G4, NOTE_E4, 
        NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4,
        NOTE_E4, NOTE_D4, NOTE_E4, NOTE_G4,
        NOTE_A4, NOTE_G4, NOTE_F4s, NOTE_E4,
        
        // 第二句  
        NOTE_D4, NOTE_E4, NOTE_G4, NOTE_E4,
        NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
        NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4,
        NOTE_G4, NOTE_E4, NOTE_D4, NOTE_C4,
        
        // 第三句 (高潮部分)
        NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4,
        NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4,
        NOTE_A4, NOTE_G4, NOTE_F4s, NOTE_E4,
        NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4,
        
        // 结尾句
        NOTE_G4, NOTE_E4, NOTE_G4, NOTE_E4,
        NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4,
        NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B3,
        NOTE_C4, 0, 0, 0  // 最后三个0表示休止符
    };
    
    // 对应时值序列 (使用舒缓的节奏)
    const uint16_t melody_durations[] = {
        // 第一句
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE,
        HALF_NOTE_PLUS, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, HALF_NOTE_PLUS,
        
        // 第二句
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, HALF_NOTE_PLUS,
        
        // 第三句
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, HALF_NOTE_PLUS,
        
        // 结尾句
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE,
        HALF_NOTE_PLUS, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        HALF_NOTE_PLUS, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE
    };
    
    // 音量序列 (根据情感变化调整)
    const uint8_t melody_volumes[] = {
        // 第一句: 轻柔起奏
        5, 6, 7, 6, 8, 7, 7, 6, 6, 5, 6, 7, 8, 7, 6, 5,
        // 第二句: 稍微加强
        6, 7, 8, 7, 8, 8, 9, 8, 8, 7, 7, 6, 7, 6, 5, 4,
        // 第三句: 高潮部分
        7, 8, 9, 9, 10, 9, 8, 7, 8, 7, 6, 5, 6, 5, 6, 5,
        // 结尾句: 渐弱收尾
        6, 5, 6, 5, 8, 7, 6, 5, 5, 4, 3, 2, 4, 0, 0, 0
    };
    
    const uint8_t total_notes = sizeof(melody_notes) / sizeof(melody_notes[0]);
    
    // 播放旋律
    for (int i = 0; i < total_notes; i++) {
        if (melody_notes[i] == 0) {
            // 休止符
            delay_ms(melody_durations[i]);
        } else {
            buzzer_play_tone(melody_notes[i], melody_durations[i], melody_volumes[i]);
        }
        
        // 音符间间隔 (除了休止符外)
        if (melody_notes[i] != 0 && i < total_notes - 1 && melody_notes[i + 1] != 0) {
            delay_ms(30); // 短间隔，让音符清晰分离
        }
    }
}

/**
 * @brief 播放完整版《小星星》
 */
void buzzer_play_twinkle_star(void) {
    // 旋律音符序列 (C大调标准小星星)
    const uint16_t melody_notes[] = {
        // 第一段
        NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
        NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4,
        NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
        NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
        // 第二段
        NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
        NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
    };

    // 对应时值序列 (四分音符+二分音符)
    const uint16_t melody_durations[] = {
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE,
        QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, HALF_NOTE
    };

    // 统一音量设置
    const uint8_t melody_volumes[] = {
        5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5
    };

    const uint8_t total_notes = sizeof(melody_notes) / sizeof(melody_notes[0]);

    // 播放旋律
    for (int i = 0; i < total_notes; i++) {
        if (melody_notes[i] == 0) {
            delay_ms(melody_durations[i]);
        } else {
            buzzer_play_tone(melody_notes[i], melody_durations[i], melody_volumes[i]);
        }

        // 音符间间隔
        if (melody_notes[i] != 0 && i < total_notes - 1 && melody_notes[i + 1] != 0) {
            delay_ms(30);
        }
    }
}

/**
 * @brief 播放完整的《起风了》全曲 (包含前奏、主歌、副歌、间奏、结尾)
 * @note 这是最完整的版本，适合演示或长时间播放使用
 */
void buzzer_play_wind_rises_full(void) {
    // 第一部分：前奏
    buzzer_play_tone(NOTE_G4, 800, 5);
    buzzer_play_tone(NOTE_E4, 400, 6);
    buzzer_play_tone(NOTE_G4, 800, 6);
    buzzer_play_tone(NOTE_E4, 400, 5);
    delay_ms(200);
    
    // 第二部分：主歌 (播放完整的56音符旋律)
    buzzer_play_wind_rises();
    
    // 第三部分：间奏
    delay_ms(500);
    buzzer_play_tone(NOTE_C5, 600, 7);
    buzzer_play_tone(NOTE_B4, 400, 6);
    buzzer_play_tone(NOTE_A4, 800, 6);
    buzzer_play_tone(NOTE_G4, 1200, 5);
    
    // 第四部分：副歌
    const uint16_t chorus_notes[] = {
        NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4,
        NOTE_A4, NOTE_G4, NOTE_F4s, NOTE_E4,
        NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4,
        NOTE_G4, NOTE_E4, NOTE_C4, 0
    };
    
    const uint16_t chorus_durations[] = {
        QUARTER_NOTE, EIGHTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE,
        HALF_NOTE_PLUS, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        DOTTED_QUARTER, EIGHTH_NOTE, QUARTER_NOTE, QUARTER_NOTE,
        HALF_NOTE_PLUS, EIGHTH_NOTE, WHOLE_NOTE, QUARTER_NOTE
    };
    
    for (int i = 0; i < 15; i++) {
        if (chorus_notes[i] != 0) {
            buzzer_play_tone(chorus_notes[i], chorus_durations[i], 8);
        } else {
            delay_ms(chorus_durations[i]);
        }
        delay_ms(30);
    }
    
    // 结尾：渐弱结束
    buzzer_play_tone(NOTE_C4, 2000, 3);
}




