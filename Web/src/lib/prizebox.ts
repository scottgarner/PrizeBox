export enum BoardIdentifier {
  CODETV_BOARD_0 = "CodeTV Board 0",
  CODETV_BOARD_1 = "CodeTV Board 1",
  CODETV_BOARD_2 = "CodeTV Board 2",
  CODETV_BOARD_3 = "CodeTV Board 3",
}

export enum MessageType {
  KEY_PRESS_0 = 0x00,
  KEY_PRESS_1 = 0x01,
  KEY_PRESS_2 = 0x02,
  KEY_PRESS_3 = 0x03,
  KEY_PRESS_4 = 0x04,
  KEY_PRESS_5 = 0x05,
  KEY_PRESS_6 = 0x06,
  KEY_PRESS_7 = 0x07,
  KEY_PRESS_8 = 0x08,
  KEY_PRESS_9 = 0x09,

  KEY_PRESS_CANCEL = 0x0a,
  KEY_PRESS_ENTER = 0x0b,

  SET_FAILURE = 0x20,
  SET_STANDBY = 0x30,
  SET_SUCCESS = 0x40,

  TRIGGER_BUZZER = 0x50,
  TRIGGER_UNLOCK = 0x60,

  TRIGGER_RESET = 0x70,
}

export enum SysEx {
  HEADER = 0xf0,
  VENDOR = 0x7d,
  FOOTER = 0xf7,
}

export class PrizeBox {
  private messageCallback: (d: BoardIdentifier, data: Uint8Array) => void =
    () => {};
  private keyCallback?: (id: BoardIdentifier, key: number) => void = () => {};
  private enterCallback?: (id: BoardIdentifier) => void = () => {};
  private cancelCallback?: (id: BoardIdentifier) => void = () => {};

  private inputMap = new Map<BoardIdentifier, MIDIInput>();
  private outputMap = new Map<BoardIdentifier, MIDIOutput>();

  private getBoardIdentifier(name: string): BoardIdentifier | undefined {
    return Object.values(BoardIdentifier).find((v) => v === name);
  }

  private updatePort(port: MIDIPort) {
    const id = this.getBoardIdentifier(port.name!);
    if (!id) return;

    // Handle input ports.
    if (port.type === "input") {
      const input = port as MIDIInput;

      if (port.state === "connected") {
        console.log("Updating input device:", port.name, port.state);
        input.onmidimessage = (e: MIDIMessageEvent) => {
          this.handleMidiMessageEvent(id, e);
        };
        this.inputMap.set(id, input);
      } else {
        console.log("Removing input device:", input.name, input.state);
        this.inputMap.delete(id);
      }
    }

    // Handle output ports.
    if (port.type === "output") {
      const output = port as MIDIOutput;
      if (port.state === "connected") {
        console.log("Updating output device:", output.name, output.state);
        this.outputMap.set(id, output);
      } else {
        console.log("Removing output device:", output.name, output.state);
        this.outputMap.delete(id);
      }
    }
  }

  handleStateChange(e: MIDIConnectionEvent): void {
    console.log("MIDI device state change:", e);
    if (e.port) this.updatePort(e.port);
  }

  async connect(): Promise<void> {
    await navigator.requestMIDIAccess({ sysex: true }).then((midiAccess) => {
      midiAccess.inputs.forEach((input: MIDIInput) => {
        this.updatePort(input);
      });

      midiAccess.outputs.forEach((output: MIDIOutput) => {
        this.updatePort(output);
      });

      midiAccess.onstatechange = (event: Event) =>
        this.handleStateChange(event as MIDIConnectionEvent);
    });
  }

  send(id: BoardIdentifier, bytes: number[] | Uint8Array): void {
    if (this.outputMap.has(id)) {
      console.log("Sending to output:", id, bytes);
      const output = this.outputMap.get(id);
      output!.send(bytes);
    } else {
      console.log("Output not found!");
    }
  }

  private handleMidiMessageEvent(id: BoardIdentifier, e: MIDIMessageEvent) {
    const data = new Uint8Array(e.data!);

    // All messages
    this.messageCallback(id, data);

    if (data[0] === SysEx.HEADER && data[1] === SysEx.VENDOR) {
      const key = data[2];
      const state = data[3];

      // Keys.
      if (
        key >= MessageType.KEY_PRESS_0 &&
        key <= MessageType.KEY_PRESS_9 &&
        state == 1
      ) {
        this.keyCallback?.(id, key);
      }

      // Cancel.
      if (key === MessageType.KEY_PRESS_CANCEL && state == 1) {
        this.cancelCallback?.(id);
      }

      // Enter.
      if (key === MessageType.KEY_PRESS_ENTER && state == 1) {
        this.enterCallback?.(id);
      }
    }
  }

  // Callbacks.

  onMessage(callback: (id: BoardIdentifier, data: Uint8Array) => void): void {
    this.messageCallback = callback;
  }

  onKey(callback: (id: BoardIdentifier, key: number) => void): void {
    this.keyCallback = callback;
  }

  onCancel(callback: (id: BoardIdentifier) => void): void {
    this.cancelCallback = callback;
  }

  onEnter(callback: (id: BoardIdentifier) => void): void {
    this.enterCallback = callback;
  }

  // Simple commands.

  setFailureOn(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_FAILURE,
      1,
      SysEx.FOOTER,
    ]);
  }
  setFailureOff(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_FAILURE,
      0,
      SysEx.FOOTER,
    ]);
  }

  setStandByOn(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_STANDBY,
      1,
      SysEx.FOOTER,
    ]);
  }
  setStandByOff(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_STANDBY,
      0,
      SysEx.FOOTER,
    ]);
  }

  setSuccessOn(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_SUCCESS,
      1,
      SysEx.FOOTER,
    ]);
  }
  setSuccessOff(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_SUCCESS,
      0,
      SysEx.FOOTER,
    ]);
  }

  triggerBuzzer(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.TRIGGER_BUZZER,
      5,
      SysEx.FOOTER,
    ]);
  }

  triggerUnlock(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.TRIGGER_UNLOCK,
      1,
      SysEx.FOOTER,
    ]);
  }

  triggerReset(id: BoardIdentifier): void {
    this.send(id, [
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.TRIGGER_RESET,
      1,
      SysEx.FOOTER,
    ]);
  }
}
