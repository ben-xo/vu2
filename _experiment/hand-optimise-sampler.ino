
#define SAMP_BUFF_LEN 128

struct Sampler {
  volatile uint8_t current_sample = 0;
  uint8_t last_processed_sample = 0;
  volatile uint16_t sample_sum; // (DC offset approximated by sample_sum / SAMP_BUFF_LEN)
  byte samples[SAMP_BUFF_LEN] = { 0 };
  byte beat_bitmap[SAMP_BUFF_LEN >> 3] = { 0 };
} sampler;

uint8_t new_sample = 0;

uint8_t reset_val = 0;
uint16_t fails = 0;

void reset_samples_to(uint8_t val)
{
	sampler.current_sample = 127;
	sampler.sample_sum = 0;
	for(uint8_t i = 0; i<SAMP_BUFF_LEN; i++)
	{
		sampler.samples[i] = val;
		sampler.sample_sum += val;
	}
}

void setup()
{
	  Serial.begin(1000000);
}

void c_sampler(uint8_t s)
{
	uint8_t sample_idx = (sampler.current_sample + 1) & ~SAMP_BUFF_LEN;
	uint8_t *ss = sampler.samples;
	sampler.current_sample = sample_idx;

	byte sample = s;
	byte* the_sample = ss + sample_idx;
	uint8_t old_sample_at_position = *the_sample;
	sampler.sample_sum = sampler.sample_sum - old_sample_at_position + sample;
	*the_sample = sample;
}

void asm1_sampler(uint8_t s)
{

	asm volatile (

  //// incremenent current_sample offset, then point Z to it

  // // uint8_t sample_idx = (current_sample + 1) & ~SAMP_BUFF_LEN;
    "lds r30, %[current_sample] \t\n"
    "subi  r30, 0xFF ; 255 \t\n"
    "andi  r30, 0x7F ; 127 \t\n"
  // // current_sample = sample_idx;
    "sts %[current_sample], r30 ; \t\n"   
  // // byte* the_sample = samples + sample_idx;
    "ldi r31, 0x00 ; 0 \t\n"
    "subi  r30, lo8(-(%[ss])) ; 128 \t\n"
    "sbci  r31, hi8(-(%[ss])) ; 252 \t\n"

  //// take a sample and put it in the sample buffer at the new offset.
  //// but also, read the old value so that we can subtract it from the sample sum.
  //// (ends up with the new sample in r30 and the old one in r24)

  // // byte sample = ADCH;
  // // uint8_t old_sample_at_position = *the_sample;
  // // *the_sample = sample;
    "lds r24, %[ADCH_addr] \t\n"
    "ld  r25, Z \t\n"
    "st  Z, r24 \t\n"

  //// subtract the old sample from the sum, and add the new one.
  //// doing this without using any new registers is tricky.
  //// if we can ONLY do subtracts, we can get away without using r1 (__zero_reg__)
  //// which helps as this is an interrupt handler so it's only more stuff to save.
  //// "sample_sum - old_sample_at_position + sample" is equivalent to
  //// "sample_sum - (old_sample_at_position - sample)"
  //// we also load the content of sample_sum incrementally being sure to preseve SREG

  // // sample_sum = sample_sum - old_sample_at_position + sample;

  //// r24 <- sample
  //// r25 <- old_sample_at_position
  //// r30 <- (unallocated)
  //// r31 <- (unallocated)

    "sub r24, r25 \t\n"
    "ldi r25, 0x00 \t\n"
    "sbci r25,  0x00 \t\n"

  //// r24 <- (old_sample_at_position - sample) *with SREG Carry*
  //// r30 <- (unallocated)
  //// r31 <- sample

    "lds r30, %[sample_sum]  \t\n"
    "lds r31, %[sample_sum]+0x1  \t\n"

  //// r24 <- (old_sample_at_position - sample) *with SREG Carry*
  //// r30:r31 <- sample_sum


    // TODO: CHECK ME

    // "sbci r31, 0x00 \t\n" // this should apply the carry from (old_sample_at_position - sample)

    "add r30, r24 \t\n"   // this should subtract (old_sample_at_position - sample) from sample_sum
    "adc r31, r25 \t\n"

    "sts %[sample_sum]+0x1, r31 \t\n"
    "sts %[sample_sum], r30 \t\n"
     :
     : 
     [ADCH_addr] "i" (&new_sample),
     [ss] "i" (&sampler.samples),
     [current_sample] "i" (&sampler.current_sample),
     [sample_sum] "i" (&sampler.sample_sum)
     :
     "r24", "r25", "r30", "r31"
   );
}

void asm2_sampler(uint8_t s)
{

	// the idea of asm1_sampler was to take the work that the GCC did and reduce the number of registers used
	// so that the push/pop load of an ISR is greatly reduced (bearing in mind that each extra reg adds 4 cycles total)

	// asm2_sampler tries to improve on this further by using Struct locality / Z displacement to cut down on full lds/sts (as lds and sts are 2x but ld/st 1x)

	// however this iteration is no better than asm1 in terms of cycles because we don't have enough registers to play with
	asm volatile (
		"ldi r30, lo8(%[ss]) \t\n"
		"ldi r31, hi8(%[ss]) \t\n"
		"ld r24, Z \t\n" // current_sample is offset 0
		"subi r24, 0xFF \t\n"
		"andi r24, 0x7F \t\n"
		"st Z, r24 \t\n"

		// instead of ldi / add / adc (which means we need a 0 reg), do add / brcc / subi (no reg needed)
		"add r30, r24 \t\n"
		// "ldi r25, 0x00 \t\n"
		// "adc r31, r25 \t\n"
		"brcc .+2 \t\n"
		"subi r31, 0xFF \t\n"

		"ldd r25, Z+4 \t\n"
    "lds r24, %[ADCH_addr] \t\n"
		"std Z+4, r24 \t\n"
    "lds r30, %[sample_sum]  \t\n"
    "lds r31, %[sample_sum]+0x1  \t\n"
    "sub r30, r25 \t\n"
    "sbci r31, 0x00 \t\n"
    "add r30, r24 \t\n"
    // "ldi r24, 0x00 \t\n"
    // "adc r31, r24 \t\n"
		"brcc .+2 \t\n"
		"subi r31, 0xFF \t\n"
    "sts %[sample_sum]+0x1, r31 \t\n"
    "sts %[sample_sum], r30 \t\n"
     :
     : 
     [ADCH_addr] "i" (&new_sample),
     [ss] "i" (&sampler.current_sample),
     [sample_sum] "i" (&sampler.sample_sum)
     :
     "r24", "r25", "r30", "r31"
   );
}

void asm3_sampler(uint8_t s)
{

	// building on asm2, make use of GPIO1 as a fast temporary stack for any single variable
	asm volatile (
		"ldi r30, lo8(%[ss]) \t\n"
		"ldi r31, hi8(%[ss]) \t\n"
		"ld r24, Z \t\n" // current_sample is offset 0
		"subi r24, 0xFF \t\n"
		"andi r24, 0x7F \t\n"
		"st Z, r24 \t\n"

		// instead of ldi / add / adc (which means we need a 0 reg), do add / brcc / subi (no reg needed)
		"add r30, r24 \t\n"
		// "ldi r25, 0x00 \t\n"
		// "adc r31, r25 \t\n"
		"brcc .+2 \t\n"
		"subi r31, 0xFF \t\n"

		"ldd r24, Z+4 \t\n"
		"out %[GPIOR1_addr], r24 \t\n"

    "lds r24, %[ADCH_addr] \t\n"
		"std Z+4, r24 \t\n"
    "lds r30, %[sample_sum]  \t\n"
    "lds r31, %[sample_sum]+0x1  \t\n"

    "add r30, r24 \t\n"
    // "ldi r24, 0x00 \t\n"
    // "adc r31, r24 \t\n"
		"brcc .+2 \t\n"
		"subi r31, 0xFF \t\n"

		"in r24, %[GPIOR1_addr] \t\n"
    "sub r30, r24 \t\n"
    "sbci r31, 0x00 \t\n"

    "sts %[sample_sum]+0x1, r31 \t\n"
    "sts %[sample_sum], r30 \t\n"
     :
     : 
		 [GPIOR1_addr] "I" (_SFR_IO_ADDR(GPIOR1)),
     [ADCH_addr] "i" (&new_sample),
     [ss] "i" (&sampler.current_sample),
     [sample_sum] "i" (&sampler.sample_sum)
     :
     "r24", "r30", "r31"
   );
}

void asm4_sampler(uint8_t s)
{

	// building on asm3, use only subtraction so that we can use sbci rN, 0x00 (1 cycle) instead of brcc / subi (2 cycles)

	//// subtract the old sample from the sum, and add the new one.
  //// doing this without using any new registers is tricky.
  //// if we can ONLY do subtracts, we can get away without using r1 (__zero_reg__)
  //// which helps as this is an interrupt handler so it's only more stuff to save.
  //// "sample_sum - old_sample_at_position + sample" is equivalent to
  //// "sample_sum - (old_sample_at_position - sample)"
  //// "sample_sum - (old_sample_at_position - sample)"


	asm volatile (
		"ldi r30, lo8(%[ss]) \t\n"
		"ldi r31, hi8(%[ss]) \t\n"
		"ld r24, Z \t\n" // current_sample is offset 0
		"subi r24, 0xFF \t\n"
		"andi r24, 0x7F \t\n"
		"st Z, r24 \t\n"

		// instead of ldi / add / adc (which means we need a 0 reg), do add / brcc / subi (no reg needed)
		"add r30, r24 \t\n"
		// "ldi r25, 0x00 \t\n"
		// "adc r31, r25 \t\n"
		"brcc .+2 \t\n"
		"subi r31, 0xFF \t\n"

		"ldd r24, Z+4 \t\n"
		"out %[GPIOR1_addr], r24 \t\n"

    "lds r24, %[ADCH_addr] \t\n"
		"std Z+4, r24 \t\n"

		// r24 = new sample

		"in r30, %[GPIOR1_addr] \t\n"

		// r24 = new sample
		// r30 = old sample

		"sub r30, r24 \t\n"

    "lds r31, %[sample_sum]+0x1  \t\n"
		"brcc .+2 \t\n"
		// carry
		"subi r31, 0xFF \t\n"
    "lds r24, %[sample_sum]  \t\n"
    "sub r24, r30 \t\n"
    "sbci r31, 0x00 \t\n"

    "sts %[sample_sum]+0x1, r31 \t\n"
    "sts %[sample_sum], r24 \t\n"
     :
     : 
		 [GPIOR1_addr] "I" (_SFR_IO_ADDR(GPIOR1)),
     [ADCH_addr] "i" (&new_sample),
     [ss] "i" (&sampler.current_sample),
     [sample_sum] "i" (&sampler.sample_sum)
     :
     "r24", "r30", "r31"
   );
}

void fail_uint8(char *err_str, uint8_t err_val, uint8_t expected)
{
	fails++;
	Serial.print("FAIL: ");
	Serial.print(err_str);
	Serial.print(" = ");
	Serial.print(err_val, HEX);
	Serial.print(", expected ");
	Serial.println(expected, HEX);
}

void fail_uint16(char *err_str, uint16_t err_val, uint16_t expected)
{
	fails++;
	Serial.print("FAIL: ");
	Serial.print(err_str);
	Serial.print(" = ");
	Serial.print(err_val, HEX);
	Serial.print(", expected ");
	Serial.print(expected, HEX);
	Serial.print(" with reset_val ");
	Serial.print(reset_val, HEX);
	Serial.print(" and new_sample ");
	Serial.println(new_sample, HEX);
}

void loop()
{
	// Serial.println("C:");
	// do {
	// 	Serial.println();
	// 	Serial.print("reset_val: ");
	// 	Serial.print(reset_val, HEX);

	// 	new_sample = 0;
	// 	do {
	// 		reset_samples_to(reset_val);

	// 		//Serial.print(".");

	// 		if(sampler.current_sample != 0) {
	// 			char err_str[] = "current_sample before";
	// 			fail_uint8(err_str, sampler.current_sample, 0);
	// 		}
	// 		if(sampler.samples[sampler.current_sample] != reset_val) {
	// 			char err_str[] = "samples[current_sample] before";
	// 			fail_uint8(err_str, sampler.samples[sampler.current_sample], reset_val);
	// 		}
	// 		if(sampler.sample_sum != reset_val * SAMP_BUFF_LEN) {
	// 			char err_str[] = "sample_sum before";
	// 			fail_uint16(err_str, sampler.sample_sum, reset_val * SAMP_BUFF_LEN);
	// 		}

	// 		c_sampler(new_sample);

	// 		if(sampler.current_sample != 1) {
	// 			char err_str[] = "current_sample after";
	// 			fail_uint8(err_str, sampler.current_sample, 1);
	// 		}
	// 		if(sampler.samples[sampler.current_sample] != new_sample) {
	// 			char err_str[] = "samples[current_sample] after";
	// 			fail_uint8(err_str, sampler.samples[sampler.current_sample], new_sample);
	// 		}
	// 		if(sampler.sample_sum != (reset_val * (SAMP_BUFF_LEN-1))+new_sample) {
	// 			char err_str[] = "sample_sum after";
	// 			fail_uint16(err_str, sampler.sample_sum, (reset_val * (SAMP_BUFF_LEN-1))+new_sample);
	// 		}
			

	// 		new_sample++;
	// 		if(fails) break;
	// 	} while(new_sample != 0);
	// 	reset_val++;
	// 	if(fails) break;
	// } while(reset_val != 0);


	// Serial.println();
	// Serial.print("Fails: ");
	// Serial.println(fails);

	// fails = 0;

	// Serial.println("ASM1:");
	// do {
	// 	Serial.println();
	// 	Serial.print("reset_val: ");
	// 	Serial.print(reset_val, HEX);

	// 	new_sample = 0;
	// 	do {
	// 		reset_samples_to(reset_val);

	// 		//Serial.print(".");

	// 		if(sampler.current_sample != 0) {
	// 			char err_str[] = "current_sample before";
	// 			fail_uint8(err_str, sampler.current_sample, 0);
	// 		}
	// 		if(sampler.samples[sampler.current_sample] != reset_val) {
	// 			char err_str[] = "samples[current_sample] before";
	// 			fail_uint8(err_str, sampler.samples[sampler.current_sample], reset_val);
	// 		}
	// 		if(sampler.sample_sum != reset_val * SAMP_BUFF_LEN) {
	// 			char err_str[] = "sample_sum before";
	// 			fail_uint16(err_str, sampler.sample_sum, reset_val * SAMP_BUFF_LEN);
	// 		}

	// 	 	asm1_sampler(new_sample);

	// 		if(sampler.current_sample != 1) {
	// 			char err_str[] = "current_sample after";
	// 			fail_uint8(err_str, sampler.current_sample, 1);
	// 		}
	// 		if(sampler.samples[sampler.current_sample] != new_sample) {
	// 			char err_str[] = "samples[current_sample] after";
	// 			fail_uint8(err_str, sampler.samples[sampler.current_sample], new_sample);
	// 		}
	// 		if(sampler.sample_sum != (reset_val * (SAMP_BUFF_LEN-1))+new_sample) {
	// 			char err_str[] = "sample_sum after";
	// 			fail_uint16(err_str, sampler.sample_sum, (reset_val * (SAMP_BUFF_LEN-1))+new_sample);
	// 		}
			

	// 		new_sample++;
	// 		if(fails) break;
	// 	} while(new_sample != 0);
	// 	reset_val++;
	// 	if(fails) break;
	// } while(reset_val != 0);


	// Serial.println();
	// Serial.print("Fails: ");
	// Serial.println(fails);

	// fails = 0;

	Serial.println("ASM3:");
	do {
		Serial.println();
		Serial.print("reset_val: ");
		Serial.print(reset_val, HEX);

		new_sample = 0;
		do {
			reset_samples_to(reset_val);

			//Serial.print(".");

			if(sampler.current_sample != 127) {
				char err_str[] = "current_sample before";
				fail_uint8(err_str, sampler.current_sample, 127);
			}
			if(sampler.samples[sampler.current_sample] != reset_val) {
				char err_str[] = "samples[current_sample] before";
				fail_uint8(err_str, sampler.samples[sampler.current_sample], reset_val);
			}
			if(sampler.sample_sum != reset_val * SAMP_BUFF_LEN) {
				char err_str[] = "sample_sum before";
				fail_uint16(err_str, sampler.sample_sum, reset_val * SAMP_BUFF_LEN);
			}

		 	asm3_sampler(new_sample);

			if(sampler.current_sample != 0) {
				char err_str[] = "current_sample after";
				fail_uint8(err_str, sampler.current_sample, 0);
			}
			if(sampler.samples[sampler.current_sample] != new_sample) {
				char err_str[] = "samples[current_sample] after";
				fail_uint8(err_str, sampler.samples[sampler.current_sample], new_sample);
			}
			if(sampler.sample_sum != (reset_val * (SAMP_BUFF_LEN-1))+new_sample) {
				char err_str[] = "sample_sum after";
				fail_uint16(err_str, sampler.sample_sum, (reset_val * (SAMP_BUFF_LEN-1))+new_sample);
			}
			

			new_sample++;
			if(fails) break;
		} while(new_sample != 0);
		reset_val++;
		if(fails) break;
	} while(reset_val != 0);


	Serial.println();
	Serial.print("Fails: ");
	Serial.println(fails);

	while(true) {};
}
