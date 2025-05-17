#

## Board Identifier

Each board is identified by a `BoardIdentifier` that comes in with all callbacks and is required for all method calls.

```
export enum BoardIdentifier {
  CODETV_BOARD_0 = "CodeTV Board 0",
  CODETV_BOARD_1 = "CodeTV Board 1",
  CODETV_BOARD_2 = "CodeTV Board 2",
  CODETV_BOARD_3 = "CodeTV Board 3",
}
```

## Callbacks

### Raw MIDI Callback

```
private messageCallback: (d: BoardIdentifier, data: Uint8Array) => void;
```

### Key Press Callbacks

```
private keyCallback?: (id: BoardIdentifier, key: number) => void;
private enterCallback?: (id: BoardIdentifier) => void;
private cancelCallback?: (id: BoardIdentifier) => void;
```

## Methods

### Light control.

```
setFailureOn(id: BoardIdentifier): void;
setFailureOff(id: BoardIdentifier): void;
setStandByOn(id: BoardIdentifier): void;
setStandByOff(id: BoardIdentifier): void;
setSuccessOn(id: BoardIdentifier): void;
setSuccessOff(id: BoardIdentifier): void;
```

### Buzzer/lock control.

```
triggerBuzzer(id: BoardIdentifier): void;
triggerUnlock(id: BoardIdentifier): void;
```

### MPR-121 (Touch Sensor) Reset

```
triggerReset(id: BoardIdentifier): void;
```
