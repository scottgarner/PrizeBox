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
}

export enum SysEx {
  HEADER = 0xf0,
  VENDOR = 0x7d,
  FOOTER = 0xf7,
}

export class PrizeBox {
  private messageCallback: (data: Uint8Array) => void = () => {};

  private keyCallback: (key: number) => void = () => {};
  private enterCallback: () => void = () => {};
  private cancelCallback: () => void = () => {};

  private output: MIDIOutput | null = null;

  async connect(): Promise<void> {
    const midiAccess = await navigator.requestMIDIAccess({ sysex: true });
    const outputs = Array.from(midiAccess.outputs.values());
    const inputs = Array.from(midiAccess.inputs.values());

    if (!outputs.length || !inputs.length) {
      throw new Error("No MIDI ports found");
    }

    this.output = outputs[0];

    inputs[0].onmidimessage = (e: MIDIMessageEvent) => {
      console.log("Received", e.data);

      const data = new Uint8Array(e.data!);

      // All messages
      this.messageCallback(data);

      console.log(data);
      if (data[0] === SysEx.HEADER && data[1] === SysEx.VENDOR) {
        const key = data[2];
        const state = data[3];

        console.log(key, state);

        // Keys.
        if (
          key >= MessageType.KEY_PRESS_0 &&
          key <= MessageType.KEY_PRESS_9 &&
          state == 1
        ) {
          console.log(this, this.keyCallback);
          this.keyCallback(key);
        }

        // Cancel.
        if (key === MessageType.KEY_PRESS_CANCEL && state == 1) {
          this.cancelCallback();
        }

        // Enter.
        if (key === MessageType.KEY_PRESS_ENTER && state == 1) {
          this.enterCallback();
        }
      }
    };

    outputs.forEach((o) => console.log("Output:", o.name));
    inputs.forEach((i) => console.log("Input:", i.name));
  }

  send(bytes: number[] | Uint8Array): void {
    console.log("Sending", bytes);

    if (!this.output) {
      console.log("Not connected");
      return;
    }

    this.output.send(bytes);
  }

  onMessage(callback: (data: Uint8Array) => void): void {
    this.messageCallback = callback;
  }

  onKey(callback: (key: number) => void): void {
    this.keyCallback = callback;
  }

  onCancel(callback: () => void): void {
    this.cancelCallback = callback;
  }

  onEnter(callback: () => void): void {
    this.enterCallback = callback;
  }

  // Simple commands.

  setFailureOn(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_FAILURE,
      1,
      SysEx.FOOTER,
    ]);
  }
  setFailureOff(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_FAILURE,
      0,
      SysEx.FOOTER,
    ]);
  }

  setStandByOn(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_STANDBY,
      1,
      SysEx.FOOTER,
    ]);
  }
  setStandByOff(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_STANDBY,
      0,
      SysEx.FOOTER,
    ]);
  }

  setSuccessOn(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_SUCCESS,
      1,
      SysEx.FOOTER,
    ]);
  }
  setSuccessOff(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.SET_SUCCESS,
      0,
      SysEx.FOOTER,
    ]);
  }

  triggerBuzzer(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.TRIGGER_BUZZER,
      50,
      SysEx.FOOTER,
    ]);
  }

  triggerUnlock(): void {
    this.send([
      SysEx.HEADER,
      SysEx.VENDOR,
      MessageType.TRIGGER_UNLOCK,
      1,
      SysEx.FOOTER,
    ]);
  }
}
