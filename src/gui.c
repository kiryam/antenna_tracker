#include "gui.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdbool.h"
#include "gfx.h"
#include "telemetry.h"

static GHandle ghContainerTelemetry, ghTelemetryRxGood, ghTelemetryRxBad, ghTelemetryPosLat, ghTelemetryPosLon;
static GHandle ghContainerSystem, ghSystemHeapFree;
static GWidgetInit wi;
static font_t  font;

typedef uint8_t SCREEN;
static const SCREEN SCREEN_TELEMETRY = 1;
static const SCREEN SCREEN_SYSTEM = 2;
static SCREEN active_screen;
TimerHandle_t screen_switch_timer;
static SCREEN screen_sequence[2];
static uint8_t screen_i=0;

void UITelemetryScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerTelemetry = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);

	wi.g.show = TRUE;
	wi.g.parent = ghContainerTelemetry;
	wi.g.width = gwinGetInnerWidth(ghContainerTelemetry);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryRxGood = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryRxGood, 40, "Ok:");

	wi.g.y = 12;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryRxBad = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryRxBad, 40, "Error:");


	wi.g.y = 24;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryPosLat = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryPosLat, 40, "Lat:");

	wi.g.y = 36;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghTelemetryPosLon = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghTelemetryPosLon, 40, "Lon:");
}

void UISystemInfoScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerSystem = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerSystem;
	wi.g.width = gwinGetInnerWidth(ghContainerSystem);

	wi.g.show = TRUE;
	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghSystemHeapFree = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghSystemHeapFree, 60, "HeapFree:");
}

void vTimerScreenSwitch( TimerHandle_t xTimer ) {
	(void) xTimer;
	if ( screen_i++ > sizeof(screen_sequence) ){
		screen_i = 0;
	}
	active_screen = screen_sequence[screen_i];
}

void UICreate(){
	UITelemetryScreen();
	UISystemInfoScreen();

	active_screen = SCREEN_TELEMETRY;
	screen_sequence[0] = SCREEN_SYSTEM;
	screen_sequence[1] = SCREEN_TELEMETRY;

	screen_switch_timer = xTimerCreate ("ScreenSwitchTimer", 1000, pdTRUE, ( void * ) 0, vTimerScreenSwitch);
	xTimerStart( screen_switch_timer, 0);

	/*gdispDrawCircle(20, 40, 10, White);*/
}


void lat_to_char( int32_t degE7, char* buf ){
  // Extract and print negative sign
	uint8_t pos=0;
	uint8_t i=0;
	char tmp[32] ={0};

  if (degE7 < 0) {
    degE7 = -degE7;
    buf[pos++] = '-';
  }

  // Whole degrees
  int32_t deg = degE7 / 10000000L;

  itoa(deg, tmp, 10);
  while(tmp[i] != '\0' ){
	  buf[pos++] = tmp[i++];
  }
  buf[pos++] = '.';

  // Get fractional degrees
  degE7 -= deg*10000000L;

  // Print leading zeroes, if needed
  int32_t factor = 1000000L;
  while ((degE7 < factor) && (factor > 1L)){
	buf[pos++] = '0';
    factor /= 10L;
  }

  // Print fractional degrees
  //Serial.print( degE7 );
  i=0;
  memset(&tmp[0], 0, sizeof(tmp));
  itoa(degE7, tmp, 10);
  while(tmp[i] != '\0' ){
	  buf[pos++] = tmp[i++];
	}
}
void UIRenderTask(void * argunment){
	(void) argunment;

	gfxInit();

	font = gdispOpenFont("UI2");
	gwinSetDefaultFont(font);

	UICreate();
	while(1) {
		char tmp[64] ={0};
		if (active_screen == SCREEN_TELEMETRY ){
			gwinHide(ghContainerSystem);
			gwinShow(ghContainerTelemetry);

			itoa(telemetry.rx_good, tmp, 10);
			gwinSetText(ghTelemetryRxGood, tmp, TRUE);

			itoa(telemetry.rx_bad, tmp, 10);
			gwinSetText(ghTelemetryRxBad, tmp, TRUE);

			lat_to_char(telemetry.current_messages.global_position_int.lat, tmp);
			gwinSetText(ghTelemetryPosLat, tmp, TRUE);

			lat_to_char(telemetry.current_messages.global_position_int.lon, tmp);
			gwinSetText(ghTelemetryPosLon, tmp, TRUE);
		} else if (active_screen == SCREEN_SYSTEM ){
			gwinHide(ghContainerTelemetry);
			gwinShow(ghContainerSystem);

			itoa(xPortGetFreeHeapSize(), tmp, 10);
			gwinSetText(ghSystemHeapFree, tmp, TRUE);
		}
		gfxSleepMilliseconds(100);
	}
}
