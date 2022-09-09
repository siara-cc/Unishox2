/*
 * Copyright (C) 2022 Siara Logics (cc)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Arundale Ramanathan
 *
 */

/**
 * @file usx3_progmem.h
 * @author Arundale Ramanathan
 * @brief Define the PROGMEM macro for all platforms
 *
 * This file is for defining the PROGMEM macro\n
 * that is used to store strings into Flash to save RAM space.\n
 */

#ifndef usx3_progmem
#define usx3_progmem

#if defined(ARDUINO)
#include <WString.h>
#elif defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP_PLATFORM) // defined(ESP32) || defined(ESP8266) || defined(ESP32S2) || defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6) || defined(ESP32H2)
#include <pgmspace.h>
#else
#define PROGMEM 
#endif

#endif
