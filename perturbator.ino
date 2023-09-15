// Define constants
#define SAMPLE_RATE 10000 // Sample rate in Hz
#define SAMPLE_LENGTH 10000 // Sample length in samples
#define WAVELET_LENGTH 100 // Wavelet length in samples
#define ANALOG_PIN 34 // Analog input pin
#define DIGITAL_PIN 2 // Digital output pin
#define DAC_PIN 25 // DAC output pin

// Declare global variables
float sample[SAMPLE_LENGTH]; // Array to store the audio sample
float wavelet[WAVELET_LENGTH]; // Array to store the wavelet
float error; // Variable to store the error between the sample and the wavelet
float delta[WAVELET_LENGTH]; // Array to store the random values for perturbing and restoring the wavelet

// Setup function
void setup() {
  Serial.begin(115200); // Start serial communication
  pinMode(DIGITAL_PIN, OUTPUT); // Set digital pin as output
  dacWrite(DAC_PIN, 0); // Set DAC output to zero
}

// Loop function
void loop() {
  // Read the audio sample from the analog pin
  Serial.println("Reading audio sample...");
  for (int i = 0; i < SAMPLE_LENGTH; i++) {
    sample[i] = analogRead(ANALOG_PIN) / 4095.0; // Read and normalize the analog value
    delayMicroseconds(1000000 / SAMPLE_RATE); // Wait for the next sample
  }
  
  // Find the best wavelet for the audio sample using an iterative fitting algorithm
  Serial.println("Finding wavelet...");
  initializeWavelet(); // Initialize the wavelet with random values
  float bestError = calculateError(); // Calculate the initial error
  for (int i = 0; i < 1000; i++) { // Repeat for a fixed number of iterations
    perturbWavelet(); // Perturb the wavelet slightly by generating new random values for delta array and adding them to wavelet array 
    error = calculateError(); // Calculate the new error
    if (error < bestError) { // If the new error is lower than the best error
      bestError = error; // Update the best error
    } else { // If the new error is higher than or equal to the best error
      restoreWavelet(); // Restore the previous wavelet by subtracting the same random values from delta array from wavelet array 
    }
  }
  
  // Analyze the audio input on the analog pin and trigger the digital pin and DAC output when a wavelet is found
  Serial.println("Analyzing audio input...");
  while (true) { // Repeat indefinitely
    float correlation = calculateCorrelation(); // Calculate the correlation between the current input and the wavelet
    if (correlation > 0.9) { // If the correlation is high enough (threshold can be adjusted)
      digitalWrite(DIGITAL_PIN, HIGH); // Set digital pin to high
      dacWrite(DAC_PIN, map(correlation, 0, 1, 0, 255)); // Set DAC output to a value proportional to the correlation (frequency shift)
      Serial.println("Wavelet found!"); // Print a message
    } else { // If the correlation is low enough
      digitalWrite(DIGITAL_PIN, LOW); // Set digital pin to low
      dacWrite(DAC_PIN, 0); // Set DAC output to zero
    }
    delayMicroseconds(1000000 / SAMPLE_RATE); // Wait for the next input sample
  }
}

// Function to initialize the wavelet with random values and generate initial random values for delta array 
void initializeWavelet() {
  for (int i = 0; i < WAVELET_LENGTH; i++) {
    wavelet[i] = random(-100, 100) / 100.0; // Generate a random value between -1 and 1 for wavelet array 
    delta[i] = random(-10, 10) / 1000.0; // Generate a random value between -0.01 and 0.01 for delta array 
  }
}

// Function to calculate the error between the sample and the wavelet (mean squared error)
float calculateError() {
  float sum = 0; // Variable to store the sum of squared differences
  for (int i = 0; i < SAMPLE_LENGTH; i++) {
    sum += sq(sample[i] - wavelet[i % WAVELET_LENGTH]); // Add the squared difference between each sample value and wavelet value (with wrap-around)
  }
  return sum / SAMPLE_LENGTH; // Return the average of squared differences
}

// Function to perturb the wavelet slightly by generating new random values for delta array and adding them to wavelet array 
void perturbWavelet() {
  for (int i = 0; i < WAVELET_LENGTH; i++) {
    delta[i] = random(-10, 10) / 1000.0; // Generate a new random value for delta array 
    wavelet[i] += delta[i]; // Add the random value from delta array to wavelet array 
  }
}

// Function to restore the previous wavelet by subtracting the same random values from delta array from wavelet array 
void restoreWavelet() {
  for (int i = 0; i < WAVELET_LENGTH; i++) {
    wavelet[i] -= delta[i]; // Subtract the same random value from delta array from wavelet array 
  }
}

// Function to calculate the correlation between the current input and the wavelet (normalized dot product)
float calculateCorrelation() {
  float dot = 0; // Variable to store the dot product
  float norm1 = 0; // Variable to store the norm of the input
  float norm2 = 0; // Variable to store the norm of the wavelet
  for (int i = 0; i < WAVELET_LENGTH; i++) {
    float x = analogRead(ANALOG_PIN) / 4095.0; // Read and normalize the current input value
    float y = wavelet[i]; // Get the current wavelet value
    dot += x * y; // Add the product of the input and wavelet values
    norm1 += sq(x); // Add the square of the input value
    norm2 += sq(y); // Add the square of the wavelet value
  }
  return dot / sqrt(norm1 * norm2); // Return the normalized dot product
}
