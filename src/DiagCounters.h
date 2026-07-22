#ifndef DiagCounters_H
#define DiagCounters_H

#include <stdint.h>

// Temporary, passive diagnostic counters for tracking down the slow heap
// decline reported in the field. Each one is only ever incremented by +1 at
// a single call site - no other behavior is changed. Exposed read-only via
// the existing /fs endpoint (see BrewPiLess.cpp) so they can be correlated
// against the free-heap trend without needing a serial connection.
//
// Safe to remove this whole file (and its increments) once the leak is
// found; nothing else depends on it.

extern volatile uint32_t diagExtHandlerBodyStarts;   // ExternalDataHandler::handleBody() index==0 (malloc)
extern volatile uint32_t diagExtHandlerFrees;        // ExternalDataHandler::_freeBuffer() actual free()
extern volatile uint32_t diagGravityReportCalls;     // ExternalData::processGravityReport()
extern volatile uint32_t diagDuckDnsUpdateCalls;     // DuckDNSUpdaterClass::sendUpdateRequest()
extern volatile uint32_t diagBleScanRestarts;        // BleScanner::_startScan()
extern volatile uint32_t diagDataLoggerSends;        // DataLogger::sendData()
extern volatile uint32_t diagMqttPublishCalls;        // MqttRemoteControl::_reportData()
extern volatile uint32_t diagPeriodicalReports;      // periodicalReport() / WebSocket status push
extern volatile uint32_t diagWsTextAllCalls;         // stringAvailable() -> ws.textAll()
extern volatile uint32_t diagWsCleanupClients;       // ws.cleanupClients() invocations

#endif
