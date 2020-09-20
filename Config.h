#ifndef INCLUDE_Config_h_
#define INCLUDE_Config_h_

// 5.3ms latency
#define EXT_BLOCKLEN		(512)			/* only multiples of 512 */
#define ZEROS_TO_MUTE		(32)
#define SAMPLE_RATE			(96000)
#define MUTE_ENVELOPE_LEN	(96*2)
#define CW_IQ_TONE_OFFSET	(1000)

enum KeyerMode {
	KEYER_MODE_SK = 0,
	KEYER_MODE_IAMBIC_A = 1,
	KEYER_MODE_IAMBIC_B = 2,
};

#endif /* INCLUDE_Config_h_ */
