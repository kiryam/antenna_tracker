#include <stdio.h>

#include "servo.h"
#include "gui.h"
#include "gfx.h"

#include "../controls.h"
#include "../tracking.h"
#include "../stepper.h"

static GWidgetInit wi;
static GHandle ghServoBearingTuning;
static int16_t last_encoder_value;
static GHandle ghContainerServoTuning;
void ServoTuningRender();


void ServoTuningCreate(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

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
	ghServoBearingTuning = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoBearingTuning, 60, "BTuning:");
}

void ServoTuningDestroy(){

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
	if( btn2 ){
		btn2 = false;
		//renderer = RENDER_INFO;
		//switchScreen(encoder_value);
		return;
	}

	if ( encoder_value != last_encoder_value ){
		if (encoder_value > last_encoder_value) {
			if( BearingTuning < 90 ){
				BearingTuning = BearingTuning + STEPPER_MIN_ANGILE;
			}
		} else {
			if (BearingTuning > -90 ) {
				BearingTuning = BearingTuning - STEPPER_MIN_ANGILE;
			}
		}

		last_encoder_value = encoder_value;
	}
	char tmp[32] ={0};
	sprintf(tmp,"%f", BearingTuning);
	gwinSetText(ghServoBearingTuning, tmp, TRUE);
}
