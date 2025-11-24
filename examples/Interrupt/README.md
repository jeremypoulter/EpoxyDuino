# Interrupt Example

This example demonstrates the interrupt support in EpoxyDuino.

## Features

EpoxyDuino provides a functional implementation of Arduino's interrupt API:

### Standard Arduino Functions

- `attachInterrupt(interruptNum, handler, mode)` - Attach an interrupt handler
- `detachInterrupt(interruptNum)` - Remove an interrupt handler
- `attachInterruptArg(pin, handler, arg, mode)` - Attach handler with argument (ESP8266 only)

### Interrupt Modes

EpoxyDuino supports all standard Arduino interrupt modes:

- `RISING` - Trigger when pin goes from LOW to HIGH
- `FALLING` - Trigger when pin goes from HIGH to LOW
- `CHANGE` - Trigger on any state change
- `ONLOW` - Trigger when pin is LOW (ESP8266 only)
- `ONHIGH` - Trigger when pin is HIGH (ESP8266 only)
- `ONLOW_WE` - Trigger when pin is LOW with wake enable (ESP8266 only)
- `ONHIGH_WE` - Trigger when pin is HIGH with wake enable (ESP8266 only)

### EpoxyDuino-Specific Functions

For testing interrupt-driven code, EpoxyDuino provides additional functions:

- `checkInterrupts()` - Check all pins and trigger appropriate interrupt handlers
- `triggerInterrupt(interruptNum)` - Manually trigger an interrupt handler
- `digitalReadValue(pin, value)` - Set the value that `digitalRead(pin)` will return

## Usage Pattern

The typical usage pattern in EpoxyDuino is:

```cpp
// 1. Attach interrupt handlers in setup()
attachInterrupt(PIN_NUMBER, myHandler, RISING);

// 2. Simulate pin changes using digitalReadValue()
digitalReadValue(PIN_NUMBER, HIGH);

// 3. Call checkInterrupts() to process pin state changes
checkInterrupts();
```

Alternatively, you can directly trigger interrupts for testing:

```cpp
// Directly trigger an interrupt without changing pin state
triggerInterrupt(PIN_NUMBER);
```

## Building and Running

```bash
make
./Interrupt.out
```

## Example Output

The example demonstrates:
1. RISING edge detection
2. FALLING edge detection  
3. CHANGE detection (any edge)
4. Detaching and reattaching interrupts
5. Interrupt statistics tracking

## Implementation Notes

- Interrupts in EpoxyDuino are **not asynchronous** - they must be explicitly checked using `checkInterrupts()`
- Up to 32 interrupt handlers can be active simultaneously
- Each pin can have one interrupt handler attached
- Interrupt handlers should follow the same best practices as on real hardware (keep them short, use `volatile` for shared variables)
- The implementation tracks the last pin state to detect edges (RISING, FALLING, CHANGE)
