#ifndef INC_TITAN_DATA_H_
#define INC_TITAN_DATA_H_

struct GPSState{
    float altitude_m;
    float latitude_deg;
    float longitude_deg;
    float speed_m_s;
};

struct AtmoConditions {
	float temperature_c;
	float humidity_rel;
	float static_pressure_pa;
	uint16_t co2_ppm;
};


struct CompleteData {
	struct AtmoConditions atmo;
    struct GPSState gps;
};

#endif
