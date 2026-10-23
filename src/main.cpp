#include <Arduino.h>
#include <HX711.h>

#define SCALE_CLOCK_PIN D1
#define SCALE_DATA_PIN D2
#define SCALE_GRAMS_CALIBRATION 2200

#define PUMP_PIN D5

HX711 scale;

void initPump() {
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, 0);
}

void setPumpPower(uint16 power) {
    analogWrite(PUMP_PIN, power);
}

void initScale() {
    scale.begin(SCALE_DATA_PIN, SCALE_CLOCK_PIN);
    scale.set_scale(SCALE_GRAMS_CALIBRATION);
    scale.tare();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Donna The Wine Dispenser is starting");

    initPump();
    initScale();
}

uint stableCounter = 0;
bool isStable = false;
float previousMass = 0;
const float STABLE_MASS_TRESHHOLD = 0.5;
uint STABLE_COUNTER_TRESHHOLD = 3;
uint EMPTY_GLASS_TRESHHOLD = 1.5;
uint FULL_GLASS = 80;

enum Status {
    AWAITING_GLASS,
    POURING,
    DONE
};

Status status = AWAITING_GLASS;

void loop() {
    if (scale.is_ready()) {
        float mass = scale.get_units(5);

        if (abs(previousMass - mass) < STABLE_MASS_TRESHHOLD) {
            stableCounter++;
        } else {
            stableCounter = 0;
        }

        isStable = stableCounter > STABLE_COUNTER_TRESHHOLD;

        Serial.printf("mass: %.2f isStable: %d\n", mass, isStable);

        if (status == AWAITING_GLASS && isStable && mass > EMPTY_GLASS_TRESHHOLD) {
            status = POURING;
            scale.tare();
            setPumpPower(140);
        }

        if (status == POURING && mass > FULL_GLASS) {
            status = DONE;
            setPumpPower(0);
        }

        if (status == DONE && mass < EMPTY_GLASS_TRESHHOLD) {
            status = AWAITING_GLASS;
			delay(500);
            scale.tare();
        }

        previousMass = mass;
    }

    delay(100);
}
