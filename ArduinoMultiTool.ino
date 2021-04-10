//Setting up mouse for clicking
//#include <Mouse.h>
//Use HID-Project insted of original libary. This alows more flexebility and volum control at the same time. 
#include <HID-Project.h>
//Include Wire library for I2C
#include <Wire.h>
//Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Define keyboard layout
#define HID_CUSTOM_LAYOUT
#define LAYOUT_NORWEGIAN

#define SWI 128 // OLED display width, in pixels
#define SHE  64 // OLED display height, in pixels

//Reset pin not used but needed for library
#define OLED_RESET -1
Adafruit_SSD1306 display(SWI, SHE, &Wire, OLED_RESET);

//Set up values used for text offset
int16_t x1, y1;
uint16_t w1, h1; 

//setting up values for offsets that needs to be stored.
uint16_t clkw1 = 0;
uint16_t clkh1 = 0;
uint16_t clkoffw = 0;
uint16_t clkoff;
char snum[5];

//Settin gup pins for menu buttons
#define BUTTONmen1 4
#define BUTTONmen2 6
#define BUTTONmode 5
#define BUTTONback 7

// Rotary Encoder Inputs
#define CLK 14
#define DT 16
#define SW 10

//Setting up values used later in the code
int buttonState;
int buttonStateSpeed = 1;
int onoff = false;
int last = true;
int stateMen1;
int stateMen2;
int stateBack;
bool backMode = false;

//setting up button states. i Know this code is gay and i will fix it. TBD
int lastButtonModeState;
int buttonStateMode = LOW;
int lastButtonMen1State;
int buttonStateMen1 = LOW;
int lastButtonMen2State;
int buttonStateMen2 = LOW;
int lastButtonBackState;
int buttonStateBack = LOW;

//setting up values for menu
int menu = 1;
bool menuChange = true;

//debunce
long lastDebounceTime = 0; // the last time the output pin was toggled
long lastDebounceTimeMen1 = 0; // the last time the output pin was toggled
long lastDebounceTimeMen2 = 0; // the last time the output pin was toggled
long lastDebounceTimeBack = 0; // the last time the output pin was toggled
long debounceDelay = 50;  // the debounce time; increase if the output flickers

//Setting up rotary encoder values
int counter = 1;
int currentStateCLK;
int lastStateCLK;
unsigned long lastButtonPress = 0;
int rotSpeed = 1;
int rotSpeedOffset = 1;
uint16_t rotw1 = 0;
uint16_t roth1 = 0;
int count;
bool longPress = false;

int tcounter = 1;
int num = 0;
bool clickState = false;

//Bit maps
static const unsigned char PROGMEM lockIMG[] = {
 B00000000, B00000000,
 B00000000, B00000000,
 B00000001, B10000000,
 B00000010, B01000000,
 B00000100, B00100000,
 B00000100, B00100000,
 B00001111, B11110000,
 B00010000, B00001000,
 B00010001, B10001000,
 B00010010, B01001000,
 B00010010, B01001000,
 B00010001, B10001000,
 B00010000, B00001000,
 B00001111, B11110000,
 B00000000, B00000000,
 B00000000, B00000000,
};
static const unsigned char PROGMEM prevIMG[] = {
 B01000110,
 B01001010,
 B01010010,
 B01100010,
 B01100010,
 B01010010,
 B01001010,
 B01000110,
};
static const unsigned char PROGMEM nextIMG[] = {
 B01100010,
 B01010010,
 B01001010,
 B01000110,
 B01000110,
 B01001010,
 B01010010,
 B01100010,
};
static const unsigned char PROGMEM plauPauseIMG[] = {
 B00110000, B11001100,
 B00111000, B11001100,
 B00111100, B11001100,
 B00111110, B11001100,
 B00111110, B11001100,
 B00111100, B11001100,
 B00111000, B11001100,
 B00110000, B11001100,
};
static const unsigned char PROGMEM soundIMG[] = {
 B00010010,
 B00110001,
 B11110101,
 B11110101,
 B11110101,
 B00110001,
 B00010010,
 B00000000,
};
static const unsigned char PROGMEM sleepIMG[] = {
 B00111110,
 B01111000,
 B11110000,
 B11110000,
 B11110000,
 B11110000,
 B01111000,
 B00111110,
};


void setup() {
  //Start Wire library for I2C
  Wire.begin();
  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //Setting up menu buttons
  pinMode(BUTTONmen1, INPUT_PULLUP);
  pinMode(BUTTONmen2, INPUT_PULLUP);
  pinMode(BUTTONmode, INPUT_PULLUP);
  pinMode(BUTTONback, INPUT_PULLUP);

	// Set encoder pins as inputs
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	pinMode(SW, INPUT_PULLUP);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  //uses HID-project mouse
  Mouse.begin(); //Starting mosue
  //Starts Consumer for media controls. 
  Consumer.begin();
  //Start system to be able to put pc in sleep mode
  System.begin();
  
  //Setting up first display.
  menuDisplay();
}

// the loop function runs over and over again forever
void loop() {
  toggle();
  //mode select

  //menu logic
  if(menu == 1){
    if(onoff && !longPress){
      click();
    } else if(!longPress) {
      rotary();
      displ();
      menubutton();
      modeChange();
    }
  } else if (menu == 2) {
      rotary();
      menubutton();
      modeChange();
  }
}

void toggle() {
  buttonState = rotaryButton(); //gets if the rotary button is pressed 

  //checks if last is false and current is True and if onoff is true
  if (menu == 1) {
    if ((!last && buttonState) && onoff && !longPress) {
      onoff = false;
      if (menu == 1)
        updateClickSpeed(counter);
        display.display();
    }
    //checks if last is false and current is True and if onoff is false
    else if ((!last && buttonState) && !onoff && !longPress) {
      onoff = true;
      if (menu == 1){
        clickOn();
        display.display(); 
      }
    } else if (longPress && !onoff){
      onoff = true;
    } else if ((!last && buttonState) && onoff && longPress){
      onoff = false;
      longPress = false;
      updateClickSpeed(1001);
      display.display();
      if (clickState)
        Mouse.release(MOUSE_RIGHT);
      else
        Mouse.release();
    }
  } else if(menu == 2){
    if (!last && buttonState){
      Consumer.write(MEDIA_PLAY_PAUSE);
    } 
  }

  last = buttonState;
}

void rotary()
{
	// Read the current state of CLK
	currentStateCLK = digitalRead(CLK);
	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
        if (menu == 1){
          counter = counter + rotSpeed;
          if (counter > 1000)
            counter = 1000;
        } else if (menu == 2){
          Consumer.write(MEDIA_VOLUME_UP);
        }
     
		} else {
      if (menu == 1) {
			  counter = counter - rotSpeed;
        if (counter < 1)
          counter = 1;
      } else if (menu == 2){
        Consumer.write(MEDIA_VOLUME_DOWN);
      }

		}
	}
  // Remember last CLK state
	lastStateCLK = currentStateCLK;
	// Put in a slight delay to help debounce the reading
	delay(1);
}

bool rotaryButton() {
	// Read the button state
	int btnState = digitalRead(SW);
	//If we detect LOW signal, button is pressed
	if (btnState == LOW) {
    //Check if a long press is preformed
    if(menu ==1 && !longPress && !onoff){
        while (digitalRead(SW) == LOW) {
            count+=1;
            if (count >= 500){
              longPress = true;
              clickOn();
              display.display();
              if (clickState)
                Mouse.press(MOUSE_RIGHT);
              else
                Mouse.press();
              while (digitalRead(SW) == LOW) count+=1;
              count = 0;
              lastButtonPress = millis();
              return false;
            } 
            delay(1);
        }
        count = 0;
      }
		//if 50ms have passed since last LOW pulse, it means that the
		//button has been pressed, released and pressed again
		if (millis() - lastButtonPress > 50) {
      lastButtonPress = millis();
      return true;
		}
		// Remember last button press event
		lastButtonPress = millis();
	}
  return false;
}

void click() {
    if (clickState)
      Mouse.click(MOUSE_RIGHT);
    else
      Mouse.click();
    for (int i = 0; i < 1000/counter; ++i){
      delay(1);
      toggle();
      if(!onoff){
        break;
      }
    }   
}

void displ() {
    if (counter != num) {
     if ((tcounter != counter)) {
       updateClickSpeed(counter);
       display.display();
       tcounter = counter;
     }
    } 
    num = counter;
}

void menubutton() {
  stateMen1 = digitalRead(BUTTONmen1); 
  stateMen2 = digitalRead(BUTTONmen2);
  stateBack = digitalRead(BUTTONback);

  if (menu == 1){
    if (stateMen1 == HIGH){
      if(rotSpeed > 0){
        rotSpeed--;
        rotSpeedF();
        display.display();
        delay(100);
      }
    } else if (stateMen2 == HIGH){
      rotSpeed++;
      rotSpeedF();
      display.display();
      delay(100);
    }
    menubuttonBack();
  } else if (menu ==2){
    menubuttonMen1();
    menubuttonMen2();
    menubuttonBack();
  }
}

void menubuttonMen1() {
  // If the switch changed, due to noise or pressing:
  if (stateMen1 != lastButtonMen1State) {
    // reset the debouncing timer
    lastDebounceTimeMen1 = millis();
  }

  if ((millis() - lastDebounceTimeMen1) > debounceDelay) {
    // if the button state has changed:
    if (stateMen1 != buttonStateMen1) {
      buttonStateMen1 = stateMen1;
      // only toggle the LED if the new button state is HIGH
      if (buttonStateMen1 == HIGH) {
        Consumer.write(MEDIA_PREVIOUS);
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonMen1State = stateMen1;

}
void menubuttonMen2() {
  // If the switch changed, due to noise or pressing:
  if (stateMen2 != lastButtonMen2State) {
    // reset the debouncing timer
    lastDebounceTimeMen2 = millis();
  }

  if ((millis() - lastDebounceTimeMen2) > debounceDelay) {
    // if the button state has changed:
    if (stateMen2 != buttonStateMen2) {
      buttonStateMen2 = stateMen2;
      // only toggle the LED if the new button state is HIGH
      if (buttonStateMen2 == HIGH) {
        Consumer.write(MEDIA_NEXT);
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonMen2State = stateMen2;

}
void menubuttonBack() {
  // If the switch changed, due to noise or pressing:
  if (stateBack!= lastButtonBackState) {
    // reset the debouncing timer
    lastDebounceTimeBack = millis();
  }

  if ((millis() - lastDebounceTimeBack) > debounceDelay) {
    // if the button state has changed:
    if (stateBack != buttonStateBack) {
      buttonStateBack = stateBack;
      // only toggle the LED if the new button state is HIGH
      if (buttonStateBack == HIGH) {
        if (menu == 1 && !onoff && !longPress)
            clickShift();        
        else if (menu == 2){
          count = 0;
          while (digitalRead(BUTTONback) == HIGH) {
            count+=1;
            if (count >= 2000){
              System.write(SYSTEM_SLEEP);
              while (digitalRead(BUTTONback) == HIGH) count+=1;
              backMode = true;
            }
            delay(1);
        }
        count = 0;
        if (!backMode)
          Consumer.write(MEDIA_VOLUME_MUTE);
        else if(backMode) 
          backMode = false;
        } 
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonBackState = stateBack;
}

void clickShift() {
    uint16_t w11, h11; 
    if(!menuChange)
      clickState = !clickState;
    String shiftText = "(r)";
    display.getTextBounds(shiftText, 0, 0, &x1, &y1, &w11, &h11);
    display.getTextBounds("Mode", 0, 0, &x1, &y1, &w1, &h1);
    display.fillRect((SWI/2) + (w1/2)+4,2,w11,h11, BLACK);
    if (clickState){
     display.setTextColor(WHITE);
     display.setCursor((SWI/2) + (w1/2)+4, 2);
     display.print(shiftText);
    }
    display.display();
}

void modeChange(){
   // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTONmode);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonModeState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonStateMode) {
      buttonStateMode = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonStateMode == HIGH) {
        if (menu < 2)
          menu++;
        else menu = 1;
        menuChange = true;
        menuDisplay();
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonModeState = reading;
}

void menuDisplay() {
  if (menuChange) {
    if (menu == 1){
      clickmenu();
      clickShift();
      menuChange = false;
    } else if (menu == 2){
      volummenu();
      menuChange = false;
    }
    display.display();
  }
}

void clickmenu(){
  //setting up values and strings for top row
  String left = "-";
  String middle = "Mode";
  String right = "+";
  //clearing display and making it ready
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.clearDisplay();

  //printing left corner
  display.setCursor(2, 2);
  display.print(left);
  //printing middle
  display.getTextBounds(middle , 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((SWI/2) - (w1/2), 2);
  display.print(middle);
  //print right
  display.getTextBounds(right, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor(SWI - (w1 + 2),2);
  display.print(right);
  int offsetTop = h1+4;
  //printing next line
  String line2 = "Clicks per second:"; //Setting up text on next line.
  display.getTextBounds(line2, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((SWI/2) - (w1/2),offsetTop);
  display.print(line2);
  //printing plaze holder for click speed 
  //finding offset for click speed height and stroing it (clkw1, clkh1)
  display.setTextSize(2);
  clkoff = offsetTop + h1 + 4;
  updateClickSpeed(counter);
  //setting up rotary speed indicator
  display.setTextSize(1);
  offsetTop = clkoff + clkh1 + 2;
  line2 = "Rotary step value:"; //Setting up text on next line.
  display.getTextBounds(line2, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((SWI/2) - (w1/2),offsetTop);
  display.print(line2);
  rotSpeedOffset = offsetTop + h1 + 2;
  //Redering the number on the bottom of the screen.
  rotSpeedF();
}

void volummenu() {
  //clearing display and making it ready
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  //setting up values and strings for top row
  String middle = "Mode";

  //printing left corner
  display.drawBitmap(2, 2, prevIMG, 8, 8, WHITE);
  //printing middle
  display.getTextBounds(middle , 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((SWI/2) - (w1/2), 2);
  display.print(middle);
  //print right
  w1 = 8;
  h1 = 8;
  display.drawBitmap(SWI - (w1 + 2), 2, nextIMG, w1, h1, WHITE);
  int offsetTop = h1+4;

  //printing next line
  String line2 = "Media controls"; //Setting up text on next line.
  display.getTextBounds(line2, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((SWI/2) - (w1/2),offsetTop);
  display.print(line2);

  //calculate middle based of offset top and bottom
  int middleOffset = ((SHE - (offsetTop + 8 + 2))/2) + offsetTop;
  //middle left
  display.setCursor(18, middleOffset);
  display.print("VOL-");
  //middle right
  String right = "VOL+";
  display.getTextBounds(right, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor(SWI - (w1 + 18),middleOffset);
  display.print(right);

  //bottom line center
  w1 = 16;
  h1 = 8;
  display.drawBitmap((SWI/2) - (w1/2), SHE - h1 - 2, plauPauseIMG, w1, h1, WHITE);
  //bottom line right
  w1 = 8;
  h1 = 8;
  display.drawBitmap(SWI - w1 - 2, SHE - h1 - 2, sleepIMG, w1, h1, WHITE);
  display.setCursor(SWI - w1*2 - 2, SHE - h1 - 1);
  display.print("/");
  display.drawBitmap(SWI - w1*3 - 4, SHE - h1 - 1, soundIMG, w1, h1, WHITE);
}

void rotSpeedF() {
  //This function renders the number on the botton. Its also responsible for updating the number
  display.fillRect((SWI/2) - (rotw1/2),rotSpeedOffset,rotw1,roth1, BLACK); 
  display.getTextBounds(itoa(rotSpeed, snum, 10), 0, 0, &x1, &y1, &rotw1, &roth1);
  display.setCursor((SWI/2) - (rotw1/2),rotSpeedOffset);
  display.print(rotSpeed);
}

void updateClickSpeed(int nums){
  String numb;
  display.setTextSize(2);
  if (nums > 1000){
    numb = itoa(counter, snum, 10);
    display.fillRect(0,clkoff-2,128,clkh1+2, BLACK);     
  } else{
    numb = itoa(nums, snum, 10);
    display.fillRect(clkoffw-2,clkoff-2,clkw1 + 17,clkh1+2, BLACK); 
  }
  display.getTextBounds(numb, 0, 0, &x1, &y1, &clkw1, &clkh1);
  clkoffw = (SWI/2) - (clkw1/2);
  display.setCursor(clkoffw,clkoff);
  display.print(numb);
  display.setTextSize(1);
}

void clickOn(){
  display.setTextSize(2);
  String numb;
  if(longPress)
    numb = "Holding";
  else
    numb = itoa(counter, snum, 10);
  display.getTextBounds(numb, 0, 0, &x1, &y1, &clkw1, &clkh1);
  clkoffw = (SWI/2) - (clkw1/2);
  display.fillRect(clkoffw-2,clkoff-2,clkw1+2,clkh1+2, WHITE); 
  display.setCursor(clkoffw,clkoff);
  display.setTextColor(BLACK);
  display.print(numb);
  display.setTextColor(WHITE);
  display.drawBitmap((SWI/2) + (clkw1/2) + 2, clkoff, lockIMG, 16, 16, WHITE);
  display.setTextSize(1);
}
