#include "main.h"
#define POWER_AVERAGE_FRAMES 50 // Needs to be #define for array sizes

int overlay_frames_to_render = -1; // Number of frames to render for testing (-1 for infinite)
const float WHEEL_CIRCUMFERENCE_M = 2.104;

// Serial configuration
bool using_serial_data = false; // Use serial or not
int serial_line_in_use = -1;

bool collecting_ant_data = false; 	   // Use ANT data from USB?
bool logging_enabled = false;   // Logging configuration
bool cameara_enabled = false;    // Camera
bool system_is_front_rider = true;          // Is front or not

int main(int argc, char *argv[]) {
   printf(ANSI_COLOUR_BLUE "\n==================\n! TITAN 2022 HUD !\n==================" ANSI_COLOUR_RESET);
   
   // Start by reading in arguements
   printf("\n\nExpected arguements when called in order: 'setup string' 'number of frames'\n");
   printf("\tSpecify role with 'f' or 'r' (assumed to be front if not specified)\
         \n\tEnable camera with 'c'\n\tEnable logging with 'l'\
         \n\tEnable ANT collection with 'a'\n\tEnable serial coms with 's'\
         \n\n\tFrames as -1 for infinty, otherwise the number wanted.\
         \n\tE.g. 'bike.bin fc 10' sets system for front operation with camera for 10 frames\n");
   
   // Read in configuration using a run through switch based on arguement count
   // Note: arguement 0 is the call to run the program itself e.g. "./bike.bin"
   int characterToRead = 0;
   switch (argc) {
      case 3:
         // Get frame count
         overlay_frames_to_render = atoi(argv[2]);
      case 2:
         // Go through setup string
         do {
            switch (argv[1][characterToRead]) {
               case 'l':
                  logging_enabled = true;
                  break;
               case 's':
                  using_serial_data = true;
                  break;
               case 'a':
                  collecting_ant_data = true;
                  break;
               case 'c':
                  cameara_enabled = true;
                  break;
               case 'r':
                  system_is_front_rider = false;
                  break;
               case 'f':
                  system_is_front_rider = true;
                  break;
            }
         } while (argv[1][characterToRead++] != '\0'); 
         break;
   }
   

   
   // Reguritate configuration
   printf("\nParsed configuration:\n" ANSI_COLOUR_YELLOW);
   if (system_is_front_rider == true) printf("\tRole: FRONT\n");
   else printf("\tRole: REAR\n");
   printf("\tCamera running: %d\n", cameara_enabled);
   printf("\tCollecting ANT data: %d\n", collecting_ant_data);
   printf("\tCommunicate over serial: %d\n", using_serial_data);
   printf("\tLogging: %d\n", logging_enabled);
   if (overlay_frames_to_render > 0) printf("\tNumber of frames: %d\n\n" ANSI_COLOUR_RESET, overlay_frames_to_render);
   else printf("\tNo limit to number of frames.\n\n" ANSI_COLOUR_RESET);
   
   // Local variables
   int cadence_front_r_m = 0, cadence_rear_r_m = 0; 
   int power_front_w = 0, power_rear_w = 0;
   int heart_rate_front_b_m = 0, heart_rate_rear_b_m = 0;
   int ANTData[] = {0,0,0,0,0,0};
   int battery_soc_rear_percent = 0, battery_soc_front_percent = 0;
   int wheel_rotations = 0;
   float speed_wheel_km_h = 0.0;
   float dist_wheel_km = 0.0, dist_gps_km = 0.0;
   float temperature_c = 0.0;
   float humidity_per = 0.0;
   float performance_percentage = 0.0;
   float brake_temp_front_c = 200.0, brake_temp_rear_c = 200.0;
   int co2_ppm = 0;
   float speed_gps_km_h = 0.0;
   
   // Open serial line
   if (using_serial_data) {
      serial_line_in_use = openLine("/dev/serial0");
      
      if (serial_line_in_use != -1) printf(ANSI_COLOUR_GREEN "Successfully opened serial line!\n" ANSI_COLOUR_RESET);
      else {
         printf(ANSI_COLOUR_RED "FAILED TO OPEN SERIAL LINE!\nReverting to non-serial behavior.\n" ANSI_COLOUR_RESET);
         using_serial_data = false;
      }
   }
   else printf(ANSI_COLOUR_RED "NOT USING SERIAL DATA.\n" ANSI_COLOUR_RESET);
   
   // Inform if ANT data is expected, and set up stdin accordingly
   if (collecting_ant_data) {
      printf("Expecting ANT data to be piped in.\n");
      fcntl(0, F_SETFL, O_NONBLOCK); // Set stdin (0) to be non-blocking
   }
   else if (using_serial_data) printf(ANSI_COLOUR_GREEN "Not collecting ANT data locally, requesting ANT data over serial.\n" ANSI_COLOUR_RESET);
   else printf(ANSI_COLOUR_RED "NOT USING ANY ANT DATA. DISPLAYING RANDOM ANT DATA!\n" ANSI_COLOUR_RESET);
   
   // Start logging
   if (logging_enabled) startLogging();
   else printf(ANSI_COLOUR_YELLOW "NOT LOGGING DATA!\n" ANSI_COLOUR_RESET);
   
   // Print camera status
   if (cameara_enabled) printf(ANSI_COLOUR_RESET "Camera will be launched shortly!\n" ANSI_COLOUR_RESET);
   else printf(ANSI_COLOUR_RED "Camera not configured to start.\n" ANSI_COLOUR_RESET);
      
   sleep(3); // Pause to show config before potentially running camera
   
   printf("\n=========================================================\n");
   printf("RUNNING MAIN LOOP");
   printf("\n=========================================================\n\n");
   
   
   // Timing the entire process
   struct timespec tSystemStart,tSystemEnd;
   clock_gettime(CLOCK_MONOTONIC, &tSystemStart); // Get start time
   clock_t titanProcessClock = clock(); // Get start processor time
   
   startOverlay(cameara_enabled);
   
   // Main HUD loop
   int overlay_frames_left_to_render = overlay_frames_to_render;
   do {
      
      // ANT data
      printf("Grabbing ANT data\n");
      if (collecting_ant_data == true) getANTDataPipedIn(ANTData, serial_line_in_use);      // It is meant to collect data locally
      else if (using_serial_data == false) { 
         // Cant' collect nor request ANT data, use random numbers
         ANTData[0] = 60;
         ANTData[1] = 60;
         ANTData[2] = 120;
         ANTData[3] = 60;
         ANTData[4] = 60;
         ANTData[5] = 120;
      }
      
      // Copy to nicer named variables
      heart_rate_front_b_m = ANTData[0];
      cadence_front_r_m = ANTData[1];
      power_front_w = ANTData[2];
      heart_rate_rear_b_m = ANTData[3];
      cadence_rear_r_m = ANTData[4];
      power_rear_w = ANTData[5];

      
      // Get bike data
      if (using_serial_data) { // Collect bike data from STM32 over serial      
         printf("Grabbing serial data\n");
         
         char bulkBuffer[32];
         startTrial(); 
         requestData(serial_line_in_use, '{', &bulkBuffer[1]); 
         
         struct bulkDataStruct {
            char messageType;
            char messageLength;
            uint16_t distGPS;
            uint32_t speedEncoder;
            uint32_t speedGPS;
            uint16_t rotations;
            uint16_t frontBrakeT;
            uint16_t rearBrakeT;
            uint8_t fBatt;
            uint8_t rBatt;
            uint8_t humid;
            uint8_t temp;
            uint16_t CO2;
            uint8_t fhr;
            uint8_t rhr;
            uint8_t fcad;
            uint8_t rcad;
            uint16_t fpwr;
            uint16_t rpwr;
         } dataLoad;
         
         memcpy(&dataLoad, bulkBuffer, sizeof(dataLoad));
         dataLoad.messageType = '[';
         
         dist_gps_km = dataLoad.distGPS / 1000.0;
         speed_wheel_km_h = dataLoad.speedEncoder / 1000.0;
         speed_gps_km_h = dataLoad.speedGPS / 1000.0;
         wheel_rotations = dataLoad.rotations;
         brake_temp_front_c = dataLoad.frontBrakeT / 100.0;
         brake_temp_rear_c =dataLoad.rearBrakeT / 100.0;
         battery_soc_front_percent = dataLoad.fBatt;
         battery_soc_rear_percent = dataLoad.rBatt;
         humidity_per = dataLoad.humid / 2.0;
         temperature_c = (dataLoad.temp - 50.0) / 2.0;
         co2_ppm = dataLoad.CO2;
         heart_rate_front_b_m = dataLoad.fhr;
         heart_rate_rear_b_m = dataLoad.rhr;
         cadence_front_r_m = dataLoad.fcad;
         cadence_rear_r_m = dataLoad.rcad;
         power_front_w = dataLoad.fpwr;
         power_rear_w = dataLoad.rpwr;

         endTrialIgnore("Bulk transfer", 30);

         dist_wheel_km = wheel_rotations * WHEEL_CIRCUMFERENCE_M / 1000.0;
         
         /*
         printf("Message type: %c, length char %c\n", dataLoad.messageType, dataLoad.messageLength);
         printf("\tSpeeds: %.3f / %.3f\n", speed_wheel_km_h, speed_gps_km_h);
         printf("\tDist: %.3f / %.3f\n", dist_wheel_km, dist_gps_km);
         printf("\tBrake Temps: %.2f / %.2f\n", brake_temp_front_c, brake_temp_rear_c);
         printf("\tBatteries: %d / %d\n", battery_soc_front_percent, battery_soc_rear_percent);
         printf("\tAtmosphere: %.1f degC, %.1f %%RH, %d ppmCO2\n", temperature_c, humidity_per, co2_ppm);
         printf("\tANT front: HR %d | CAD %d | PWR %d\n", heart_rate_front_b_m, cadence_front_r_m, power_front_w);
         printf("\tANT rear: HR %d | CAD %d | PWR %d\n", heart_rate_rear_b_m, cadence_rear_r_m, power_rear_w);
         */
         
      }
      else {
         // Placeholder data for when not collecting anything over serial
         battery_soc_rear_percent = 25;
         battery_soc_front_percent = 25;
         speed_wheel_km_h = 120.6;
         dist_wheel_km = 7;
         temperature_c = 0.0;
         humidity_per = 0.0;
         performance_percentage = 101.2;
         brake_temp_front_c = 200.0;
         brake_temp_rear_c = 200.0;
         co2_ppm = 1550;
         speed_gps_km_h = 120.6;
         dist_gps_km = 6.92;
      }
      
      // Get average power over POWER_AVERAGE_FRAMES frames 
      // 10 frames/second, power sensor polls 1 times/second 
      // This does a continuous (rolling) average instead of a periodic average
      if (true) {
         // printf("Averaging power\n");
         static int front_power_values[POWER_AVERAGE_FRAMES], rear_power_values[POWER_AVERAGE_FRAMES];
         static int current_index = 0;
         
         // Add current power to rolling buffer
         front_power_values[current_index] = power_front_w;
         rear_power_values[current_index] = power_rear_w;
         
         // Determine average value for each buffer and save it
         long frontPowerTotal = 0;
         long rearPowerTotal = 0;
         
         for (int i = 0; i < POWER_AVERAGE_FRAMES; i++) {
            frontPowerTotal = frontPowerTotal + front_power_values[i];
            rearPowerTotal = rearPowerTotal + rear_power_values[i];
         }
         
         power_front_w = frontPowerTotal / POWER_AVERAGE_FRAMES;
         power_rear_w = rearPowerTotal / POWER_AVERAGE_FRAMES;
         
         // Increment and loop current frame index
         current_index++;
         current_index = current_index % POWER_AVERAGE_FRAMES;
      }
      else {
         printf("you done screw up on the power loop.");
      }
      
      
      printf("Performance factor\n");
      // Performance factor
      performance_percentage = compareToSimulation(speed_wheel_km_h / 3.6, dist_wheel_km / 1000.0, (power_front_w + power_rear_w));

      
      // Overlays
      printf("Making overlay\n");
      if (system_is_front_rider) { // Front overlay
         startTrial();
         updateOverlayFront(speed_wheel_km_h, dist_wheel_km, power_front_w, cadence_front_r_m, heart_rate_front_b_m, performance_percentage, brake_temp_front_c, battery_soc_front_percent, speed_gps_km_h);
         endTrialIgnore("front overlay", 100);
      }
      else { // Rear overlay
         startTrial();
         updateOverlayRear(speed_wheel_km_h, dist_wheel_km, power_rear_w, power_front_w, cadence_rear_r_m, heart_rate_rear_b_m, brake_temp_front_c, brake_temp_rear_c, battery_soc_rear_percent, performance_percentage, co2_ppm, speed_gps_km_h);
         endTrialIgnore("rear overlay", 100);
      }
      
      
      // Logging
      if (logging_enabled) {
         printf("Logging\n");
         updateLog(speed_wheel_km_h, dist_wheel_km, power_front_w, power_rear_w, 
                  cadence_front_r_m, cadence_rear_r_m, heart_rate_front_b_m, heart_rate_rear_b_m, 
                  temperature_c, humidity_per, battery_soc_front_percent, battery_soc_rear_percent,
                  brake_temp_front_c, brake_temp_rear_c, co2_ppm, performance_percentage,
                  speed_gps_km_h, dist_gps_km);
      }
      
      // Count down number of frames if there was a limit stated
      if (overlay_frames_to_render > 0) overlay_frames_left_to_render--;
   } while ((overlay_frames_left_to_render != 0) || (overlay_frames_to_render == -1));
   
   // Overal time
   titanProcessClock = clock() - titanProcessClock; // Get run time
   clock_gettime(CLOCK_MONOTONIC, &tSystemEnd); // Get end time
   float secondsElapsed = ((float) titanProcessClock)/CLOCKS_PER_SEC;
   
   float overallDelta = (tSystemEnd.tv_sec - tSystemStart.tv_sec) + ((tSystemEnd.tv_nsec - tSystemStart.tv_nsec) / 1000000000.0);
   
   printf(ANSI_COLOUR_MAGENTA "\nIt took %2.3f processor seconds, %.3f realtime seconds for %d frames.\n" ANSI_COLOUR_RESET, secondsElapsed, overallDelta, overlay_frames_to_render);
   printf(ANSI_COLOUR_MAGENTA "An average of %.3f realtime seconds for each frame (%.2f fps).\n" ANSI_COLOUR_RESET, overallDelta / overlay_frames_to_render, overlay_frames_to_render / overallDelta);
   
   closeOverlay();
   
   return 0;
}