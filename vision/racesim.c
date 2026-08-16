#include "racesim.h"

/*INPUT:
radius_front_m,radius_rear_m - radii of the front and rear wheels (m)
If,Ir - Inertia of the front and rear wheels (kg*m^2)
m_total_kg - mass of the bike/rider system (kg)
PFcn - power as a function of x (position on the course) (W)
rho - air density (kg/m^3)
vehicle_reference_length_m - length of the vehicle used for Re calcs (m)
CdA_Fcn - drag area as  a function of Re (m^2)
Crr - rolling resistance coeff.
eta_drivetrain - drivetrain efficiency
MaxLeanAng - maximum lean angle in turns (deg.)
TimeDurationGuess - estimate of longest time it would take to complete
      the course (be conservative) (sec.)
BrakeAccel - acceleration to use when braking (m/s^2)
xpTol - tolerance to use for speed for the simulation. Typically global
      tolerance = xpTol/1000 so keep this in mind (m/s)
xpStart - start speed (or estimated start speed if periodic) (m/s)


Ported and stripped down for C (from MATLAB) by Savo Bajic 2022/07/22
Calvin Moes 2016/03/29
Based on code by Trefor Evans
*/

static float CdA_Fcn(float Re);
static float Cd_flatplate(float re);
static float Crr(float xp);
static float PFcn(float dist);
static float Re(float xp);
static float slopePolynomial(float distance);

// Bike Mass and geometry parameters
static const float radius_front_m = 0.255;
static const float radius_rear_m = 0.255;
static const float m_front_wheel_kg = 2.5;
static const float m_rear_wheel_kg = 2.5;
static const float If = (m_front_wheel_kg-0.200)*(radius_front_m-25e-3)*(radius_front_m-25e-3);
static const float Ir = (m_rear_wheel_kg-0.200)*(radius_rear_m-25e-3)*(radius_rear_m-25e-3);
static const float m_frame_kg = 40.0 - m_front_wheel_kg - m_rear_wheel_kg; // TITAN except the wheels
static const float m_riders_kg = 145.0;
static const float m_total_kg = m_front_wheel_kg + m_rear_wheel_kg + m_frame_kg + m_riders_kg;
static const float MI = m_total_kg + If / (radius_front_m * radius_front_m) + Ir / (radius_rear_m * radius_rear_m);
static const float vehicle_reference_length_m = 3.4;

static const float g_m_s2 = 9.81;	// Gravitational acceleration (m/s^2)
static const float weight_n = g_m_s2*m_total_kg;  	// Weight (N)

// Bike Drag Parameters
static const float rho = 0.95;
static const float mu = 1.983e-5;   // dynamic viscosity of air
static const float nu = mu / rho;   // Kinematic viscosity
static const float xpRef = 130 / 3.6; // reference velocity for CdA_Ref
static const float CdA_Valkyrie = 0.014 * 1.7; // [m^2] 
static const float CdA_Ref = 1.0 * CdA_Valkyrie;
static const float ReRef = xpRef * rho * vehicle_reference_length_m / mu;
static const float L2nu = vehicle_reference_length_m / nu;  // Used in Reynolds

// Rolling resistance parameters/function
static const float Crr1 = 0.0018; // 0.0023 * 0.4 + 0.0039 * 0.6; 
static const float Crr2 = 0.000005; // 0.000064 * 0.6; // 0.000064 for Pro One, 0 for GP Custom
static const float eta_drivetrain = 0.97; // Drive train efficiency

// Simulation parameters
static const float distance_end_m = 8000; // Distance to end

// Power Setup
static const float p_runup_w = 290; // For both riders
static const float p_sprint_w = 490; // For both riders
static const float sprint_start_mile_mark = 1.25; // mile marker from finish
static const float sprint_start_m = (5.0*1600.0)-(sprint_start_mile_mark*1600.0);

static float Re(float xp) {
	return (xp * L2nu);
}

static float Cd_flatplate(float re) {
    // turb flat plate. Anderson page 840
    float cd = 0.074 * pow(re, -0.2);

    // Return minimum between cd and a threshold
    const float Cd_MINIMUM_THRESHOLD = 0.01;
    if (cd < Cd_MINIMUM_THRESHOLD) return cd;
    else return Cd_MINIMUM_THRESHOLD;
}

static float CdA_Fcn(float Re) {
    return (CdA_Ref * (Cd_flatplate(Re) / Cd_flatplate(ReRef))); 
    //this allows the Cd to vary approx accurately with Re but be correct at a given speed
}

static float Crr(float xp) {
    return (Crr1 + xp * Crr2);
}

static float PFcn(float dist) {
    // Return sprint power if in sprint region
    if (dist < sprint_start_m) return p_runup_w;
    else return p_sprint_w;
}

static float slopePolynomial(float distance) {
    distance = distance / 1000.0; // Reduce differences in orders to keep things accurate
    float slope = -4.87E-06*pow(distance,4) + 0.0000643*pow(distance,3) - 0.0000185*pow(distance,2) - 0.0015*distance - 0.00427;
    return slope;
}

void RaceSimV3_WHPSC_complete(float initial_speed_m_s, const bool RECORD_TO_FILE) {
    // Run time-marching algorithm
    const float step_duration_s = 0.005;
    float current_speed_m_s = initial_speed_m_s;
    float current_distance_m = 0;
    int step = 0;
    
    FILE * log_file;
	if (RECORD_TO_FILE) {
        log_file = fopen("./testlog.csv", "w+");
        fprintf(log_file, "Time,Position (m),Speed (m/s),Passive power(w),Aero Power,Rolling Power,Slope Power\n");
        fprintf(log_file, "%.3f,%f,%f,0,0,0,0\n", step*step_duration_s, current_distance_m, current_speed_m_s);
        fclose(log_file);
    }
    
    while (current_distance_m < distance_end_m) {        
        // figure out passive power loss
        float p_aero_w = -0.5*rho*pow(current_speed_m_s,3)*CdA_Fcn(Re(current_speed_m_s));
        float p_rolling_w = -Crr(current_speed_m_s)*m_total_kg*g_m_s2*current_speed_m_s;
        float p_slope_w = -slopePolynomial(current_distance_m)*current_speed_m_s*weight_n;

        float p_passive_w = p_slope_w + p_rolling_w + p_aero_w;

        current_distance_m = current_distance_m + current_speed_m_s * step_duration_s;

        // Compute new speed
        float acceleration_m_s2 = ((p_passive_w + PFcn(current_distance_m)*eta_drivetrain)/current_speed_m_s)/MI;
        const float ACCELERATION_LIMIT_M_S2 = 4.5;
        if (acceleration_m_s2 > ACCELERATION_LIMIT_M_S2) acceleration_m_s2 = ACCELERATION_LIMIT_M_S2;
        current_speed_m_s = current_speed_m_s + acceleration_m_s2 * step_duration_s; // min is just to stop the singularity at xp = zero
        step++;

        if (RECORD_TO_FILE) {
            // Open and append data
            log_file = fopen("./testlog.csv", "a"); // Append
            fprintf(log_file, "%.3f,%f,%f,%f,%f,%f,%f\n", step*step_duration_s, current_distance_m, current_speed_m_s, p_passive_w, p_aero_w, p_rolling_w, p_slope_w);
            fclose(log_file);
        }
    }
    
    const float FINISH_TIME_S = step * step_duration_s;
    printf("\nRaceSim Results:\n\tTime: %.2f\n\tEnd Speed (m/s | km/h | mph): %.2f | %.2f | %.2f", FINISH_TIME_S, current_speed_m_s, current_speed_m_s * 3.6, current_speed_m_s * 2.2369);
    return;
}

float compareToSimulation (float current_speed_m_s, float current_position_m, float current_power_w) {
    // Use 'static' to carry data between calls without resorting to a global scope
    static float prev_speed_m_s = 0;
    static float prev_position_m = 0;
    static float prev_power_w = 0;
    struct timespec prev_time, current_time;  // Realtime marks

    float performance_factor = 100.0; // Default to nominal

    // Check if it is first call
    if ((prev_speed_m_s == 0) && (prev_position_m == 0) && (prev_power_w == 0)) {
        // Record data and return with nominal (100%)
        prev_speed_m_s = current_speed_m_s;
        prev_position_m = current_position_m;
        prev_power_w = current_power_w;
        clock_gettime(CLOCK_MONOTONIC, &prev_time); // Record time

        return performance_factor;
    }

    // Get time difference from last call in seconds
    clock_gettime(CLOCK_MONOTONIC, &current_time); // Get end time
    float delta_time_s = ((current_time.tv_sec - prev_time.tv_sec)) + ((current_time.tv_nsec - prev_time.tv_nsec) / 1000000000);
    
    // Find expected speed for present extrapolated from previous call
    // Copied from simulation 
    
    // figure out passive power loss
    float p_aero_w = -0.5*rho*pow(prev_speed_m_s,3)*CdA_Fcn(Re(prev_speed_m_s));
    float p_rolling_w = -Crr(prev_speed_m_s)*m_total_kg*g_m_s2*prev_speed_m_s;
    float p_slope_w = -slopePolynomial(prev_position_m)*prev_speed_m_s*weight_n;
    float p_passive_w = p_slope_w + p_rolling_w + p_aero_w;

    // Compute estimated speed
    float acceleration_m_s2 = ((p_passive_w + prev_power_w*eta_drivetrain)/prev_speed_m_s)/MI;
    float estimated_speed = prev_speed_m_s + acceleration_m_s2 * delta_time_s;
    
    performance_factor = 100.0 * (current_speed_m_s / estimated_speed);

    // Record parameters for next iteration
    prev_speed_m_s = current_speed_m_s;
    prev_position_m = current_position_m;
    prev_power_w = current_power_w;
    clock_gettime(CLOCK_MONOTONIC, &prev_time); // Record time

    return performance_factor;
}
 
/*   
int main() {
    // Test file with 
    RaceSimV3_WHPSC_complete(2.7, true);
    return 0;
}
*/
    
