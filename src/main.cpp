/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Kernel.h"
#include "modules/tools/laser/Laser.h"
#include "modules/tools/extruder/Extruder.h"
#include "modules/tools/temperaturecontrol/TemperatureControlPool.h"
#include "modules/tools/endstops/Endstops.h"
#include "modules/tools/switch/SwitchPool.h"
#include "modules/robot/Conveyor.h"
#include "modules/utils/button/ButtonPool.h"
#include "modules/utils/simpleshell/SimpleShell.h"
#include "modules/utils/configurator/Configurator.h"
#include "modules/utils/currentcontrol/CurrentControl.h"
#include "modules/utils/player/Player.h"
#include "modules/utils/pausebutton/PauseButton.h"
// #include "libs/ChaNFSSD/SDFileSystem.h"
#include "libs/Config.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"

// Debug
#include "libs/SerialMessage.h"

#include "libs/USBDevice/USB.h"
#include "libs/USBDevice/USBMSD/USBMSD.h"
#include "libs/USBDevice/USBMSD/SDCard.h"
#include "libs/USBDevice/USBSerial/USBSerial.h"
#include "libs/USBDevice/DFU.h"

#include "libs/SDFAT.h"

#include "libs/Watchdog.h"

#define second_usb_serial_enable_checksum  CHECKSUM("second_usb_serial_enable")

// Watchdog wd(5000000, WDT_MRI);

// #include "libs/USBCDCMSC/USBCDCMSC.h"
// SDFileSystem sd(p5, p6, p7, p8, "sd");  // LPC17xx specific : comment if you are not using a SD card ( for example with a mBed ).
SDCard sd(P0_9, P0_8, P0_7, P0_6);
//LocalFileSystem local("local");       // LPC17xx specific : comment if you are not running a mBed
// USBCDCMSC cdcmsc(&sd);                  // LPC17xx specific : Composite serial + msc USB device

USB u;

USBSerial usbserial(&u);
USBMSD msc(&u, &sd);
DFU dfu(&u);

SDFAT mounter("sd", &sd);

char buf[512];

GPIO leds[5] = {
    GPIO(P1_18),
    GPIO(P1_19),
    GPIO(P1_20),
    GPIO(P1_21),
    GPIO(P4_28)
};

int main() {

    // Default pins to low status
    for (int i = 0; i < 5; i++){
        leds[i].output();
        leds[i] = (i & 1) ^ 1;
    }

    sd.disk_initialize();

    Kernel* kernel = new Kernel();

    kernel->streams->printf("Smoothie ( grbl port ) version 0.7.2 \r\n");

    // Create and add main modules
    kernel->add_module( new Laser() );
    kernel->add_module( new Extruder() );
    kernel->add_module( new SimpleShell() );
    kernel->add_module( new Configurator() );
    kernel->add_module( new CurrentControl() );
    kernel->add_module( new TemperatureControlPool() );
    kernel->add_module( new SwitchPool() );
    kernel->add_module( new ButtonPool() );
    kernel->add_module( new PauseButton() );
    kernel->add_module( new Endstops() );
    kernel->add_module( new Player() );

    // Create and initialize USB stuff
    u.init();
    kernel->add_module( &msc );
    kernel->add_module( &usbserial );
    if( kernel->config->value( second_usb_serial_enable_checksum )->by_default(false)->as_bool() ){
        kernel->add_module( new USBSerial(&u) );
    }
    kernel->add_module( &dfu );
    kernel->add_module( &u );

    // Main loop
    while(1){
        kernel->call_event(ON_MAIN_LOOP);
        kernel->call_event(ON_IDLE);
    }
}
