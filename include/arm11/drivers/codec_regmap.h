#pragma once


// Based on Texas Instruments TSC2117 SLAS550B.
// Adjusted for the custom Texas Instruments PAIC3010B, AIC3010B, AIC3010D
// and possibly other variants.



enum
{
	CDC_REG_PAGE_CTRL              =   0u, // Available on every page.

	// ----------------------------------------------------------------------------------
	// Control Registers, Page 0 (Default Page):
	// Clock Multipliers, Dividers, Serial Interfaces, Flags, Interrupts, and GPIOs.
	CDC_REG_SOFT_RST_TWL           =   1u,
	CDC_REG_0_2                    =   2u, // Vendor and device ID?
	CDC_REG_0_3                    =   3u, // Revision?
	CDC_REG_CLK_GEN_MUXING         =   4u,
	CDC_REG_PLL_P_R_VAL            =   5u,
	CDC_REG_PLL_J_VAL              =   6u,
	CDC_REG_PLL_D_VAL_MSB          =   7u,
	CDC_REG_PLL_D_VAL_LSB          =   8u,
	// 9-10 reserved.
	CDC_REG_DAC_NDAC_VAL           =  11u,
	CDC_REG_DAC_MDAC_VAL           =  12u,
	// 13 DAC DOSR_VAL MSB?
	// 14 DAC DOSR_VAL LSB?
	// 15 DAC IDAC_VAL?
	// 16 DAC miniDSP Engine Interpolation?
	// 17 reserved.
	CDC_REG_ADC_NADC_VAL           =  18u,
	CDC_REG_ADC_MADC_VAL           =  19u,
	// 20 ADC AOSR_VAL?
	// 21 ADC IADC_VAL?
	// 22 ADC miniDSP Engine Decimation?
	// 23-24 reserved.
	// 25 CLKOUT MUX?
	// 26 CLKOUT M_VAL?
	CDC_REG_INTERFACE_CTRL         =  27u, // Audio
	// 28 Data-Slot Offset Programmability?
	// 29 Codec Interface Control 2?
	// 30  BCLK N_VAL?
	// 31 Codec Secondary Interface Control 1?
	// 32 Codec Secondary Interface Control 2?
	// 33 Codec Secondary Interface Control 3?
	CDC_REG_I2C_BUS_COND           =  34u, // I2C Bus Condition.
	// 35 reserved.
	CDC_REG_ADC_FLAG               =  36u,
	CDC_REG_DAC_FLAG1              =  37u, // TODO: Validate.
	CDC_REG_DAC_FLAG2              =  38u, // TODO: Validate.
	CDC_REG_OVERFLOW_FLAGS         =  39u,
	// 40-43 reserved.
	CDC_REG_INT_FLAGS_DAC          =  44u, // Interrupt Flags—DAC.
	CDC_REG_INT_FLAGS_ADC          =  45u, // Interrupt Flags—ADC.
	// 46 Interrupt Flags – DAC? Same as 44?
	// 47 Interrupt Flags – ADC? Same as 45?
	// 48 INT1 Control Register?
	// 49 INT2 Control Register?
	CDC_REG_INT1_INT2_CTRL         =  50u, // TODO: Validate.
	CDC_REG_GPIO1_INOUT_PIN_CTRL   =  51u, // TODO: Validate.
	CDC_REG_GPIO2_INOUT_PIN_CTRL   =  52u, // TODO: Validate.
	CDC_REG_SDOUT_PIN_CTRL         =  53u, // TODO: Validate.
	// 54 SDIN (IN Pin) Control?
	// 55 MISO (OUT Pin) Control?
	// 56  SCLK (IN Pin) Control?
	CDC_REG_GPI1_GPI2_PIN_CTRL     =  57u, // TODO: Validate.
	CDC_REG_GPI3_PIN_CTRL          =  58u, // TODO: Validate.
	// 59 reserved.
	CDC_REG_DAC_INSTR_SET          =  60u, // DAC Instruction Set.
	CDC_REG_ADC_INSTR_SET          =  61u, // ADC Instruction Set.
	// 62 Programmable Instruction Mode-Control Bits?
	CDC_REG_DAC_DATA_PATH_SETUP    =  63u, // I2S1?
	CDC_REG_DAC_VOLUME_CTRL        =  64u, // I2S1?
	CDC_REG_DAC_L_VOLUME_CTRL      =  65u, // I2S1?
	CDC_REG_DAC_R_VOLUME_CTRL      =  66u, // I2S1?
	// 67 Headset Detection?
	// 68 DRC Control 1?
	// 69 DRC Control 2?
	// 70 DRC Control 3?
	CDC_REG_L_BEEP_GEN             =  71u, // Left Beep Generator.
	CDC_REG_R_BEEP_GEN             =  72u, // Right Beep Generator.
	CDC_REG_BEEP_LEN_MSB           =  73u,
	CDC_REG_BEEP_LEN_MID_BITS      =  74u, // Beep Length Middle Bits.
	CDC_REG_BEEP_LEN_LSB           =  75u,
	CDC_REG_BEEP_SIN_X_MSB         =  76u, // Beep Sin(x) MSB.
	CDC_REG_BEEP_SIN_X_LSB         =  77u, // Beep Sin(x) LSB.
	CDC_REG_BEEP_COS_X_MSB         =  78u, // Beep Cos(x) MSB.
	CDC_REG_BEEP_COS_X_LSB         =  79u, // Beep Cos(x) LSB.
	// 80 reserved.
	CDC_REG_ADC_DIGITAL_MIC        =  81u, // TODO: Validate.
	CDC_REG_ADC_DIG_VOL_FINE_ADJ   =  82u, // TODO: Validate. ADC Digital Volume Control Fine Adjust.
	// 83 ADC Digital Volume Control Coarse Adjust?
	// 84-85 reserved.
	CDC_REG_AGC_CTRL1              =  86u,
	CDC_REG_AGC_CTRL2              =  87u,
	CDC_REG_AGC_MAX_GAIN           =  88u,
	CDC_REG_AGC_ATTACK_TIME        =  89u,
	CDC_REG_AGC_DECAY_TIME         =  90u,
	CDC_REG_AGC_NOISE_DEBOUNCE     =  91u,
	CDC_REG_AGC_SIGNAL_DEBOUNCE    =  92u,
	CDC_REG_AGC_GAIN_APPLIED       =  93u,
	// 94-101 reserved.
	// 102 ADC DC Measurement 1?
	// 103 ADC DC Measurement 2?
	// 104 ADC DC Measurement Output 1?
	// 105 ADC DC Measurement Output 2?
	// 106 ADC DC Measurement Output 3?
	// 107-115 reserved.
	CDC_REG_VOL_MICDET_PIN_SAR_ADC = 116u, // TODO: Validate.
	// 117 VOL/MICDET-Pin Gain?
	// 118-127 reserved.

	// ----------------------------------------------------------------------------------
	// Control Registers, Page 1:
	// DAC and ADC Routing, PGA, Power-Controls and MISC Logic Related Programmabilities:
	// 1-29 reserved.
	// 30 Headphone and Speaker Amplifier Error Control?
	CDC_REG_HEADPHONE_DRIVERS         = 1u<<8 |  31u,
	CDC_REG_CLASS_D_SPEAKER_AMP       = 1u<<8 |  32u, // TODO: Validate.
	CDC_REG_HP_POP_REM_SETTINGS       = 1u<<8 |  33u, // HP Output Drivers POP Removal Settings.
	CDC_REG_OUT_PGA_RD_PERIOD_CTRL    = 1u<<8 |  34u, // Output Driver PGA Ramp-Down Period Control.
	CDC_REG_DAC_LR_OUT_MIX_ROUTING    = 1u<<8 |  35u, // DAC_L and DAC_R Output Mixer Routing.
	CDC_REG_L_ANALOG_VOL_TO_HPL       = 1u<<8 |  36u,
	CDC_REG_R_ANALOG_VOL_TO_HPR       = 1u<<8 |  37u,
	CDC_REG_L_ANALOG_VOL_TO_SPL       = 1u<<8 |  38u,
	CDC_REG_R_ANALOG_VOL_TO_SPR       = 1u<<8 |  39u,
	CDC_REG_HPL_DRIVER                = 1u<<8 |  40u,
	CDC_REG_HPR_DRIVER                = 1u<<8 |  41u,
	CDC_REG_SPL_DRIVER                = 1u<<8 |  42u,
	CDC_REG_SPR_DRIVER                = 1u<<8 |  43u,
	// 44 HP Driver Control?
	// 45 reserved.
	CDC_REG_MICBIAS                   = 1u<<8 |  46u,
	CDC_REG_MIC_PGA                   = 1u<<8 |  47u, // TODO: Validate.
	CDC_REG_ADC_IN_SEL_FOR_P_TERMINAL = 1u<<8 |  48u, // TODO: Validate. Delta-Sigma Mono ADC Channel Fine-Gain Input Selection for P-Terminal.
	CDC_REG_ADC_IN_SEL_FOR_M_TERMINAL = 1u<<8 |  49u, // ADC Input Selection for M-Terminal.
	CDC_REG_INPUT_CM_SETTINGS         = 1u<<8 |  50u,
	// 51-127 reserved.

	// ----------------------------------------------------------------------------------
	// Control Registers, Page 3: TSC Control and Data Programmabilities.
	// 1 reserved.
	CDC_REG_SAR_ADC_CTRL           = 3u<<8 |   2u, // First ADC?
	CDC_REG_SAR_ADC_CTRL2          = 3u<<8 |   3u, // TODO: Validate.
	CDC_REG_PRECHARGE_AND_SENSE    = 3u<<8 |   4u,
	CDC_REG_PANEL_VOLT_STABIL      = 3u<<8 |   5u, // Panel Voltage Stabilization.
	CDC_REG_VOLT_REF               = 3u<<8 |   6u, // TODO: Validate. Voltage Reference.
	// 7-8 reserved.
	CDC_REG_STATUS_BIT             = 3u<<8 |   9u,
	CDC_REG_STATUS_BIT2            = 3u<<8 |  10u, // TODO: Validate.
	// 11-12 reserved.
	// 13 Buffer Mode?
	CDC_REG_RESERVED_3_14          = 3u<<8 |  14u, // Buffer mode moved here?
	CDC_REG_SCAN_MODE_TIMER        = 3u<<8 |  15u,
	CDC_REG_SCAN_MODE_TIMER_CLK    = 3u<<8 |  16u,
	CDC_REG_SAR_ADC_CLK            = 3u<<8 |  17u,
	CDC_REG_DEBOUNCE_TIME_PEN_UP   = 3u<<8 |  18u, // Debounce Time for Pen-Up Detection.
	CDC_REG_AUTO_AUX_MEASURE_SEL   = 3u<<8 |  19u, // Auto AUX Measurement Selection.
	CDC_REG_TOUCH_PEN_DOWN         = 3u<<8 |  20u, // TODO: Validate. Touch-Screen Pen Down.
	CDC_REG_TRESHOLD_CHECK_FLAGS   = 3u<<8 |  21u,
	// 22 AUX1 Maximum Value Check (MSB)?
	// 23 AUX1 Maximum Value Check (LSB)?
	// 24 AUX1 Minimum Value Check (MSB)?
	// 25 AUX1 Minimum Value Check (LSB)?
	// 26 AUX2 Maximum Value Check (MSB)?
	// 27 AUX2 Maximum Value Check (LSB)?
	// 28 AUX2 Minimum Value Check (MSB)?
	// 29 AUX2 Minimum Value Check (LSB)?
	CDC_REG_TEMP_MAX_VAL_CHECK_MSB = 3u<<8 |  30u, // Temperature Maximum Value Check (MSB).
	CDC_REG_TEMP_MAX_VAL_CHECK_LSB = 3u<<8 |  31u, // Temperature Maximum Value Check (LSB).
	CDC_REG_TEMP_MIN_VAL_CHECK_MSB = 3u<<8 |  32u, // Temperature Minimum Value Check (MSB).
	CDC_REG_TEMP_MIN_VAL_CHECK_LSB = 3u<<8 |  33u, // Temperature Minimum Value Check (LSB).
	// 34-41 reserved.
	CDC_REG_X_COORDINATE_DATA_MSB  = 3u<<8 |  42u,
	CDC_REG_X_COORDINATE_DATA_LSB  = 3u<<8 |  43u,
	CDC_REG_Y_COORDINATE_DATA_MSB  = 3u<<8 |  44u,
	CDC_REG_Y_COORDINATE_DATA_LSB  = 3u<<8 |  45u,
	// 46 Z1 MSB Register?
	// 47 Z1 LSB Register?
	// 48 Z2 MSB Register?
	// 49 Z2 LSB Register
	// 50-53 reserved.
	CDC_REG_AUX1_DATA_MSB          = 3u<<8 |  54u, // TODO: Validate.
	CDC_REG_AUX1_DATA_LSB          = 3u<<8 |  55u, // TODO: Validate.
	CDC_REG_AUX2_DATA_MSB          = 3u<<8 |  56u, // TODO: Validate.
	CDC_REG_AUX2_DATA_LSB          = 3u<<8 |  57u, // TODO: Validate.
	CDC_REG_VBAT_DATA_MSB          = 3u<<8 |  58u, // TODO: Validate.
	CDC_REG_VBAT_DATA_LSB          = 3u<<8 |  59u, // TODO: Validate.
	// 60-65 reserved.
	CDC_REG_TEMP1_MSB_DATA         = 3u<<8 |  66u,
	CDC_REG_TEMP1_LSB_DATA         = 3u<<8 |  67u,
	CDC_REG_TEMP2_MSB_DATA         = 3u<<8 |  68u,
	CDC_REG_TEMP2_LSB_DATA         = 3u<<8 |  69u,
	// 70-127 reserved.

	// ----------------------------------------------------------------------------------
	// Page 4.

	// ----------------------------------------------------------------------------------
	// Page 5.

	// ----------------------------------------------------------------------------------
	// Page 8.

	// ----------------------------------------------------------------------------------
	// Page 9.

	// ----------------------------------------------------------------------------------
	// Page 10.

	// ----------------------------------------------------------------------------------
	// Page 11.

	// ----------------------------------------------------------------------------------
	// Page 12.

	// ----------------------------------------------------------------------------------
	// Page 100 (only CTR).
	CDC_REG_SOFT_RST_CTR = 100u<<8 |   1u,
	// 2-33 reserved?
	CDC_REG_100_34       = 100u<<8 |  34u,
	// 35-36 reserved?
	CDC_REG_100_37       = 100u<<8 |  37u,
	CDC_REG_100_38       = 100u<<8 |  38u,
	CDC_REG_100_39       = 100u<<8 |  39u,
	// 40-43 reserved?
	CDC_REG_100_44       = 100u<<8 |  44u,
	CDC_REG_100_48       = 100u<<8 |  48u,
	CDC_REG_100_49       = 100u<<8 |  49u,
	// 50-66 reserved?
	CDC_REG_100_67       = 100u<<8 |  67u,
	CDC_REG_100_68       = 100u<<8 |  68u,
	CDC_REG_HEADSET_SEL  = 100u<<8 |  69u,
	// 70-116 reserved?
	CDC_REG_100_117      = 100u<<8 | 117u,
	CDC_REG_100_118      = 100u<<8 | 118u,
	CDC_REG_100_119      = 100u<<8 | 119u,
	CDC_REG_100_120      = 100u<<8 | 120u,
	CDC_REG_100_121      = 100u<<8 | 121u,
	CDC_REG_100_122      = 100u<<8 | 122u,
	CDC_REG_100_123      = 100u<<8 | 123u,
	CDC_REG_100_124      = 100u<<8 | 124u,
	// 125-128 reserved?

	// ----------------------------------------------------------------------------------
	// Page 101 (only CTR).
	// 1-7 reserved?
	CDC_REG_101_8   = 101u<<8 |   8u,
	CDC_REG_101_9   = 101u<<8 |   9u,
	CDC_REG_101_10  = 101u<<8 |  10u,
	CDC_REG_101_11  = 101u<<8 |  11u,
	CDC_REG_101_12  = 101u<<8 |  12u,
	// 13-16 reserved?
	CDC_REG_101_17  = 101u<<8 |  17u,
	CDC_REG_101_18  = 101u<<8 |  18u,
	CDC_REG_101_19  = 101u<<8 |  19u,
	// 20-21 reserved?
	CDC_REG_101_22  = 101u<<8 |  22u,
	CDC_REG_101_23  = 101u<<8 |  23u,
	// 24-26 reserved?
	CDC_REG_101_27  = 101u<<8 |  27u,
	CDC_REG_101_28  = 101u<<8 |  28u,
	// 29-50 reserved?
	CDC_REG_101_51  = 101u<<8 |  51u,
	// 52-64 reserved?
	CDC_REG_101_65  = 101u<<8 |  65u,
	CDC_REG_101_66  = 101u<<8 |  66u,
	// 67-69 reserved?
	CDC_REG_101_70  = 101u<<8 |  70u,
	CDC_REG_101_71  = 101u<<8 |  71u,
	CDC_REG_101_72  = 101u<<8 |  72u,
	// 73-118 reserved?
	CDC_REG_101_119 = 101u<<8 | 119u,
	CDC_REG_101_120 = 101u<<8 | 120u,
	// 121 reserved?
	CDC_REG_101_122 = 101u<<8 | 122u,
	// 123-128 reserved?

	// ----------------------------------------------------------------------------------
	// Page 103 (only CTR).
	// 1-22 reserved?
	CDC_REG_103_23  = 103u<<8 |  23u,
	CDC_REG_103_24  = 103u<<8 |  24u,
	CDC_REG_103_25  = 103u<<8 |  25u,
	CDC_REG_103_26  = 103u<<8 |  26u,
	CDC_REG_103_27  = 103u<<8 |  27u,
	// 28-35 reserved?
	CDC_REG_103_36  = 103u<<8 |  36u,
	CDC_REG_103_37  = 103u<<8 |  37u,
	CDC_REG_103_38  = 103u<<8 |  38u,
	CDC_REG_103_39  = 103u<<8 |  39u,
	// 40-128 reserved?

	// ----------------------------------------------------------------------------------
	// Page 251 (only CTR).

	// ----------------------------------------------------------------------------------
	// Page 252.

	// ----------------------------------------------------------------------------------
	// Page 255 (only CTR).
	// 1 reserved?
	// 2 reserved?
	// 3 reserved?
	// 4 reserved?
	CDC_REG_TWL_MODE = 255<<8 | 5u
};


// ----------------------------------------------------------------------------------
// CDC_REG_HEADSET_SEL (page 100 (0x64), reg 69 (0x45))
#define HEADSET_SEL_HP_SHIFT   (4u)
#define HEADSET_SEL_SP         (0u)                        // Force speaker output.
#define HEADSET_SEL_HP         (1u<<HEADSET_SEL_HP_SHIFT)  // Force headphone output.
#define HEADSET_SEL_HP_EN      (1u<<5)                     // Enable headphone override.
#define HEADSET_SEL_MIC_SHIFT  (6u)
#define HEADSET_SEL_INT_MIC    (0u)                        // Force internal microphone input.
#define HEADSET_SEL_EXT_MIC    (1u<<HEADSET_SEL_MIC_SHIFT) // Force external microphone input.
#define HEADSET_SEL_MIC_EN     (1u<<7)                     // Enable microphone override.
