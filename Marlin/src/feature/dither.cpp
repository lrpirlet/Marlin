#include "dither.h"
#include "../core/macros.h"
#include "../core/serial.h"
#include "../feature/babystep.h"

uint16_t Dithering::Amplitude = 32; // how many steps for dither
float Dithering::MinLayerInterval = 0.1;
float Dithering::PrevZ = 0.0;

uint8_t Dithering::Ditherssteps[4] = {32, 16, 8, 4};

void Dithering::Handle(float CurrZ) {
  if (Dithering::Amplitude == 0 || (fabs(CurrZ - Dithering::PrevZ) < Dithering::MinLayerInterval)) {
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM("Delta Z to small, skipping dithering");
    Dithering::PrevZ = CurrZ;
    return;

  } else {
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM("Start Dithering...");
    while (!Babystep::steps[BS_AXIS_IND(Z_AXIS)] == 0); // if there are some existing babysteps to do just wait it out

    Dithering::PrevZ = CurrZ;
    uint32_t DitherStartTime = millis();
    uint32_t DitherDurationTime = millis();

    for (int i = 0; i < 4; i++) {
      DitherDurationTime = millis();
      if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPAIR("-> Dither #", i + 1, "/4");

      Babystep::add_steps(Z_AXIS, Dithering::Ditherssteps[i]);
      if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPAIR("--> DitherPendingBabysteps : ", Babystep::steps[BS_AXIS_IND(Z_AXIS)]);
      while (!Babystep::steps[BS_AXIS_IND(Z_AXIS)] == 0) { // wait
        if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPAIR("--> DitherPendingBabysteps : ", Babystep::steps[BS_AXIS_IND(Z_AXIS)]);
      }

      Babystep::add_steps(Z_AXIS, 2 * -Dithering::Ditherssteps[i]);
      while (!Babystep::steps[BS_AXIS_IND(Z_AXIS)] == 0); // wait

      Babystep::add_steps(Z_AXIS, Dithering::Ditherssteps[i]);
      while (!Babystep::steps[BS_AXIS_IND(Z_AXIS)] == 0); // wait

      if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPAIR("-> Dithering Sub Duration : ", millis() - DitherDurationTime);
    }
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPAIR("Dithering done, Total Duration : ", millis() - DitherStartTime);
  }
}

void Dithering::CalculateParameters() {
  Dithering::Ditherssteps[0] = Dithering::Amplitude;
  Dithering::Ditherssteps[1] = Dithering::Amplitude / 2;
  Dithering::Ditherssteps[2] = Dithering::Amplitude / 4;
  Dithering::Ditherssteps[3] = Dithering::Amplitude / 8;
}
