#ifndef RACE_SIM_HEADER
#define RACE_SIM_HEADER

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

void RaceSimV3_WHPSC_complete(float initial_speed_m_s, const bool RECORD_TO_FILE); // Runs the entire simulation
float compareToSimulation (float current_speed_m_s, float current_position_m, float current_power_w); // Called to estimate performance to simulation

#endif