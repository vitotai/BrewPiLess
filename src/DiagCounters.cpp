#include "DiagCounters.h"

volatile uint32_t diagExtHandlerBodyStarts = 0;
volatile uint32_t diagExtHandlerFrees = 0;
volatile uint32_t diagGravityReportCalls = 0;
volatile uint32_t diagDuckDnsUpdateCalls = 0;
volatile uint32_t diagBleScanRestarts = 0;
volatile uint32_t diagDataLoggerSends = 0;
volatile uint32_t diagMqttPublishCalls = 0;
volatile uint32_t diagPeriodicalReports = 0;
volatile uint32_t diagWsTextAllCalls = 0;
volatile uint32_t diagWsCleanupClients = 0;
