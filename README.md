# SoftwareEmbarcado
    - Repositorio para projeto de software embarcado - UFABC 2024.2
    - Autor: Pedro Henrique Correa Foramilio
        - Complementos: Prof. João Ranhel

## General Behavior:

This embedded software application is designed to manage and display internal and external values on a 7-segment display. The application operates in different modes, displaying either two internal values (CRON and ADC) or both internal and external values, depending on user interactions and received requests. Full specifications are available on PDF file on this github repository.

### Key Components:
1. **Internal Values:**
   - **CRON:** A chronometer value.
   - **ADC:** An analog-to-digital converter value.

2. **External Values:**
   - **External CRON:** Received via UART.
   - **External ADC:** Received via UART.

3. **Harware Devices:**
    - **LEDs**: 4 LEDs to indicate which value is being displayed.
    - **UART**: An UART to handle communication between devices.
    - **ADC**
    - **7-segment display**

### Mode Switching:
The application switches between different display modes based on:
- **User Button Presses:** Physical buttons connected to GPIO pins trigger interrupts that change the display mode.
- **Received Requests:** UART communication allows the application to receive external values and requests, influencing the display mode.

### Display Modes:
1. **STARTUP:** Initial mode where all LEDs are turned on.
2. **WFI (Wait For Interrupt):** All LEDs are turned on, waiting for an interrupt while displaying RA.
3. **LED_CRON:** Toggles LED1 and displays the internal CRON value.
4. **LED_ADC:** Toggles LED2 and displays the internal ADC value.
5. **LED_CRON_EXT:** Toggles LED3 and displays the external CRON value.
6. **LED_ADC_EXT:** Toggles LED4 and displays the external ADC value.

### Behavior:
1. **Initialization:**
   - The `StartDefaultTask` function initializes peripherals and starts the default task, which includes setting up UART reception and resetting LEDs.

2. **LED Management:**
   - The `MngLED` task manages the state of LEDs based on the current mode (`modoLed`). It operates in an task, switching LED states and delaying for a predefined time (`DT_LEDS`).

3. **Display Management:**
   - The `fn_Task_Varrer` function updates the display based on the current LED mode. It handles the display of internal and external values and ensures the correct decimal point position. Each time it needs to display an external value, it puts a request code into the queue.

4. **Communication Management:**
   - The `fn_Task_MngComns` function manages UART communication, handling requests from a queue and transmitting data. It processes messages to send appropriate data over UART.

5. **Button Press Handling:**
   - The `EXTI1_IRQHandler`, `EXTI2_IRQHandler`, and `EXTI3_IRQHandler` functions handle external interrupts for button presses. They debounce the button presses and update the system state, sending requests or changing display modes as needed.

6. **UART Callback Function:**
    - The `HAL_UART_RxCpltCallback` function is responsible for receiving external values and managing mode trasnsition whenever a request for service is received (or end of service).

### Summary:

The application continuously monitors button presses and UART communication to update the display with the appropriate values. It ensures smooth transitions between different display modes, providing real-time updates of internal and external values on the 7-segment display.

---

## StartDefaultTask
### Description
The `StartDefaultTask` function is the default task in the system. It initializes certain peripherals and enters an infinite loop.

### Parameters
- `argument`: A pointer to the argument passed to the task (not used in this function).

### Behavior
1. Records the start time using `HAL_GetTick()`.
2. Resets the state of LEDs connected to GPIO pins `LED1`, `LED2`, `LED3`, and `LED4` on port `GPIOB`.
3. Initiates an interrupt-based UART receive operation using `HAL_UART_Receive_IT`.
4. Enters an infinite loop with a delay of 1 ms in each iteration.

### Expected Side-effects
1. Responsible for the initial display of LEDs on the 7-seg display for 2 seconds. This side-effect is manifested in a later mentioned task

---

## MngLED

### Description
The LED control task manages the state of LEDs connected to GPIO pins on port `GPIOB`. It operates in an infinite loop and changes the LED states based on the current mode (`modoLed`).

### Modes
- `STARTUP`: Turns off all LEDs.
- `WFI`: Turns on all LEDs.
- `LED_CRON`: Toggles `LED1` and turns off `LED2`, `LED3`, and `LED4`.
- `LED_ADC`: Toggles `LED2` and turns off `LED1`, `LED3`, and `LED4`.
- `LED_CRON_EXT`: Toggles `LED3` and turns off `LED1`, `LED2`, and `LED4`.
- `LED_ADC_EXT`: Toggles `LED4` and turns off `LED1`, `LED2`, and `LED3`.

### Behavior
1. Enters an infinite loop.
2. Switches the LED states based on the current mode (`modoLed`).
3. Delays for a predefined time (`DT_LEDS`) before the next iteration.

---

## TaskDisplay

### Description
The `fn_TaskDisplay` function is responsible for managing the display mode and updating the LED mode accordingly. It operates in an infinite loop and changes the LED mode based on the current display mode (`modoDisplay`). In short, the function of this task is to change the (`modoDisplay`) variable to provides the appropriate sequence of values to display on screen.

### Parameters
- `argument`: A pointer to the argument passed to the task (not used in this function).

### Display Modes
- `WFI`: No action is taken.
- `DISPLAY_INTRN`: Toggles between `LED_CRON` and `LED_ADC` modes.
- `DISPLAY_EXTRN`: Cycles through `LED_CRON_EXT`, `LED_ADC`, `LED_CRON_EXT`, `LED_ADC_EXT`, and `WFI` modes based on the current LED mode and the state of `A1_foi_apertado`. 
    - If `A1_foi_apertado == 0` then only the external values are displayed, and the RA is displayed in place of the internal values.
    - If `A1_foi_apertado == 1` then the task cycles through all the values, ignoring RA.

### Behavior
1. Enters an infinite loop.
2. Switches the LED mode based on the current display mode (`modoDisplay`).
3. Delays for a predefined time (`DT_DISPLAY_MD1` or `DT_DISPLAY_MD2`) before the next iteration.

### Expected Side-effects
1. Besides the use of (`modoDisplay`) by the task MngLED to determine which LED to toggle on and off, this variable is also used on a later specified task to determine which value to display on the 7-seg display.

---

## Task_Varrer

### Description
The `fn_Task_Varrer` function is responsible for updating the display based on the current LED mode (`modoLed`). It operates in an infinite loop and updates the display values accordingly.

### Parameters
- `argument`: A pointer to the argument passed to the task (not used in this function).

### LED Modes
- `STARTUP`: Displays `8888` on the display.
- `WFI`: Displays the values from the `RA` array.
- `LED_CRON`: Displays the values from the `Crono` array and sets the decimal point value to 10 (`0b0110`).
- `LED_ADC`: Displays the values from the `ValAdc` array and sets the decimal point value to 8 (`0b0100`).
- `LED_CRON_EXT`: Puts in the queue a request for external chronometer data, displays the values from the `CronoExt` array, and sets the decimal point value to (`0b0110`).
- `LED_ADC_EXT`: Puts in the queue a request for external ADC data, displays the values from the `ValAdcExt` array, and sets the decimal point value to 8 (`0b0100`).

### Behavior
1. Enters an infinite loop.
2. Checks if the startup period has completed (2000 ms) and sets the LED mode to `WFI` if not already done. This is responsible for the startup procedure for 2 seconds, where every point and segment is displayed on screen.
3. Switches the display values based on the current LED mode (`modoLed`).
4. Calls `mostrar_no_display` to update the display with the current values.
5. Delays for a predefined time (`DT_VARRE_DISPLAY`) before the next iteration.

---

## fn_Task_MngComns

### Description
The `fn_Task_MngComns` function manages communication by handling requests from a queue and transmitting data over UART. It operates in an infinite loop and processes messages from the queue to send appropriate data. It is the main and only process to send messages to a external device.

### Parameters
- `argument`: A pointer to the argument passed to the task (not used in this function).

### Queue Messages
- `Q_SND_CRN`: Sends the current chronometer value.
- `Q_SND_ADC`: Sends the current ADC value.
- `Q_REQ_CRN`: Sends a request for chronometer data.
- `Q_REQ_ADC`: Sends a request for ADC data.
- `Q_REQ_SRV`: Sends a request for external service, where the external device must display values provided.
- `Q_REQ_OFF`: Sends a request to turn stop displaying provided values.

### Behavior
1. Enters an infinite loop.
2. Checks if the UART is busy transmitting.
3. If the UART is not busy and there are messages in the queue, it processes the messages.
4. Depending on the message, it prepares the data to be sent and transmits it over UART using interrupt-based transmission.
5. Delays for 1 ms before the next iteration.


---

## HAL_UART_RxCpltCallback - Callback Function

### Description
The `HAL_UART_RxCpltCallback` function is a UART receive complete callback. It processes incoming UART data and updates the system state based on the received messages.

### Parameters
- `huart`: A pointer to the `UART_HandleTypeDef` structure that contains the configuration information for the specified UART module.

### Behavior
1. Disables interrupts to ensure atomic operations.
2. Checks the first character of the received buffer (`BufIN`) to determine the type of message received and performs the corresponding actions:
   - `'s'`: Resets all LEDs, sets `modoDisplay` to `DISPLAY_EXTRN`, and updates `modoLed` based on the state of `A1_foi_apertado` to decide whether to show internal values or not.
   - `'n'`: Resets all LEDs, sets `modoDisplay` to `DISPLAY_INTRN`, and updates `modoLed` based on the state of `A1_foi_apertado` to decide whether to enter the state `waiting for intput` or not. It's purpose is to signal that external values must not be displayed anymore.
   - `'A'`: External ADC value recieved. Converts the next four characters to numeric values and stores them in `ValAdcExt`.
   - `'T'`: External CRON value recieved. Converts the next four characters to numeric values and stores them in `CronoExt`.
   - `'t'`: Puts a `Q_SND_CRN` message in the queue to signal the need to send a cron value update.
   - `'a'`: Puts a `Q_SND_ADC` message in the queue to signal the need to send a adc value update.
3. Re-enables interrupts.
4. Re-initiates UART reception in interrupt mode to continuously listen for incoming data.

---

## EXTI1_IRQHandler

### Description
The `EXTI1_IRQHandler` function handles the external interrupt for GPIO pin, triggered by a button press. It debounces the button press and updates the system state accordingly.

### Behavior
1. Checks if the debounce period (`DT_DEBOUNCING`) has elapsed since the last interrupt.
2. If the debounce period has elapsed:
   - Updates the timestamp of the last interrupt (`tIN_IRQ1`).
   - Sets the LED mode to `LED_CRON`.
   - Updates the display mode to `DISPLAY_INTRN` if not receiving data, otherwise sets it to `DISPLAY_EXTRN`.
   - Sets the flag `A1_foi_apertado` to indicate the button was pressed.
   - Turns on all LEDs connected to GPIO pins `LED1`, `LED2`, `LED3`, and `LED4` on port `GPIOB`.
3. Calls `HAL_GPIO_EXTI_IRQHandler` to handle the GPIO interrupt.

---

## EXTI2_IRQHandler

### Description
The `EXTI2_IRQHandler` function handles the external interrupt for GPIO pin, triggered by a button press. It debounces the button press and sends a request to the server.

### Behavior
1. Checks if the debounce period (`DT_DEBOUNCING`) has elapsed since the last interrupt.
2. If the debounce period has elapsed:
   - Updates the timestamp of the last interrupt (`tIN_IRQ2`).
   - Inserts a `Q_REQ_SRV` message into the queue to request the external device to show provided values.
3. Calls `HAL_GPIO_EXTI_IRQHandler` to handle the GPIO interrupt.


---

## EXTI3_IRQHandler

### Description
The `EXTI3_IRQHandler` function handles the external interrupt for GPIO pin, triggered by a button press. It debounces the button press and sends a request to turn off the server.

### Behavior
1. Checks if the debounce period (`DT_DEBOUNCING`) has elapsed since the last interrupt.
2. If the debounce period has elapsed:
   - Updates the timestamp of the last interrupt (`tIN_IRQ3`).
   - Inserts a `Q_REQ_OFF` message into the queue to request turning off the external service.
3. Calls `HAL_GPIO_EXTI_IRQHandler` to handle the GPIO interrupt.
