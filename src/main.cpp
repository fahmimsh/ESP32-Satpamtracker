#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define LVGL_TICK_PERIOD 60
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
//PN532 nfc(pn532_i2c);

//Ticker tick; /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//Screen Function
void MenuScreen();
void WiFiScreen();
void TrackingScreen();
void SettingsScreen();
void PeminjamanScreen();
void LoginScreen();
void LogoutScreen();
void TapScreen();
void TrackingScreen_2();
void LoginScreen_2();

//Callback Function
static void Menu_cb(lv_obj_t* obj, lv_event_t event);
static void WiFi_cb(lv_obj_t* obj, lv_event_t event);
static void Login_cb(lv_obj_t* obj, lv_event_t event);
static void Logout_cb(lv_obj_t* obj, lv_event_t event);
static void Settings_cb(lv_obj_t* obj, lv_event_t event);
static void Peminjaman_cb(lv_obj_t* obj, lv_event_t event);
static void Tracking_cb(lv_obj_t* obj, lv_event_t event);
static void NavButton_cb(lv_obj_t* obj, lv_event_t event);
static void Tap_cb(lv_obj_t* obj, lv_event_t event);

//Object Function
void showObjTop();
void showObjMain();
void showObjBottom();

/**********************
*  STATIC VARIABLES
**********************/
static lv_indev_t * kb_indev;

//Screen declaration
static lv_obj_t *scrMenu;
static lv_obj_t *scrWiFi;
static lv_obj_t *scrLogin;
static lv_obj_t *scrSettings;
static lv_obj_t *scrPeminjaman;
static lv_obj_t *scrTracking;
static lv_obj_t *scrTap;

/*	Global Object	*/
//Background
static lv_obj_t *bckTop;
static lv_obj_t *bckMain;
static lv_obj_t *bckBottom;

//Button
static lv_obj_t *btnBack;
static lv_obj_t *btnHome;

//Keyboard
static lv_obj_t *keyboard;

//Text Area
lv_obj_t *textarea;

//Style
static lv_style_t styleTopBar;

/*	Menu screen object	*/
//Style
static lv_style_t styleMenu;
static lv_style_t styleBtnMenus;
static lv_style_t styleBGMenus;
static lv_style_t styleLabelSatpam;

//Object
static lv_obj_t *btnMenus;
static lv_obj_t *lblNama;
static lv_obj_t *lblNIP;

/* WiFi screen object */

//Object
static lv_obj_t *listWiFi;
static lv_obj_t *btnWiFis;
static lv_obj_t *msgboxWiFi;

//Style

/*Login screen object*/
//Object
static lv_obj_t *msgboxLogin;
static lv_obj_t *textareaUser;
static lv_obj_t *textareaPassword;

/*Settings screen object*/
//Object
static lv_obj_t *sliderBrightness;
static lv_obj_t *lblBrightness;
static lv_obj_t *bckBrightness;
static lv_obj_t *lblValBrightness;

static lv_obj_t *sliderBuzzer;
static lv_obj_t *lblBuzzer;
static lv_obj_t *bckBuzzer;
static lv_obj_t *lblValBuzzer;

static lv_obj_t *btnCheckID;
static lv_obj_t *msgboxCheckID;

/*Tracking screen object*/
//Object
static lv_obj_t *tabviewTracking;
static lv_obj_t *tableTracking;

/*Peminjaman screen object*/
static lv_obj_t *tabviewPeminjaman;
static lv_obj_t *tablePeminjaman;

/*Tap Screen*/
static lv_obj_t *lblTapClass;
static lv_obj_t *lblTapStatus;
static lv_obj_t *btnTapLanjut;
static lv_obj_t *btnTapProblem;

/*Top Screen*/
static lv_obj_t *lblClock;
static lv_obj_t *lblDate;
static lv_obj_t *barBattery;
static lv_obj_t *lblBattery;

lv_obj_t * slider_label;
int screenWidth = 480;
int screenHeight = 320;
uint8_t statusScreen = 0;
  
#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

void printEvent(String Event, lv_event_t event)
{
  
  Serial.print(Event);
  printf(" ");

  switch(event) {
      case LV_EVENT_PRESSED:
          printf("Pressed\n");
          break;

      case LV_EVENT_SHORT_CLICKED:
          printf("Short clicked\n");
          break;

      case LV_EVENT_CLICKED:
          printf("Clicked\n");
          break;

      case LV_EVENT_LONG_PRESSED:
          printf("Long press\n");
          break;

      case LV_EVENT_LONG_PRESSED_REPEAT:
          printf("Long press repeat\n");
          break;

      case LV_EVENT_RELEASED:
          printf("Released\n");
          break;
  }
}

void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{

  printEvent("Slider", event);

  if(event == LV_EVENT_VALUE_CHANGED) {
      static char buf[4];                                 /* max 3 bytes  for number plus 1 null terminating byte */
      snprintf(buf, 4, "%u", lv_slider_get_value(slider));
      lv_label_set_text(slider_label, buf);               /*Refresh the text*/
  }
}
/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if(!touched)
    {
      return false;
    }

    if(touchX>screenWidth || touchY > screenHeight)
    {
      Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);
    }
    else
    {

      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
  
      /*Save the state and save the pressed coordinate*/
      //if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);
     
      /*Set the coordinates (if released use the last pressed coordinates)*/
      data->point.x = touchX;
      data->point.y = touchY;
  
      Serial.print("Data x");
      Serial.println(touchX);
      
      Serial.print("Data y");
      Serial.println(touchY);

    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

bool initSPIFFS(){
	if(!SPIFFS.begin(true)){
		return false;
	}
	return true;
}

void Task_NFC(lv_task_t *task){
  uint32_t *user_data = (uint32_t*)task->user_data;

  if(nfc.tagPresent()){
	  Serial.println("ada kartu");
	  NfcTag tag = nfc.read();
	  tag.print();
  }
}

void Task_Clock(lv_task_t *task){
	static uint8_t hour, minute, second;
	static char secondBuf[3], minuteBuf[3], hourBuf[3];
	static char timeBuf[10];

	second++;
	if(second == 60){
		second = 0;
		minute++;
	}
	if(minute == 60){
		minute = 0;
		hour++;
	}

	if(second < 10){
		secondBuf[0] = '0';
		secondBuf[1] = '0' + second; 
	} else sprintf(secondBuf, "%d", second);

	if(minute < 10){
		minuteBuf[0] = '0';
		minuteBuf[1] = '0' + minute;
	} else sprintf(minuteBuf, "%d", minute);

	if(hour < 10){
		hourBuf[0] = '0';
		hourBuf[1] = '0' + hour;
	} else sprintf(hourBuf, "%d", hour);

	sprintf(timeBuf, "%s:%s:%s", hourBuf, minuteBuf, secondBuf);
	lv_label_set_text(lblClock, timeBuf);
	
}

void setup() {
  ledcSetup(10, 5000/*freq*/, 12 /*resolution*/);
  ledcAttachPin(32, 10);
  analogReadResolution(12);
  ledcWrite(10, 819);

  Serial.begin(115200); /* prepare for possible serial debug */

  Serial.println("Init nfc...");
  nfc.begin();

  lv_init();

  #if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
  #endif

  tft.begin(); /* TFT init */
  tft.setRotation(3);

  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

  //Set the theme..
  lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);     
  lv_theme_set_act(th);

  lv_obj_t * scr = lv_cont_create(NULL, NULL);
  lv_disp_load_scr(scr);

  //lv_obj_t * tv = lv_tabview_create(scr, NULL);
  //lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  static uint32_t user_data = 10;
  lv_task_t * task = lv_task_create(Task_Clock, 1000, LV_TASK_PRIO_MID, &user_data);
  Serial.println("Mulai...");

  LoginScreen_2();

}

void loop() {
  lv_task_handler(); /* let the GUI do its work */
  delay(5);

  /*if(statusScreen == 1){
	  if(nfc.tagPresent()){
		Serial.println("ada kartu");
	  	NfcTag tag = nfc.read();
	 	tag.print();

		MenuScreen();
	  }
  }*/

}

void showObjTop() {
	lv_style_init(&styleTopBar);
	lv_style_set_bg_color(&styleTopBar, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_opa_scale(&styleTopBar, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_radius(&styleTopBar, LV_STATE_DEFAULT, 0);
	
	bckTop = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_add_style(bckTop, LV_STATE_DEFAULT, &styleTopBar);
	lv_obj_set_pos(bckTop, 0, 0);
	lv_obj_set_height(bckTop, 30);
	lv_obj_set_width(bckTop, 480);

	lblClock = lv_label_create(bckTop, NULL);
	lv_obj_align(lblClock, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_label_set_text(lblClock, "12:00:24");

	lblDate = lv_label_create(bckTop, NULL);
	lv_label_set_text(lblDate, "Selasa, 16 Feb 2021");
	lv_obj_align(lblDate, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_align(lblDate, LV_LABEL_ALIGN_AUTO);

	uint16_t battery = analogRead(35);
	float battPercent = ((float)battery / 4095.0) * 100.0;

	barBattery = lv_bar_create(bckTop, NULL);
	lv_obj_set_size(barBattery, 50, 20);
	lv_obj_align(barBattery, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
	lv_bar_set_range(barBattery, 0, 100);
	//lv_bar_set_value(barBattery, 30, LV_ANIM_OFF);
	lv_bar_set_value(barBattery, (int16_t)battPercent, LV_ANIM_OFF);

	lblBattery = lv_label_create(barBattery, NULL);
	lv_obj_align(lblBattery, NULL, LV_ALIGN_CENTER, 0, 0);
	char buf[5];
	sprintf(buf, "%u%%", lv_bar_get_value(barBattery));
	lv_label_set_text(lblBattery, buf);
}

void showObjMain() {
	lv_style_init(&styleMenu);
	lv_style_set_bg_color(&styleMenu, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_opa_scale(&styleMenu, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_radius(&styleMenu, LV_STATE_DEFAULT, 0);

	bckMain = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_add_style(bckMain, LV_STATE_DEFAULT, &styleMenu);
	lv_obj_set_pos(bckMain, 0, 30);
	lv_obj_set_height(bckMain, 320 - lv_obj_get_height(bckTop) - 40);
	lv_obj_set_width(bckMain, 480);
}
void showObjBottom() {
	bckBottom = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_pos(bckBottom, 0, 280);
	lv_obj_set_size(bckBottom, 480, 40);

	btnBack = lv_btn_create(bckBottom, NULL);
	lv_obj_set_event_cb(btnBack, NavButton_cb);
	lv_obj_align(btnBack, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_btn_set_fit(btnBack, LV_FIT_TIGHT);
	lv_obj_set_style_local_border_opa(btnBack, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_t* lblBack = lv_label_create(btnBack, NULL);
	lv_label_set_text(lblBack, LV_SYMBOL_LEFT);
}

static void NavButton_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == btnBack && event == LV_EVENT_CLICKED) {
		MenuScreen();
	}
}


static void Menu_cb(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_VALUE_CHANGED && obj == btnMenus) {
		uint16_t idBtn = lv_btnmatrix_get_active_btn(obj);

		switch (idBtn) {
		case 0:
			TrackingScreen_2();
			break;
		case 1:
			PeminjamanScreen();
			break;
		case 2:
			SettingsScreen();
			break;
		case 3:
			LogoutScreen();
			break;
		}
	}
}

static void WiFi_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == btnBack && event == LV_EVENT_PRESSED) {
		MenuScreen();
	}
	else if (event == LV_EVENT_CLICKED && lv_list_get_btn_index(listWiFi, btnWiFis) != NULL) {

		msgboxWiFi = lv_msgbox_create(lv_scr_act(), NULL);
		lv_obj_align(msgboxWiFi, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
		lv_msgbox_set_text(msgboxWiFi, "Insert password");

		textarea = lv_textarea_create(msgboxWiFi, NULL);
		lv_obj_align(textarea, NULL, LV_ALIGN_CENTER, 0, 0);
		//lv_obj_set_size(textarea, 200, 27);
		lv_textarea_set_one_line(textarea, true);
		lv_textarea_set_text(textarea, "");
		lv_textarea_set_pwd_mode(textarea, true);
		lv_textarea_set_pwd_show_time(textarea, 300);
		lv_textarea_set_text_align(textarea, LV_LABEL_ALIGN_LEFT);

		static const char* btns[] = { "Apply", "Cancel", "" };
		lv_msgbox_add_btns(msgboxWiFi, btns);
		lv_obj_set_event_cb(msgboxWiFi, WiFi_cb);

		keyboard = lv_keyboard_create(lv_scr_act(), NULL);
		lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2);
		lv_keyboard_set_textarea(keyboard, textarea);

	}
	else if (obj == msgboxWiFi && event == LV_EVENT_VALUE_CHANGED) {
		if (lv_msgbox_get_active_btn(msgboxWiFi) == 0) {
			char pass[30];
			strcpy(pass, lv_textarea_get_text(textarea));
		
		}
		lv_keyboard_set_textarea(keyboard, NULL);
		lv_obj_del(keyboard);
		lv_obj_del(msgboxWiFi);
	}
}

static void Login_cb(lv_obj_t* obj, lv_event_t event) {
	if ((obj == textareaUser || obj == textareaPassword) && event == LV_EVENT_CLICKED) {
		if (strcmp(lv_textarea_get_text(obj), "Username") == 0 || strcmp(lv_textarea_get_text(obj), "Password") == 0)
			lv_textarea_set_text(obj, "");
		keyboard = lv_keyboard_create(lv_scr_act(), NULL);
		lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES - lv_obj_get_height(msgboxLogin) + 20);
		lv_obj_align(keyboard, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
		lv_keyboard_set_cursor_manage(keyboard, true);

		lv_keyboard_set_textarea(keyboard, obj);
		lv_textarea_set_cursor_hidden(obj, false);
	}
	else if (obj == msgboxLogin && event == LV_EVENT_VALUE_CHANGED) {
		if (lv_msgbox_get_active_btn(msgboxLogin) == 0) {
			printf("Username: %s\tPassword: %s\n", lv_textarea_get_text(textareaUser), lv_textarea_get_text(textareaPassword));
			if (keyboard != NULL) {
				lv_keyboard_set_textarea(keyboard, NULL);
				lv_obj_del(keyboard);
			}
			lv_obj_del(msgboxLogin);
			MenuScreen();
			//TapScreen();
		}

	}
}

static void Logout_cb(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_VALUE_CHANGED) {
		if (lv_msgbox_get_active_btn(obj) == 0) {
			lv_obj_del(obj);
			LoginScreen_2();
		}
		else {
			lv_obj_del(obj);
		}
	}
}

static void Settings_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == sliderBrightness && event == LV_EVENT_VALUE_CHANGED) {
		int16_t val = lv_slider_get_value(obj);
		printf("Value Brightness: %d\n", val);
		static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
		snprintf(buf, 4, "%u", val);
		lv_label_set_text(lblValBrightness, buf);
	}
	else if (obj == sliderBuzzer && event == LV_EVENT_VALUE_CHANGED) {
		int16_t val = lv_slider_get_value(obj);
		printf("Value Buzzer: %d\n", val);
		static char buf[4];
		snprintf(buf, 4, "%u", val);
		lv_label_set_text(lblValBuzzer, buf);
	}
	else if (obj == btnCheckID && event == LV_EVENT_VALUE_CHANGED) {
		const char* btnClose[] = {"OK", "Close", ""};
		msgboxCheckID = lv_msgbox_create(bckMain, NULL);
		lv_msgbox_set_text(msgboxCheckID, "ID kartu adalah:");
		lv_msgbox_add_btns(msgboxCheckID, btnClose);
		lv_obj_align(msgboxCheckID, NULL, LV_ALIGN_CENTER, 0, 9);
	}
}

static void Peminjaman_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == tabviewPeminjaman && event == LV_EVENT_VALUE_CHANGED) {
		lv_obj_t *table;
		uint8_t tabId = lv_tabview_get_tab_act(tabviewPeminjaman);
		lv_obj_t *tab = lv_tabview_get_tab(tabviewPeminjaman, tabId);

		//lv_page_set_scrlbar_mode(tab, LV_SCRLBAR_MODE_ON);
		//lv_page_set_scroll_propagation(tab, true);

		table = lv_table_create(tab, NULL);
		lv_table_set_col_cnt(table, 6);
		lv_table_set_row_cnt(table, 6);
		lv_obj_align(table, tab, LV_ALIGN_CENTER, 0, 0);

		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				char buf[4];
				lv_table_set_cell_align(table, i, j, LV_LABEL_ALIGN_CENTER);
				lv_table_set_cell_type(table, i, j, 2);
				sprintf(buf, "%d", tabId + i + j);
				lv_table_set_cell_value(table, i, j, buf);
			}
		}

		lv_table_set_cell_value(table, 0, 0, "Lantai");
		lv_table_set_cell_value(table, 0, 1, "Ruangan");
		lv_table_set_cell_value(table, 0, 2, "Nama");
		lv_table_set_cell_value(table, 0, 3, "NRP");
		lv_table_set_cell_value(table, 0, 4, "Selesai");
		lv_table_set_cell_value(table, 0, 5, "Keterangan");

		switch (tabId) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		}
	}
}
static void Tracking_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == tabviewTracking && event == LV_EVENT_VALUE_CHANGED) {
		lv_obj_t *table;
		uint8_t tabId = lv_tabview_get_tab_act(tabviewTracking);
		lv_obj_t *tab = lv_tabview_get_tab(tabviewTracking, tabId);

		//lv_page_set_scrlbar_mode(tab, LV_SCRLBAR_MODE_ON);
		//lv_page_set_scroll_propagation(tab, true);

		table = lv_table_create(tab, NULL);
		lv_table_set_col_cnt(table, 4);
		lv_table_set_row_cnt(table, 4);
		lv_obj_align(table, tab, LV_ALIGN_CENTER, 0, 0);

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				char buf[4];
				lv_table_set_cell_align(table, i, j, LV_LABEL_ALIGN_CENTER);
				lv_table_set_cell_type(table, i, j, 2);
				sprintf(buf, "%d", tabId + i + j);
				lv_table_set_cell_value(table, i, j, buf);
			}
		}

		lv_table_set_cell_value(table, 0, 0, "Lantai");
		lv_table_set_cell_value(table, 0, 1, "Ruangan");
		lv_table_set_cell_value(table, 0, 2, "Status");
		lv_table_set_cell_value(table, 0, 3, "Keterangan");

		switch (tabId) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		}
	}
}

static void Tap_cb(lv_obj_t* obj, lv_event_t event) {
	if (obj == btnTapLanjut && event == LV_EVENT_CLICKED) {
		MenuScreen();
	} else if (obj == btnTapProblem && event == LV_EVENT_CLICKED) {
		MenuScreen();
	}
}

void TrackingScreen_2() {
	lv_obj_clean(lv_scr_act());
	scrTracking = lv_obj_create(NULL, NULL);
	lv_scr_load(scrTracking);

	showObjTop();
	showObjMain();
	showObjBottom();

	lv_obj_t* lblLokasi = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblLokasi, "Lokasi: ");
	lv_obj_align(lblLokasi, NULL, LV_ALIGN_IN_LEFT_MID, 40,-60);
	
	lv_obj_t* lblLantai = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblLantai, "Lantai: ");
	lv_obj_align(lblLantai, NULL, LV_ALIGN_IN_LEFT_MID, 40, -40);

	lv_obj_t* lblStatus = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblStatus, "Status");
	lv_obj_align(lblStatus, NULL, LV_ALIGN_IN_LEFT_MID, 40, -20);

	lv_obj_t* lblPeminjaman = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblPeminjaman, "Peminjaman: ");
	lv_obj_align(lblPeminjaman, NULL, LV_ALIGN_IN_LEFT_MID, 40, 40);

	lv_obj_t* cbxScanID = lv_checkbox_create(bckMain, NULL);
	lv_checkbox_set_disabled(cbxScanID);
	lv_checkbox_set_text(cbxScanID, "");
	lv_checkbox_set_checked(cbxScanID, false);
	lv_obj_align(cbxScanID, NULL, LV_ALIGN_IN_RIGHT_MID, -80, 0);

	lv_obj_t* lblScanID = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblScanID, "Scan ID untuk melanjutkan");
	lv_obj_align(lblScanID, NULL, LV_ALIGN_IN_RIGHT_MID, -60, 40);

}
void LoginScreen_2() {
	lv_obj_clean(lv_scr_act());
	scrLogin = lv_obj_create(NULL, NULL);
	lv_scr_load(scrLogin);

	showObjTop();
	showObjMain();
	showObjBottom();

	lv_obj_t *lblLogin = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblLogin, "Scan Kartu Untuk Login");
	lv_label_set_align(lblLogin, LV_LABEL_ALIGN_AUTO);
	lv_obj_align(lblLogin, NULL, LV_ALIGN_CENTER, 0, 0);
	statusScreen = 1;

	lv_obj_del(btnBack);
}

void MenuScreen() {
	lv_obj_clean(lv_scr_act());
	scrMenu = lv_obj_create(NULL, NULL);
	lv_scr_load(scrMenu);

	showObjTop();
	showObjMain();
	showObjBottom();

	lv_style_init(&styleBGMenus);
	lv_style_set_opa_scale(&styleBGMenus, LV_STATE_DEFAULT, 0);

	lv_style_init(&styleBtnMenus);
	lv_style_set_opa_scale(&styleBtnMenus, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_text_font(&styleBtnMenus, LV_STATE_DEFAULT, &lv_font_montserrat_12);

	static const char *btnMenus_map[] = {
		"Tracking Satpam", "Peminjaman Ruangan", "\n",
		"Settings", "Logout", "" };

	btnMenus = lv_btnmatrix_create(bckMain, NULL);
	lv_obj_add_style(btnMenus, LV_BTNMATRIX_PART_BG, &styleBGMenus);
	lv_obj_add_style(btnMenus, LV_BTNMATRIX_PART_BTN, &styleBtnMenus);
	lv_btnmatrix_set_map(btnMenus, btnMenus_map);
	lv_obj_set_width(btnMenus, lv_obj_get_width(bckMain));
	lv_obj_set_height(btnMenus, lv_obj_get_height(bckMain));
	lv_obj_align(btnMenus, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(btnMenus, Menu_cb);

	lv_style_init(&styleLabelSatpam);
	lv_style_set_text_font(&styleLabelSatpam, LV_STATE_DEFAULT, &lv_font_montserrat_12);

	lblNama = lv_label_create(bckBottom, NULL);
	lv_obj_add_style(lblNama, LV_LABEL_PART_MAIN, &styleLabelSatpam);
	lv_obj_set_pos(lblNama, 5, 0);
	lv_obj_set_size(lblNama, 480, 0);
	lv_label_set_text(lblNama, "Nama Satpam: ");
	
	lblNIP = lv_label_create(bckBottom, NULL);
	lv_obj_add_style(lblNIP, LV_LABEL_PART_MAIN, &styleLabelSatpam);
	lv_obj_set_pos(lblNIP, 5, 20);
	lv_obj_set_size(lblNIP, 480, 15);
	lv_label_set_text(lblNIP, "NIP Satpam: ");

	statusScreen = 2;

	lv_obj_del(btnBack);
}

void TapScreen() {
	lv_obj_clean(lv_scr_act());
	scrTap = lv_obj_create(NULL, NULL);
	lv_scr_load(scrTap);

	showObjTop();
	showObjMain();

	lblTapClass = lv_label_create(bckMain, NULL);
	lv_obj_align(lblTapClass, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
	lv_label_set_text(lblTapClass, "HH-303");

	lblTapStatus = lv_label_create(bckMain, NULL);
	lv_obj_align(lblTapStatus, NULL, LV_ALIGN_CENTER, 5, 0);
	lv_label_set_text(lblTapStatus, "OK");
	
	btnTapLanjut = lv_btn_create(bckMain, NULL);
	lv_obj_align(btnTapLanjut, NULL, LV_ALIGN_IN_BOTTOM_MID, 70, -10);
	lv_obj_t *lblLanjut = lv_label_create(btnTapLanjut, NULL);
	lv_label_set_text(lblLanjut, "LANJUT");
	lv_btn_set_fit(btnTapLanjut, LV_FIT_TIGHT);
	lv_obj_set_event_cb(btnTapLanjut, Tap_cb);

	btnTapProblem = lv_btn_create(bckMain, NULL);
	lv_obj_align(btnTapProblem, NULL, LV_ALIGN_IN_BOTTOM_MID, -70, -10);
	lv_obj_t *lblProblem = lv_label_create(btnTapProblem, NULL);
	lv_label_set_text(lblProblem, "PROBLEM");
	lv_btn_set_fit(btnTapProblem, LV_FIT_TIGHT);
	lv_obj_set_event_cb(btnTapProblem, Tap_cb);

	showObjBottom();
	lv_obj_del(btnBack);
}

void WiFiScreen() {
	lv_obj_clean(lv_scr_act());
	scrWiFi = lv_obj_create(NULL, NULL);
	lv_scr_load(scrWiFi);

	showObjTop();
	showObjMain();

	listWiFi = lv_list_create(bckMain, NULL);
	lv_obj_set_size(listWiFi, 480, 290);
	lv_obj_align(listWiFi, NULL, LV_ALIGN_CENTER, 0, 0);

	for (int i = 0; i < 35; i++) {
		char txtFmt[20];
		sprintf(txtFmt, "Wifi: %d", i);
		
		btnWiFis = lv_list_add_btn(listWiFi, LV_SYMBOL_WIFI, txtFmt);
		lv_obj_set_event_cb(btnWiFis, WiFi_cb);
	}

	showObjBottom();

	btnHome = lv_btn_create(bckBottom, NULL);
	lv_obj_align(btnHome, NULL, LV_ALIGN_CENTER, -40, 0);
	lv_btn_set_fit(btnHome, LV_FIT_TIGHT);
	lv_obj_t* lblHome = lv_label_create(btnHome, NULL);
	lv_label_set_text(lblHome, LV_SYMBOL_HOME);

	lv_obj_set_event_cb(btnBack, WiFi_cb);
}

void TrackingScreen() {
	lv_obj_clean(lv_scr_act());
	scrTracking = lv_obj_create(NULL, NULL);
	lv_scr_load(scrTracking);

	showObjTop();
	showObjMain();

	lv_obj_t *page = lv_page_create(bckMain, NULL);
	lv_obj_set_size(page, lv_obj_get_width(bckMain), lv_obj_get_height(bckMain));
	
	tableTracking = lv_table_create(page, NULL);
	lv_obj_set_width(tableTracking, lv_page_get_width_fit(page));
	lv_obj_set_height(tableTracking, lv_page_get_height_fit(page));
	lv_page_set_scrlbar_mode(page, LV_SCRLBAR_MODE_DRAG);
	
	lv_table_set_col_cnt(tableTracking, 4);
	lv_table_set_row_cnt(tableTracking, 10);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			char buf[4];
			lv_table_set_cell_align(tableTracking, j, i, LV_LABEL_ALIGN_CENTER);
			lv_table_set_cell_type(tableTracking, j, i, 2);
			sprintf(buf, "%d", i + j);
			lv_table_set_cell_value(tableTracking, j, i, buf);
		}
	}

	lv_table_set_cell_value(tableTracking, 0, 0, "Lantai");
	lv_table_set_cell_value(tableTracking, 0, 1, "Ruangan");
	lv_table_set_cell_value(tableTracking, 0, 2, "Status");
	lv_table_set_cell_value(tableTracking, 0, 3, "Keterangan");

	showObjBottom();
}

void PeminjamanScreen() {
	lv_obj_clean(lv_scr_act());
	scrPeminjaman = lv_obj_create(NULL, NULL);
	lv_scr_load(scrPeminjaman);

	showObjTop();
	showObjMain();

	tabviewPeminjaman = lv_tabview_create(bckMain, NULL);

	lv_obj_t *tab1 = lv_tabview_add_tab(tabviewPeminjaman, "D3");
	lv_obj_t *tab2 = lv_tabview_add_tab(tabviewPeminjaman, "D4");
	lv_obj_t *tab3 = lv_tabview_add_tab(tabviewPeminjaman, "S2");

	lv_obj_set_event_cb(tabviewPeminjaman, Peminjaman_cb);

	showObjBottom();
}

void SettingsScreen() {
	lv_obj_clean(lv_scr_act());
	scrSettings = lv_obj_create(NULL, NULL);
	lv_scr_load(scrSettings);

	showObjTop();
	showObjMain();

	lblBrightness = lv_label_create(bckMain, NULL);
	lv_obj_align(lblBrightness, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);
	lv_label_set_text(lblBrightness, "Brightness");

	sliderBrightness = lv_slider_create(bckMain, NULL);
	lv_obj_align(sliderBrightness, NULL, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_x(lblBrightness) + lv_obj_get_width(lblBrightness) + 20, 10);
	lv_slider_set_range(sliderBrightness, 0, 100);
	
	lblValBrightness = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblValBrightness, "0");
	lv_obj_set_auto_realign(lblValBrightness, true);
	lv_obj_align(lblValBrightness, sliderBrightness, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

	lv_obj_set_event_cb(sliderBrightness, Settings_cb);

	lblBuzzer = lv_label_create(bckMain, NULL);
	lv_obj_align(lblBuzzer, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 25);
	lv_label_set_text(lblBuzzer, "Volume Buzzer");

	sliderBuzzer = lv_slider_create(bckMain, NULL);
	lv_obj_align(sliderBuzzer, NULL, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_x(lblBuzzer) + lv_obj_get_width(lblBuzzer) + 20, 30);
	lv_slider_set_range(sliderBuzzer, 0, 100);

	lblValBuzzer = lv_label_create(bckMain, NULL);
	lv_label_set_text(lblValBuzzer, "0");
	lv_obj_set_auto_realign(lblValBuzzer, true);
	lv_obj_align(lblValBuzzer, sliderBuzzer, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

	lv_obj_set_event_cb(sliderBuzzer, Settings_cb);

	btnCheckID = lv_btn_create(bckMain, NULL);
	lv_obj_align(btnCheckID, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 45);

	lv_obj_t* lblCheckID = lv_label_create(btnCheckID, NULL);
	lv_obj_align(lblCheckID, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(lblCheckID, "CHECK ID");

	lv_obj_set_event_cb(btnCheckID, Settings_cb);

	showObjBottom();
}

void LoginScreen() {
	lv_obj_clean(lv_scr_act());
	scrLogin = lv_obj_create(NULL, NULL);
	lv_scr_load(scrLogin);

	msgboxLogin = lv_msgbox_create(lv_scr_act(), NULL);

	lv_obj_align(msgboxLogin, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
	lv_msgbox_set_text(msgboxLogin, "Login");

	textareaUser = lv_textarea_create(msgboxLogin, NULL);
	lv_textarea_set_text(textareaUser, "Username");
	lv_obj_align(textareaUser, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_textarea_set_one_line(textareaUser, true);
	lv_textarea_set_text_align(textareaUser, LV_LABEL_ALIGN_LEFT);
	lv_textarea_set_cursor_hidden(textareaUser, true);
	lv_obj_set_event_cb(textareaUser, Login_cb);

	textareaPassword = lv_textarea_create(msgboxLogin, NULL);
	lv_textarea_set_text(textareaPassword, "Password");
	lv_obj_align(textareaPassword, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_textarea_set_one_line(textareaPassword, true);
	lv_textarea_set_pwd_mode(textareaPassword, true);
	lv_textarea_set_pwd_show_time(textareaPassword, 1000);
	lv_textarea_set_text_align(textareaPassword, LV_LABEL_ALIGN_LEFT);
	lv_textarea_set_cursor_hidden(textareaPassword, true);
	lv_obj_set_event_cb(textareaPassword, Login_cb);

	static const char* btns[] = { "Apply", "Cancel", "" };
	lv_msgbox_add_btns(msgboxLogin, btns);
	lv_obj_set_event_cb(msgboxLogin, Login_cb);
	lv_obj_set_style_local_text_font(msgboxLogin, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
}

void LogoutScreen() {
	static const char* btns[] = { "OK", "TIDAK", "" };

	lv_obj_t *msgBox = lv_msgbox_create(lv_scr_act(), NULL);
	lv_msgbox_set_text(msgBox, "Apakah anda yakin untuk Logout?");
	lv_msgbox_add_btns(msgBox, btns);
	lv_obj_set_width(msgBox, 200);
	lv_obj_align(msgBox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(msgBox, Logout_cb);
}