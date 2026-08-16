#include "racesim.h"

/*INPUT:
Rf,Rr - radii of the front and rear wheels (m)
If,Ir - Inertia of the front and rear wheels (kg*m^2)
M - mass of the bike/rider system (kg)
PFcn - power as a function of x (position on the course) (W)
rho - air density (kg/m^3)
RefLen - length of the vehicle used for Re calcs (m)
CdA_Fcn - drag area as  a function of Re (m^2)
Crr - rolling resistance coeff.
eta - drivetrain efficiency
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
static const float Rf = 0.255;
static const float Rr = 0.255;
static const float Mf = 2.5;
static const float Mr = 2.5;
static const float If = 0.0197492333611; // (Mf-0.200)*(Rf-25e-3)^2;
static const float Ir = 0.0197492333611;
static const float Mframe = 40 - Mf - Mr; // everything except the wheels
static const float Mrider = 145;
static const float M = Mf + Mr + Mframe + Mrider;
static const float MI = M + If / (Rf * Rf) + Ir / (Rr * Rr);
static const float RefLen = 3.4;

static const float g = 9.81;	// Gravitational acceleration (m/s^2)
static const float W = g*M;  	// Weight (N)

// Bike Drag Parameters
static const float rho = 0.95;
static const float mu = 1.983e-5;   // dynamic viscosity of air
static const float nu = mu / rho;   // Kinematic viscosity
static const float xpRef = 130 / 3.6; // reference velocity for CdA_Ref
static const float CdA_Valkyrie = 0.014 * 1.7; // [m^2] 
static const float CdA_Ref = 1.0 * CdA_Valkyrie;
static const float ReRef = xpRef * rho * RefLen / mu;
static const float L2nu = RefLen / nu;  // Used in Reynolds

// Rolling resistance parameters/function
static const float Crr1 = 0.0018; // 0.0023 * 0.4 + 0.0039 * 0.6; 
static const float Crr2 = 0.000005; // 0.000064 * 0.6; // 0.000064 for Pro One, 0 for GP Custom
static const float eta = 0.97; // Drive train efficiency

// Simulation parameters
static const float distanceEnd = 8000; // Distance to end

// Power Setup
static const float p_runup = 290; // W
static const float p_sprint = 490; // W
static const float sprint_start_mark = 1.25; // mile marker from finish
static const float sprint_start = (5*1600)-(sprint_start_mark*1600); // m

static float Re(float xp) {
	return (xp * L2nu);
}

static float Cd_flatplate(float re) {
    // turb flat plate. Anderson page 840
    float cd = 0.074 * pow(re, -0.2);

    // Return minimum between cd and a threshold
    const float cdThreshold = 0.01;
    if (cd < cdThreshold) return cd;
    else return cdThreshold;
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
    if (dist < sprint_start) return p_runup;
    else return p_sprint;
}

static float slopePolynomial(float distance) {
    distance = distance / 1000.0; // Reduce differences in orders to keep things accurate
    float slope = -4.87E-06*pow(distance,4) + 0.0000643*pow(distance,3) - 0.0000185*pow(distance,2) - 0.0015*distance - 0.00427;
    return slope;
}

void RaceSimV3_WHPSC_complete(float initialSpeed, bool recordSimulation) {

    // Ensure proper input and other constants
    const float stepDuration = 0.005;
    
    // Run time-marching algorithm
    // initialize X
    float currX[] = {0, initialSpeed}; // displacement velocity
    int step = 0; //keep track of steps
    
    FILE * logFile;
	if (recordSimulation == true) {
        logFile = fopen("./testlog.csv", "w+");
        fprintf(logFile, "Time,Position (m),Speed (m/s),Charge,Ppassive (w),Paero,Prolling,Pslope\n");
        fprintf(logFile, "%.3f,%f,%f,0,0,0,0,0\n", step*stepDuration, currX[0], currX[1]);
        fclose(logFile);
    }
    
    while (currX[0] < distanceEnd) {
        float x = currX[0]; //position
        float xp = currX[1]; //speed
        
        // figure out passive power loss
        float Paero = -0.5*rho*pow(xp,3)*CdA_Fcn(Re(xp));
        float Prolling = -Crr(xp)*M*sqrt(g*g)*xp;
        float Pslope = -slopePolynomial(x)*xp*W;
        
        float Ppassive = Pslope + Prolling + Paero;

        // Compute new position
        currX[0] = currX[0] + xp * stepDuration;

        // Compute new speed
        float acceleration = ((Ppassive + PFcn(x)*eta)/xp)/MI;
        if (acceleration > 4.5) acceleration = 4.5;
        currX[1] = currX[1] + acceleration * stepDuration; // min is just to stop the singularity at xp = zero
        step++;

        if (recordSimulation == true) {
            // Open and append data
            logFile = fopen("./testlog.csv", "a"); // Append
            fprintf(logFile, "%.3f,%f,%f,0,%f,%f,%f,%f\n", step*stepDuration, currX[0], currX[1], Ppassive, Paero, Prolling, Pslope);
            fclose(logFile);
        }
    }
    
    // Output stuff (I don't think any is particularly useful to us)
    const float tEnd = step * stepDuration;
    const float xpEnd = currX[1];
    
    printf("\nRaceSim Results:\n\tTime: %.2f\n\tEnd Speed (m/s | km/h | mph): %.2f | %.2f | %.2f", tEnd, xpEnd, xpEnd * 3.6, xpEnd * 2.2369);
    
    return;
}

float compareToSimulation (float speed, float position, float power) {
    // Use 'static' to carry data between calls without resorting to a global scope
    static float prevSpeed = 0;
    static float prevPosition = 0;
    static float prevPower = 0;
    struct timespec previousTime, currentTime;  // Realtime marks

    float performanceFactor = 100.0; // Default to nominal

    // Check if it is first call
    if ((prevSpeed == 0) && (prevPosition == 0) && (prevPower == 0)) {
        // Record data and return with nominal (100%)
        prevSpeed = speed;
        prevPosition = position;
        prevPower = power;
        clock_gettime(CLOCK_MONOTONIC, &previousTime); // Record time

        return 100.0;
    }

    // Get time difference from last call in seconds
    clock_gettime(CLOCK_MONOTONIC, &currentTime); // Get end time
    float deltaTime = ((currentTime.tv_sec - previousTime.tv_sec)) + ((currentTime.tv_nsec - previousTime.tv_nsec) / 1000000000);
    
    // Find expected speed for present extrapolated from previous call
    // Copied from simulation 
    
    // figure out passive power loss
    float Paero = -0.5*rho*pow(prevSpeed,3)*CdA_Fcn(Re(prevSpeed));
    float Prolling = -Crr(prevSpeed)*M*g*prevSpeed;
    float Pslope = -slopePolynomial(prevPosition)*prevSpeed*W;
    float Ppassive = Pslope + Prolling + Paero;

    // Compute estimated speed
    float acceleration = ((Ppassive + prevPower*eta)/prevSpeed)/MI;
    float estimatedSpeed = prevSpeed + acceleration * deltaTime;
    
    performanceFactor = 100.0 * (speed / estimatedSpeed);

    // Record parameters for next iteration
    prevSpeed = speed;
    prevPosition = position;
    prevPower = power;
    clock_gettime(CLOCK_MONOTONIC, &previousTime); // Record time

    return performanceFactor;
}
 
/*   
int main() {
    // Test file with 
    RaceSimV3_WHPSC_complete(2.7, true);
    return 0;
}
*/
    
