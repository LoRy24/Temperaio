//
// Temperaio v1.0.0
// Author: Lorenzo Rocca
//
// Copyright (C) 2025 - Lorenzo Rocca
//

//
// Inclusioni e Macro
//

#pragma region Inclusioni e Macro

// Librerie standard
#include    <stdio.h>
#include    <stdlib.h>
#include    <math.h>
#include    <unistd.h>
#include    <time.h>
#include    <windows.h>

// Macro
#define     MEASUREMENT_DELAY_SECONDS           60*30
#define     DEBUG_MEASUREMENT_DELAY_SECONDS     1
#define     MEASUREMENT_DAYS                    7
#define     DEBUG_MODE                          1

// Colori
#define     BLK                                 "\e[0;30m"
#define     RED                                 "\e[0;31m"
#define     GRN                                 "\e[0;32m"
#define     YEL                                 "\e[0;33m"
#define     BLU                                 "\e[0;34m"
#define     MAG                                 "\e[0;35m"
#define     CYN                                 "\e[0;36m"
#define     WHT                                 "\e[0;37m"
#define     BBLK                                "\e[1;30m"
#define     BRED                                "\e[1;31m"
#define     BGRN                                "\e[1;32m"
#define     BYEL                                "\e[1;33m"
#define     BBLU                                "\e[1;34m"
#define     BMAG                                "\e[1;35m"
#define     BCYN                                "\e[1;36m"
#define     BWHT                                "\e[1;37m"
#define     COLOR_RESET                         "\e[0m"

#pragma endregion

//
// Dati Globali
//

#pragma region Dati Globali

//
// Contatori globali
//
int currentMeasurement = 0;
int currentDay = 0; // 0 ... MISURATION_DAYS - 1

//
// Riferimenti globali
//
int measurationDelay = 0;

//
// Primo parametro: Giorno misurazioni
// Secondo parametro: n. Misurazione
//
float measures[MEASUREMENT_DAYS][48];

//
// Primo parametro: Giorno misurazioni
// Secondo parametro:
// - 0: Media
// - 1: Minimo
// - 2: Massimo
//
// Terzo Parametro:
// - 0: Prima misura
// - 1: Seconda misura
//
float middleDayValues[MEASUREMENT_DAYS][3][2];

//
// Primo parametro: Giorno misurazioni
// Secondo parametro:
// - 0: Media
// - 1: Minimo
// - 2: Massimo
//
float dayResults[MEASUREMENT_DAYS][3];

#pragma endregion

//
// Logging
//

#pragma region Logging

/**
 * Questa funzione invia un messaggio di debug se la modalità è stata avviata
 * 
 * @param message Il messaggio da scrivere
 */
void logDebugMessage(const char* message) {
    if (DEBUG_MODE) {
        printf(BYEL "[DEBUG]" YEL " %s\n" COLOR_RESET, message);
    }
}

/**
 * Questa funzione invia un messaggio informativo
 * 
 * @param message Il messaggio da scrivere
 */
void logInfoMessage(const char* message) {
    printf(BMAG "[INFO]" MAG " %s\n", message);
}

/**
 * Questa funzione invia un messaggio di Warning
 * 
 * @param message Il messaggio da scrivere
 */
void logWarnMessage(const char* message) {
    printf(BRED "[WARNING]" RED " %s\n" COLOR_RESET, message);
}

/**
 * Questa funzione invia un messaggio riguardante la lettura da parte di un sensore
 * 
 * @param message Il messaggio da scrivere
 */
void logSensorMessage(const char* message) {
    printf(BCYN "[SENSOR]" CYN " %s\n" COLOR_RESET, message);
}

void logSuccessMessage(const char* message) {
    printf(BGRN "[SUCCESS]" GRN " %s\n" COLOR_RESET, message);
}

#pragma endregion

//
// Funzioni Utilitarie
//

#pragma region Funzioni Utilitarie

// Logging

void logTemperatureValue(float value) {
    // Costruisci il messaggio
    char* temperatureSensorMessage = malloc(512);
    sprintf(temperatureSensorMessage, "Misurazione temperatura #%02d del giorno %02d: %.3f °C", currentMeasurement + 1, currentDay + 1, value);
    
    // Invia il messaggio
    logSensorMessage(temperatureSensorMessage);

    // Libera la memoria
    free(temperatureSensorMessage);
}

void logTermStats(float average, float minTemperature, float maxTemperature) {
    // Buffer
    char* buffer = malloc(1024);

    // Temperatura media
    sprintf(buffer, "Temperatura media: %.3f °C", average);
    logInfoMessage(buffer);

    // Temperatura minima
    sprintf(buffer, "Temperatura minima: %.3f °C", minTemperature);
    logInfoMessage(buffer);

    // Temperatura massima
    sprintf(buffer, "Temperatura massima: %.3f °C", maxTemperature);
    logInfoMessage(buffer);

    // Pulisci la memoria
    free(buffer);
}

// Analisi

float findMaxTemperature(int rangeStart, int rangeEnd) {
    float foundMaxTemperature = measures[currentDay][rangeStart];

    for (int i = rangeStart + 1; i < rangeEnd; ++i) {
        float current = measures[currentDay][i];
        if (foundMaxTemperature < current) {
            foundMaxTemperature = current;
        }
    }

    return foundMaxTemperature;
}

float findAverageTemperature(int rangeStart, int rangeEnd) {
    int n = rangeEnd - rangeStart;
    float averageTemperature = .0f;

    for (int i = rangeStart; i < rangeEnd; ++i) {
        float current = measures[currentDay][i];
        averageTemperature += current / n;
    }

    return averageTemperature;
}

float findMinTemperature(int rangeStart, int rangeEnd, float maxTemperature) {
    float foundMinTemperature = maxTemperature;

    for (int i = rangeStart + 1; i < rangeEnd; ++i) {
        float current = measures[currentDay][i];
        if (foundMinTemperature > current) {
            foundMinTemperature = current;
        }
    }

    return foundMinTemperature;
}

// Calcoli

void saveMiddleTermStats(int termId) {
    // Valori
    int rangeStart = termId * 24;
    int rangeEnd = rangeStart + 24;

    // Valori da salvare
    float averageTemperature = findAverageTemperature(rangeStart, rangeEnd);
    float maxTemperature = findMaxTemperature(rangeStart, rangeEnd);
    float minTemperature = findMinTemperature(rangeStart, rangeEnd, maxTemperature);

    // Salva i valori nel termine
    middleDayValues[currentDay][0][termId] = averageTemperature;
    middleDayValues[currentDay][1][termId] = minTemperature;
    middleDayValues[currentDay][2][termId] = maxTemperature;
}

void saveDailyTermStats() {
    // Valori da salvare
    float averageTemperature = (middleDayValues[currentDay][0][0] + middleDayValues[currentDay][0][1]) / 2;
    float minTemperature = max(middleDayValues[currentDay][1][0], middleDayValues[currentDay][1][1]);
    float maxTemperature = max(middleDayValues[currentDay][2][0], middleDayValues[currentDay][2][1]);

    // Salva i valori
    dayResults[currentDay][0] = averageTemperature;
    dayResults[currentDay][1] = minTemperature;
    dayResults[currentDay][2] = maxTemperature;
}

float measureTemperature() {
    // Parte intera della temperatura
    float temperatureInt = (rand() % 100) - 25.0f; // Compreso fra 75 e -25

    // Parte decimale
    float temperatureDecimal = (rand() % 999) / 100.0f;

    return temperatureInt + temperatureDecimal;
}

// Fetch

float getTermAverage(int day, int term) {
    return middleDayValues[day][0][term];
}

float getTermMin(int day, int term) {
    return middleDayValues[day][1][term];
}

float getTermMax(int day, int term) {
    return middleDayValues[day][2][term];
}

float getDayAverage(int day) {
    return dayResults[day][0];
}

float getDayMin(int day) {
    return dayResults[day][1];
}

float getDayMax(int day) {
    return dayResults[day][2];
}

#pragma endregion

//
// Routine
//

#pragma region Routine

void launchDailyMeasureProcess() {
    for (currentMeasurement = 0; currentMeasurement < 48; ++currentMeasurement) {
        sleep(measurationDelay);

        // Leggi la temperatura e notifica
        float temperature = measureTemperature();
        logTemperatureValue(temperature);

        // Salva la temperatura misurata
        measures[currentDay][currentMeasurement] = temperature;
        
        if (currentMeasurement == 23 || currentMeasurement == 47) {
            int termId = currentMeasurement / 24;
            saveMiddleTermStats(termId);

            logInfoMessage("Completato un ciclo di 24 misure (12 ore)! Esiti:");
            logTermStats(getTermAverage(currentDay, termId), getTermMin(currentDay, termId), getTermMax(currentDay, termId));
        }
    }
}

void launchMeasureProcess() {
    for (currentDay = 0; currentDay < MEASUREMENT_DAYS; ++currentDay) {
        launchDailyMeasureProcess();
        saveDailyTermStats();
        logInfoMessage("Completato un ciclo di misure giornaliero! Esiti:");
        logTermStats(getDayAverage(currentDay), getDayMin(currentDay), getDayMax(currentDay));
    }

    logSuccessMessage("Ciclo di misure completato!");
}

int main(void) {
    // Imposta il charset in UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Debug
    if (!DEBUG_MODE) {
        system("cls");
        measurationDelay = MEASUREMENT_DELAY_SECONDS;
    }
    else {
        logDebugMessage("Debug Mode abilitata!");
        measurationDelay = DEBUG_MEASUREMENT_DELAY_SECONDS;
    }

    // Seed random
    srand(time(NULL));

    // Avvia il programma
    launchMeasureProcess();
    
    // Chiusura programma
    system("PAUSE");
    return 0;
}

#pragma endregion
