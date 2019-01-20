#include "servo_tuning.h"

#include <stdio.h>

#include "gui.h"
#include "gfx.h"
#include "main.h"

#include "../tracking.h"
#include "../servo.h"
#include "settings.h"


static GWidgetInit wi;
static GHandle ghServoElevationTuning;
static int16_t last_encoder_value;
static GHandle ghContainerServoTuning;
static GListener gl;
static GSourceHandle upHandle;

void ServoTuningRender();


static void gwStepperTuningEvent(void *param, GEvent *pe) {
	(void)param;
	if ( pe->type == GEVENT_TOGGLE ){
			switch( ((GEventToggle*)pe)->instance) {
			case BUTTON_ESC:
				ServoHandler();
				break;
			}
		}else if( pe->type == GEVENT_DIAL) {
			switch( ((GEventDial*)pe)->instance) {
			case 0:
				if ( ((GEventDial*)pe)->value != last_encoder_value ){
					if (((GEventDial*)pe)->value > last_encoder_value) {
						if( ElevationTuning < 90 ){
							ElevationTuning = ElevationTuning + SERVO_STEP_ANGILE;
						}
					} else {
						if (ElevationTuning > -90 ) {
							ElevationTuning = ElevationTuning - SERVO_STEP_ANGILE;
						}
					}
					last_encoder_value = ((GEventDial*)pe)->value;
				}
				break;
			}
		}
}

void ServoTuningCreate(){
	geventListenerInit(&gl);
	upHandle = ginputGetDial(0);
	geventAttachSource(&gl, upHandle, 0);

	GSourceHandle upHandle = ginputGetToggle(BUTTON_ESC);
	geventAttachSource(&gl, upHandle, GLISTEN_TOGGLE_ON);

	geventRegisterCallback(&gl, gwStepperTuningEvent, 0);

	gwinWidgetClearInit(&wi);

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerServoTuning = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerServoTuning;
	wi.g.width = gwinGetInnerWidth(ghContainerServoTuning);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoElevationTuning = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoElevationTuning, 60, "ElevationTuning:");
}

void ServoTuningDestroy(){
	UIDestroyContainerWithChilds(ghContainerServoTuning);
	ghContainerServoTuning = NULL;
	//geventDetachSourceListeners(upHandle);
	geventDetachSource(&gl, upHandle);
	vSemaphoreDelete(gl.waitqueue);
}

Page* CreateServoTuningPage(){
	Page* page = pvPortMalloc(sizeof(Page));
	if( page == NULL ){
		return NULL;
	}
	page->Page = PAGE_SERVO_TUNING;
	page->Render = ServoTuningRender;
	page->Create = ServoTuningCreate;
	page->Destroy = ServoTuningDestroy;
	return page;
}

void ServoTuningRender(){
	char tmp[32] ={0};
	sprintf(tmp,"%f", ElevationTuning);
	//gwinSetIntCached(0, ghServoElevationTuning, ElevationTuning, TRUE);
	gwinSetText(ghServoElevationTuning, tmp, TRUE);
}
