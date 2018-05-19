// Declare variables
char resultstr[128];        // A string to store the JSON we're sending to the server
int frontdoorState = 2;     // This will be 0 if the door is closed and 1 if open
int frontdoor = 2;          // Used to store the readings from the frontdoor sensor
int elapsedTime = 0;        // Keeps track of how much time has passed in seconds
int numRequests = 0;        // Keeps track of requests per hour
int frontdoorChanged = 0;   // 0 not changed, 1 changed
const int ledPin = D0;      // LED plugged in to D0
const int speakerPin = D1;  // Speaker plugged in to D1

void setup() {
    //Create Particle variables
    Particle.variable("result", resultstr, STRING);
    Particle.variable("frontdoor", &frontdoor, INT);
    Particle.variable("numRequests", &numRequests, INT);
    Particle.variable("elapsedTime", &elapsedTime, INT);

    // Initialize D0 pin as output
    pinMode(ledPin, OUTPUT); // LED
    pinMode(speakerPin, OUTPUT); // SPEAKER

    // Initialize D3 pin as input with a pullup resistor
    pinMode(D3, INPUT_PULLUP); // FRONT DOOR

    // Publish an event to the dashboard, used for debugging
    Particle.publish("status","Security setup");
}

// Loop is called continuously while the program is running.
void loop() {
    // FRONT DOOR READING
    delay(300); // Pause for 0.4 seconds
    frontdoor = digitalRead( D3 ); // Get a reading
    if (frontdoor == LOW) { // Door is closed
        if(frontdoorState != 0) {
            frontdoorChanged = 1;
        }
        frontdoorState = 0; //closed
    } else {
        if(frontdoorState != 1) {
            frontdoorChanged = 1;
        }
        frontdoorState = 1; //open
    }


    // UPDATE THE LED
    if(frontdoorState == 1) {
        // A door is open
        digitalWrite(D0, HIGH);   // Turn ON the LED pin
        delay(100);
    } else {
        // Both doors are closed
        digitalWrite(D0, LOW);    // Turn OFF the LED pin
        delay(100);
    }

    // CHECK FOR CHANGES
    if(frontdoorChanged == 1) {
        if(frontdoorState == 1) {
            playTone(); // Play a tone when a door is opened
        }

        // Limit to 40 requests per hour. If more than that, something likely went wrong.
        // This prevents us from spamming the server.
        if(numRequests < 40) {
            numRequests += 1;
            // Inject the variables into a string
            sprintf(resultstr, "{\"status\":\"%i\", \"location\":\"frontdoor\", \"changed\":\"%i\"}", frontdoorState, frontdoorChanged);
            // Publish the string to a webhook named "door"
            Particle.publish("door", resultstr);
        } else if (numRequests == 40) {
            // Publish an event to the dashboard, used for debugging
            Particle.publish("warning", "Exceeded maximum requests");
        }
        frontdoorChanged = 0;
    }

    // REQUEST LIMITING
    elapsedTime += 1; // Used to keep track of how many seconds have passed
    // After an hour passes, reset the request limit
    if(elapsedTime > 3600) {
        elapsedTime = 0;
        numRequests = 0;
    }
}

// The large piezo alarm opperates best at 3000 +/- 500 hz
void playTone() {
    int duration = 60;
    tone(speakerPin, 3322, 50);   // G# is 3322 hz
    delay(200);
    tone(speakerPin, 3322, 100);  // G# is 3322 hz
}
