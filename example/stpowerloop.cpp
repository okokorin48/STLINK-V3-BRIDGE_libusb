// Other includes
#include <bridge.h>

// Standard includes
#include <chrono>
#include <iostream>
#include <thread>

using namespace std;

/* mapped STLINK power enable to GPIO0 */
#define ST_GPIO_POWER 0
#define ST_GPIO_CS 1

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

void
perform_gpio_work(Brg *stlink_bridge);

int
main()
{
    cout << "Welcome to the STLink GPIO0 enable!" << endl;

    STLinkInterface stlink_interface;
    Brg stlink_bridge(stlink_interface);

#ifdef USING_ERRORLOG
    cErrLog err_log;
    stlink_interface.BindErrLog(&err_log);
    stlink_bridge.BindErrLog(&err_log);
#endif

    cout << "Attempting to open ST-Link... ";
    stlink_interface.LoadStlinkLibrary("");
    Brg_StatusT status = stlink_bridge.OpenStlink();
    switch (status) {
    case BRG_NO_ERR:
        cout << "success!" << endl;
        break;
    default:
        cout << "fail! Some error occured (" << status << ")" << endl;
        break;
    }

    if (status == BRG_NO_ERR) {
        perform_gpio_work(&stlink_bridge);
        return 0;
    } else {
        return -1;
    }
}

int
brg_status_print(Brg_StatusT status)
{
    switch (status) {
    case BRG_NO_ERR:
        cout << "success!" << endl;
        return 0;
    default:
        cout << "fail! Some error occured (" << status << ")" << endl;
        return 1;
    }
}

// Write/set gpios all at once
int
write_gpio_all(Brg *brg, int gpios)
{
    Brg_StatusT status;
    uint8_t error_mask = 0;
    Brg_GpioValT gpio_vals[BRG_GPIO_MAX_NB];

    for (size_t i = 0; i < ARRAY_SIZE(gpio_vals); ++i) {
        gpio_vals[i] = (1 << i) & gpios ? GPIO_SET : GPIO_RESET;
    }

    status = brg->SetResetGPIO(BRG_GPIO_ALL, gpio_vals, &error_mask);
    if (brg_status_print(status)) {
        return 1;
    }
    if (error_mask) {
        return 1;
    }
    return 0;
}


int
write_gpio_set(Brg *brg, int gpios)
{
    Brg_StatusT status;
    uint8_t error_mask = 0;
    Brg_GpioValT gpio_vals[BRG_GPIO_MAX_NB];

    status = brg->ReadGPIO(BRG_GPIO_ALL, gpio_vals, &error_mask);

    gpio_vals[gpios] = GPIO_SET;

    status = brg->SetResetGPIO(BRG_GPIO_ALL, gpio_vals, &error_mask);
    if (brg_status_print(status)) {
        return 1;
    }
    if (error_mask) {
        return 1;
    }
    return 0;
}


int
write_gpio_reset(Brg *brg, int gpios)
{
    Brg_StatusT status;
    uint8_t error_mask = 0;
    Brg_GpioValT gpio_vals[BRG_GPIO_MAX_NB];

    status = brg->ReadGPIO(BRG_GPIO_ALL, gpio_vals, &error_mask);

    gpio_vals[gpios] = GPIO_RESET;

    status = brg->SetResetGPIO(BRG_GPIO_ALL, gpio_vals, &error_mask);
    if (brg_status_print(status)) {
        return 1;
    }
    if (error_mask) {
        return 1;
    }
    return 0;
}


void
perform_gpio_work(Brg *stlink_bridge)
{
    Brg_StatusT status;
    // Define just one to apply to all GPIOs
    Brg_GpioConfT gpio_confs[] = {
        {
            .Mode = GPIO_MODE_OUTPUT,
            .Speed = GPIO_SPEED_LOW,
            .Pull = GPIO_NO_PULL,
            .OutputType = GPIO_OUTPUT_PUSHPULL,
        },
    };

    Brg_GpioInitT init = {
        .GpioMask = BRG_GPIO_ALL,
        .ConfigNb = ARRAY_SIZE(gpio_confs),
        .pGpioConf = gpio_confs,
    };

    cout << "Attempting to configure GPIOs... ";
    status = stlink_bridge->InitGPIO(&init);
    if (brg_status_print(status)) {
        return;
    }
#if 0
    cout << "ST GPIO power set - ";

    if (write_gpio_set(stlink_bridge, ST_GPIO_POWER)) {
        return;
    }

    cout << "ST GPIO power reset - ";

    if (write_gpio_reset(stlink_bridge, ST_GPIO_POWER)) {
        return;
    }
#endif

    /* optional encless GPIO glip-flop */

#if 1
	using namespace std::this_thread; // sleep_for, sleep_until
	using namespace std::chrono; // nanoseconds, system_clock, seconds

	do {

	    sleep_until(system_clock::now() + seconds(2));
	
	    cout << "ST GPIO power set - ";

	    if (write_gpio_set(stlink_bridge, ST_GPIO_POWER)) {
	        return;
	    }
	
	    sleep_until(system_clock::now() + seconds(2));
	
	    cout << "ST GPIO CS set - ";

	    if (write_gpio_set(stlink_bridge, ST_GPIO_CS)) {
	        return;
	    }
	

	    sleep_until(system_clock::now() + seconds(2));
	
	    cout << "ST GPIO CS reset - ";

	    if (write_gpio_reset(stlink_bridge, ST_GPIO_CS)) {
	        return;
	    }

	    sleep_until(system_clock::now() + seconds(2));
	
	    cout << "ST GPIO power reset - ";

	    if (write_gpio_reset(stlink_bridge, ST_GPIO_POWER)) {
	        return;
	    }

	} while(1);

#endif
}
