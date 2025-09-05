#include <Arduino.h>

// Configurações do ADC e LEDs
const int ADC_PIN = 34;        // GPIO34 (potenciômetro)
const int LED_XOR_TRUE = 2;    
const int LED_XOR_FALSE = 4;   

// Configurações da janela
const int FS_HZ = 1000;        // 1 kHz
const int WIN_MS = 200;        // 200 ms
const int N = FS_HZ * WIN_MS / 1000;
static float buf[N];

// Thresholds para RMS e PTP
float TH_RMS = 0.01f;
float TH_PTP = 0.05f;

// Buffer para armazenar 100 linhas de dados
const int MAX_LINES = 100;
String csvBuffer[MAX_LINES];
int lineCount = 0;

// Funções auxiliares
float adc_to_unit(int raw) { return raw / 4095.0f; }

void calc_feats(const float *x, int n, float &rms, float &ptp) {
    float xmin = x[0], xmax = x[0];
    double sumsq = 0.0;
    for (int i = 0; i < n; ++i) {
        float v = x[i];
        sumsq += (double)v * v;
        if (v < xmin) xmin = v;
        if (v > xmax) xmax = v;
    }
    rms = sqrt(sumsq / n);
    ptp = xmax - xmin;
}

void calc_mean_std(const float *x, int n, float &mean, float &std) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += x[i];
    mean = sum / n;

    double sumsq = 0.0;
    for (int i = 0; i < n; ++i) sumsq += (x[i] - mean) * (x[i] - mean);
    std = sqrt(sumsq / n);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_XOR_TRUE, OUTPUT);
    pinMode(LED_XOR_FALSE, OUTPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(ADC_PIN, ADC_11db);

    // Cabeçalho CSV
    Serial.println("mean,std,rms,ptp,label");
}

void loop() {
    const uint32_t Ts = 1000000UL / FS_HZ;
    float rms, ptp, mean, std;

    // 1) Captura janela de N amostras
    for (int i = 0; i < N; ++i) {
        buf[i] = adc_to_unit(analogRead(ADC_PIN));
        delayMicroseconds(Ts);
    }

    // 2) Calcula features
    calc_feats(buf, N, rms, ptp);
    calc_mean_std(buf, N, mean, std);

    // 3) XOR para LEDs
    bool A = (rms >= TH_RMS);
    bool B = (ptp >= TH_PTP);
    bool Y = A ^ B;

    digitalWrite(LED_XOR_TRUE,  Y ? HIGH : LOW);
    digitalWrite(LED_XOR_FALSE, Y ? LOW  : HIGH);

    // 4) Salva linha no buffer e envia pelo Serial
    String label = "normal";  // altere se necessário
    String line = String(mean,4) + "," + String(std,4) + "," + String(rms,4) + "," + String(ptp,4) + "," + label;
    csvBuffer[lineCount++] = line;
    Serial.println(line);

    // 5) Quando atingir 100 linhas, avisa e reseta buffer
    if (lineCount >= MAX_LINES) {
        Serial.println("100 linhas coletadas! CSV pronto para salvar no PC.");
        lineCount = 0;
    }

    delay(500); // pausa antes da próxima janela
}
