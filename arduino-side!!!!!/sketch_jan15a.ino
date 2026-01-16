#include <Arduino.h>
#include <Mozzi.h>
#include <mozzi_midi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#define MOZZI_CONTROL_RATE 64
struct channel {
  Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> note;
  byte velocity = 0;
  bool isOn = false;
  channel() : note(SIN2048_DATA) {
  };
};
channel channels[16];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  startMozzi(CONTROL_RATE);
  channels[0].note.setFreq(440); //test thingy, to test if the code work as intended
  channels[0].velocity = 127;
}


AudioOutput updateAudio() {
  long i = 0;
  for (byte j = 0; j < 16; j++) {
    if (channels[j].velocity > 0) {
        i += (int)channels[j].note.next() * (channels[j].velocity);;
    }
  }
  return (MonoOutput::from16Bit(i >> 3)); //prevent out range
}
void updateControl() {
}
void loop() {
  // put your main code here, to run repeatedly:
  audioHook();
  while (Serial.available() >= 3) {
    byte commandChannel = Serial.read(); //command byte and channel byte
    byte noteByte = Serial.read();
    byte velocityByte = Serial.read();
    byte commandByte = commandChannel & 0xF0;
    byte channelByte = commandChannel & 0x0F;
    if (commandByte == 0x90) {
      //receive note ON
      channels[channelByte].note.setFreq(mtof(noteByte));
      channels[channelByte].velocity = velocityByte;
    }
    else if (commandByte == 0x80) {
      channels[channelByte].note.setFreq(mtof(noteByte));
      channels[channelByte].velocity = 0;
    }
  }
}
