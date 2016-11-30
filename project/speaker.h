#ifndef speaker_included
#define speaker_included

extern unsigned int note1;
extern unsigned int note2;
extern unsigned int note3;

void speaker_init();
void speaker_set_frequency();
void speaker_set_period(short cycles);

#endif // included
